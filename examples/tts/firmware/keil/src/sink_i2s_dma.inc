#include "peripheral_i2s.h"

#include "port_gen_os_driver.h"

#define GEN_OS          ((const gen_os_driver_t *)platform_get_gen_os_driver())

#define DMA_CH_ID           0
#define DMA_DESC_NUM        80
#define DMA_BLOCK_SIZE      (AMR_WB_PCM_FRAME_16k / 8)

static DMA_ChannelReg dma_desc[DMA_DESC_NUM] __attribute__((aligned (8))) = {0};

static int16_t audio_buffer[DMA_DESC_NUM][DMA_BLOCK_SIZE] = {0};

static volatile uint8_t dma_error = 0;

static DMA_Descriptor * volatile p_first_ready_dma_desc = NULL;
static DMA_Descriptor * volatile p_last_ready_dma_desc = NULL;

static DMA_Descriptor * volatile p_free_dma_desc = NULL;

static gen_handle_t event_dma_block_done = NULL;

static uint32_t dma_isr(void *user_data)
{
    uint32_t state = DMA_GetChannelIntState(DMA_CH_ID);
    DMA_ClearChannelIntState(DMA_CH_ID, state);

    if (state & DMA_IRQ_ERROR)
    {
        I2S_Enable(APB_I2S, 0, 0);
        dma_error = 1;
        GEN_OS->event_set(event_dma_block_done);
        return 0;
    }

    if (state & DMA_IRQ_TC)
    {
        if (p_first_ready_dma_desc)
        {
            DMA_Descriptor *p = p_first_ready_dma_desc;
            p_first_ready_dma_desc = p_first_ready_dma_desc->Next;

            p->Next = p_free_dma_desc;
            p_free_dma_desc = p;
        }

        if (p_first_ready_dma_desc)
        {
           APB_DMA->Channels[DMA_CH_ID].Descriptor.Ctrl |= 1;
        }
        else
        {
            I2S_Enable(APB_I2S, 0, 0);
        }

        GEN_OS->event_set(event_dma_block_done);
    }
    return 0;
}

static void init_sink(void)
{
    int i;

    SYSCTRL_ClearClkGateMulti((1 << SYSCTRL_ITEM_APB_DMA));

    // Workaround bug in Cube v0.9.26: I2S DATA OUT not set
    PINCTRL_SetPadMux(21, IO_SOURCE_I2S_DATA_OUT);

    I2S_Config(APB_I2S, I2S_ROLE_MASTER, I2S_MODE_STANDARD, 1, 0, 0, 1, 16);
    I2S_Enable(APB_I2S, 0, 0);

    platform_set_irq_callback(PLATFORM_CB_IRQ_DMA, dma_isr, NULL);
    event_dma_block_done = GEN_OS->event_create();
}

static uint8_t is_playing()
{
    return APB_I2S->ModeConfig & 1;
}

static void start_play(void)
{
    I2S_Enable(APB_I2S, 0, 0);
    //I2S_ConfigIRQ(APB_I2S, 0, 0, 0, 0);
     I2S_ConfigIRQ(APB_I2S, 1, 1, 8, 8);

    DMA_EnableChannel(DMA_CH_ID, p_first_ready_dma_desc);
    I2S_ClearTxFIFO(APB_I2S);
    I2S_ClearRxFIFO(APB_I2S);
    I2S_DMAEnable(APB_I2S, 1, 0);

    int i;
    for (i = 0; i < 16; i++)
        I2S_PushTxFIFO(APB_I2S, 0);

    I2S_Enable(APB_I2S, 1, 0);
}

static void prepare(void)
{
    int i;

    I2S_Enable(APB_I2S, 0, 0);
    DMA_Reset(1);

    p_first_ready_dma_desc = NULL;
    p_last_ready_dma_desc = NULL;
    p_free_dma_desc = &dma_desc[0].Descriptor;

    dma_error = 0;

    for (i = 0; i < DMA_DESC_NUM; i++)
    {
        dma_desc[i].Descriptor.SrcAddr = (uint32_t)audio_buffer[i];
        dma_desc[i].Descriptor.Next = i < DMA_DESC_NUM - 1 ? &dma_desc[i + 1].Descriptor : NULL;
    }
}

static void append_block(DMA_Descriptor *desc)
{
    desc->Next = NULL;

    if (p_first_ready_dma_desc == NULL)
    {
        GEN_OS->enter_critical();
        p_last_ready_dma_desc = desc;
        p_first_ready_dma_desc = desc;
        GEN_OS->leave_critical();
    }

    p_last_ready_dma_desc->Next = desc;
    p_last_ready_dma_desc = desc;
}

static int do_save_pcm_samples(const int16_t *pcm_samples, int number, int acc_number)
{
    if (acc_number == 0)
    {
        prepare();
    }

    if (number & 1) number--;

    while ((number > 0) && (dma_error == 0))
    {
        if (NULL == p_free_dma_desc)
        {
            if (is_playing() == 0)
            {
                start_play();
            }
            GEN_OS->event_wait(event_dma_block_done);
        }

        if (NULL == p_free_dma_desc)
            continue;

        GEN_OS->enter_critical();
        DMA_Descriptor *desc = p_free_dma_desc;
        p_free_dma_desc = desc->Next;
        GEN_OS->leave_critical();

        int block = number <= DMA_BLOCK_SIZE ? number : DMA_BLOCK_SIZE;

        memcpy((void *)desc->SrcAddr, pcm_samples, block * sizeof(pcm_samples[0]));

        DMA_PrepareMem2Peripheral(desc,
            SYSCTRL_DMA_I2S_TX, (uint32_t *)desc->SrcAddr, block * sizeof(pcm_samples[0]),
            DMA_ADDRESS_INC,
            DMA_MAKE_BURST_SIZE_OPTION(1)
            | (1 << DMA_DESC_OPTION_BIT_ENABLE)
            | (1 << DMA_DESC_OPTION_BIT_HIGH_PRIORITY)
            | (1 << DMA_DESC_OPTION_BIT_INTERRUPT_EACH_DESC)
            );

        append_block(desc);

        number -= block;
        pcm_samples += block;
    }

    return dma_error ? -1 : 0;
}
