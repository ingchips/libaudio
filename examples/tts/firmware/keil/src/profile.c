#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "platform_api.h"

#include "tts.h"
#include "stretch.h"
#include "amr_wb.h"

struct pcm_play_context
{
    float speed;
    struct stretch_ctx_t *stretcher;
    int16_t *stretch_output;
    int acc_number;

    uint8_t use_stretch;
};

#if (WAVE_SINK == 0)
#include "sink_uart.inc"
#define STRETCHER_PARAMS        16000, STRETCH_DEF_FREQ_RANGE, 0
#elif (WAVE_SINK == 1)
#include "sink_i2s.inc"
#define STRETCHER_PARAMS        16000, 55,200, 0
#elif (WAVE_SINK == 2)
#include "sink_i2s_dma.inc"
#define STRETCHER_PARAMS        16000, 55,200, 0
#endif

static void input_from_uart(UART_TypeDef *port, char *input, int max_len)
{
    int i = 0;
    while (i < max_len - 1)
    {
        while (apUART_Check_RXFIFO_EMPTY(port) == 1);
        input[i] = (char)port->DataRead;
        if (('\n' == input[i]) || ('\r' == input[i]))
            break;
        else
            i++;
    }
    input[i] = '\0';

    while (apUART_Check_RXFIFO_EMPTY(port) != 1)
    {
        volatile char c = (char)port->DataRead;
    }
}

static int save_pcm_samples(struct tts_context *ctx, const int16_t *pcm_samples, int number, int acc_number, void *user_data)
{
    int r = 0;
    struct pcm_play_context *cfg = (struct pcm_play_context *)user_data;

    if (0 == acc_number)
    {
        cfg->use_stretch = 0;
        cfg->acc_number  = 0;
        if (cfg->speed != 1.0f)
        {
            cfg->use_stretch = 1;
            stretch_reset(cfg->stretcher);
        }
    }

    if (cfg->use_stretch)
    {
        int n = stretch_samples(cfg->stretcher, pcm_samples, number,
                        cfg->stretch_output, cfg->speed);
        r = do_save_pcm_samples(cfg->stretch_output, n, cfg->acc_number);
        cfg->acc_number += n;
    }
    else
    {
        r = do_save_pcm_samples(pcm_samples, number, acc_number);
    }

    return r;
}

static void flush_samples(struct pcm_play_context *cfg)
{
    if (0 == cfg->use_stretch) return;

    int n = stretch_flush(cfg->stretcher, cfg->stretch_output);
    do_save_pcm_samples(cfg->stretch_output, n, cfg->acc_number);
}

void test_tts(UART_TypeDef *port)
{
    struct pcm_play_context play_context =
    {
        .use_stretch = 0,
        .speed = 1.0,
        .stretcher = NULL,
    };
    static char input[200];
    platform_printf("TTS Demo\n");

    #define MAX_SYLLABLES 100
    #define MIN_SPEED     0.8f

    const struct voice_definition * voice_def = (const struct voice_definition *)AHB_QSPI_MEM_BASE;

    void *context  = malloc(tts_get_context_size(MAX_SYLLABLES));
    void *scratch1 = malloc(tts_get_scratch_mem1_size());
    void *scratch2 = malloc(tts_get_scratch_mem2_size());

    platform_printf("TTS context        = %5d @ %p\n", tts_get_context_size(MAX_SYLLABLES), context);
    platform_printf("TTS scratch1       = %5d @ %p\n", tts_get_scratch_mem1_size(), scratch1);
    platform_printf("TTS scratch2       = %5d @ %p\n", tts_get_scratch_mem2_size(), scratch2);

    {
        int size = stretch_get_context_size(STRETCHER_PARAMS);
        void *buf = malloc(size);
        play_context.stretcher = stretch_init(STRETCHER_PARAMS, buf);
        play_context.stretch_output = scratch2;

        platform_printf("stretcher context  = %5d @ %p\n", size, buf);

        int capacity = stretch_get_output_capacity(play_context.stretcher, AMR_WB_PCM_FRAME_16k, MIN_SPEED);
        if (capacity <= tts_get_scratch_mem2_size() / sizeof(int16_t))
            platform_printf("re-use scratch2 as the output of stretcher: %d < %d\n", capacity, tts_get_scratch_mem2_size() / sizeof(int16_t));
        else
            platform_printf("this is UNEXPECTED: %d vs %d\n", capacity, tts_get_scratch_mem2_size() / sizeof(int16_t));
    }

    struct tts_context *ctx = tts_init(voice_def, MAX_SYLLABLES, context);

    while (1)
    {
        input_from_uart(port, input, sizeof(input));
        if (strlen(input) < 1) continue;
        if ('!' == input[0])
        {
            play_context.speed = atof(input + 1);
            if (play_context.speed < MIN_SPEED) play_context.speed = MIN_SPEED;
            continue;
        }

        tts_reset(ctx);
        tts_push_utf8_str(ctx, input);
        tts_synthesize(ctx, save_pcm_samples, &play_context, scratch1, scratch2);

        flush_samples(&play_context);
    }
}

uint32_t setup_profile(void *data, void *user_data)
{
    platform_printf("CPU @ %u Hz\n", SYSCTRL_GetHClk());

    init_sink();

    test_tts((UART_TypeDef *)user_data);

    return 0;
}
