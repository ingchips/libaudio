#include "peripheral_i2s.h"

#include "port_gen_os_driver.h"

#define GEN_OS          ((const gen_os_driver_t *)platform_get_gen_os_driver())

#define BUF_DESC_NUM        30
#define BUF_BLOCK_SIZE      (AMR_WB_PCM_FRAME_16k / 2)

static int32_t audio_buffer[BUF_DESC_NUM][BUF_BLOCK_SIZE / 2] = {0};

struct buf_desc
{
    struct buf_desc *next;
    int32_t *buf;
    const int32_t *p;
    int len;
};

static struct buf_desc buf_descs[BUF_DESC_NUM];

static struct buf_desc * volatile p_curr_buf = NULL;
static struct buf_desc * volatile p_last_buf = NULL;

static struct buf_desc * volatile p_free_desc = NULL;

static gen_handle_t event_dma_block_done = NULL;

static uint32_t i2s_isr(void *user_data)
{
    I2S_ClearIntState(APB_I2S, I2S_IRQ_TX);

    while (I2S_GetTxFIFOCount(APB_I2S) < I2S_FIFO_DEPTH)
    {
        if (NULL == p_curr_buf) break;
        if (p_curr_buf->len < 1)
        {
            struct buf_desc *p = p_curr_buf;
            p_curr_buf = p->next;

            p->next = p_free_desc;
            p_free_desc = p;

            GEN_OS->event_set(event_dma_block_done);
            continue;
        }
        I2S_PushTxFIFO(APB_I2S, p_curr_buf->p[0]);
        p_curr_buf->len--;
        p_curr_buf->p++;
    }

    if (NULL == p_curr_buf)
    {
        while (I2S_GetTxFIFOCount(APB_I2S) > 0) ;
        I2S_Enable(APB_I2S, 0, 0);
        GEN_OS->event_set(event_dma_block_done);
    }

    return 0;
}

static void init_sink(void)
{
    // Workaround bug in Cube v0.9.26: I2S DATA OUT not set
    PINCTRL_SetPadMux(21, IO_SOURCE_I2S_DATA_OUT);

    I2S_Config(APB_I2S, I2S_ROLE_MASTER, I2S_MODE_STANDARD, 1, 0, 0, 0, 16);
    I2S_Enable(APB_I2S, 0, 0);

    event_dma_block_done = GEN_OS->event_create();

    platform_set_irq_callback(PLATFORM_CB_IRQ_I2S, i2s_isr, NULL);
}

static uint8_t is_playing()
{
    return APB_I2S->ModeConfig & 1;
}

static void start_play(void)
{
    I2S_Enable(APB_I2S, 0, 0);
    I2S_ConfigIRQ(APB_I2S, 1, 0, 12, 0);

    I2S_ClearTxFIFO(APB_I2S);
    I2S_ClearRxFIFO(APB_I2S);

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

    p_curr_buf = NULL;
    p_last_buf = NULL;
    p_free_desc = buf_descs;

    for (i = 0; i < BUF_DESC_NUM; i++)
    {
        buf_descs[i].buf = audio_buffer[i];
        buf_descs[i].next = i < BUF_DESC_NUM - 1 ? &buf_descs[i + 1] : NULL;
    }
}

static void append_block(struct buf_desc *desc)
{
    desc->next = NULL;
    desc->p    = desc->buf;

    if (p_curr_buf == NULL)
    {
        GEN_OS->enter_critical();
        p_last_buf = desc;
        p_curr_buf = desc;
        GEN_OS->leave_critical();
    }

    p_last_buf->next = desc;
    p_last_buf       = desc;
}

static int do_save_pcm_samples(const int16_t *pcm_samples, int number, int acc_number)
{
    if (acc_number == 0)
    {
        prepare();
    }

    if (number & 1) number--;

    while (number > 0)
    {
        if (NULL == p_free_desc)
        {
            if (is_playing() == 0)
            {
                start_play();
            }
            GEN_OS->event_wait(event_dma_block_done);
        }

        if (NULL == p_free_desc)
            continue;

        GEN_OS->enter_critical();
        struct buf_desc *desc = p_free_desc;
        p_free_desc = desc->next;
        GEN_OS->leave_critical();

        int block = number <= BUF_BLOCK_SIZE ? number : BUF_BLOCK_SIZE;

        memcpy(desc->buf, pcm_samples, block * sizeof(pcm_samples[0]));
        desc->len = block / 2;

        append_block(desc);

        number -= block;
        pcm_samples += block;
    }

    return 0;
}
