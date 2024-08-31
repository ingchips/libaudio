#ifndef _AUDIO_DENOISE_H
#define _AUDIO_DENOISE_H
#include <stdint.h>

// DO NOT MODIFY THESE
#define AUDIO_DENOISE_BLOCK_LEN             128

#define AUDIO_DENOISE_CONTEXT_MEM_SIZE      2824
#define AUDIO_DENOISE_SCRATCH_MEM_SIZE      2048

#ifdef __cplusplus
extern "C"
{
#endif

typedef void *audio_denoise_context_t;

/**
 * @brief init context for de-noise
 *
 * @param[in]  buf          a buffer with `AUDIO_DENOISE_CONTEXT_MEM_SIZE` bytes
 *                          to hold the context
 * @param[in]  sample_rate  sample rate (supported: 8000)
 * @return                  the initialized context
 */
audio_denoise_context_t *audio_denoise_init(void *buf, uint32_t sample_rate);

/**
 * @brief reduce noise in the audio signal block by block
 *
 * This function takes ~6ms (CPU @ 112MHz), or uses ~42MHz.
 *
 * @param[in]   ctx         the de-noise context
 * @param[in]   in          input samples (number of samples = `AUDIO_DENOISE_BLOCK_LEN`)
 * @param[out]  out         output samples (number of samples = `AUDIO_DENOISE_BLOCK_LEN`)
 *                          `out` can be the same as `in` for inplace operation.
 * @param[in,out]  scratch  scratch buffer with `AUDIO_DENOISE_SCRATCH_MEM_SIZE` bytes
 */
void audio_denoise_process(audio_denoise_context_t *ctx, const int16_t *in, int16_t *out, void *scratch);

#ifdef __cplusplus
}
#endif

#endif