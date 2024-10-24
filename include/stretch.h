////////////////////////////////////////////////////////////////////////////
//                        **** AUDIO-STRETCH ****                         //
//                      Time Domain Harmonic Scaler                       //
//                    Copyright (c) 2022 David Bryant                     //
//                          All Rights Reserved.                          //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

// stretch.h

// Time Domain Harmonic Compression and Expansion
//
// This library performs time domain harmonic scaling with pitch detection
// to stretch the timing of a 16-bit PCM signal (either mono or stereo) from
// 1/2 to 2 times its original length. This is done without altering any of
// its tonal characteristics.
//
// Support of stereo is disabled.

#ifndef STRETCH_H
#define STRETCH_H

#include <stdint.h>

// use "fast" version of period determination code
#define STRETCH_FAST_FLAG               0x1

#define STRETCH_DEF_FREQ_RANGE          55,333

#ifdef __cplusplus
extern "C" {
#endif

struct stretch_ctx_t;

/**
 * Terminology:
 *
 * Stretch "speed" is roughly (original audio length) / (new audio length).
 * 1.0 for no change, <1.0 for stretching, and >1.0 for compressing.
 * For example, 1.2 means the original audio is made about 20% shorter (or faster).
 */

/**
 * @brief Calculates the context size based on frequency range and flags.
 *
 * This function computes the context size required for a given range of frequency and specific flags.
 *
 * @param sampling_rate     The sampling rate of the audio data in Hz.
 * @param lower_freq        The lower frequency boundary for the processing in Hz.
 * @param upper_freq        The upper frequency boundary for the processing in Hz.
 * @param flags             Flags that influence the context size calculation.
 *                          See `STRETCH_..._FLAG`.
 *
 * @return The calculated context size in bytes.
 */

int stretch_get_context_size(int sampling_rate, int lower_freq, int upper_freq, int flags);

/**
 * @brief Initializes a stretch context.
 *
 * @param sampling_rate     The sampling rate of the audio data in Hz.
 * @param lower_freq        The lower frequency boundary for the processing in Hz.
 * @param upper_freq        The upper frequency boundary for the processing in Hz.
 * @param flags             Flags that influence the context size calculation.
 *                          See `STRETCH_..._FLAG`.
 * @param buf               A buffer pointer that may be used internally by the stretching context.
 *
 * @return A pointer to the initialized `stretch_ctx_t` structure, or NULL if some parameters
 * are out of range.
 *
 * @note The buffer provided in `buf` should be appropriately allocated by the caller, the size
 * of which can be got by `stretch_get_context_size(sampling_rate, lower_freq, upper_freq, flags)`.
 */
struct stretch_ctx_t *stretch_init(int sampling_rate, int lower_freq, int upper_freq, int flags, void *buf);

/**
 * @brief Get maximum number of output samples based on the maximum number of samples and the minimum stretch speed.
 *
 * @param ctx               The stretch context.
 * @param max_num_samples   The maximum number of samples that will be fed into `stretch_samples`.
 * @param min_speed         The minimum speed that needs to be supported.
 *
 * @return Returns maximum number of output samples when calling `stretch_samples`.
 */
int stretch_get_output_capacity(struct stretch_ctx_t *ctx, int max_num_samples, float min_speed);

/**
 * @brief Stretch the input audio samples by the given ratio.
 *
 * This function resamples the input audio samples to achieve a desired stretch
 * ratio. The input samples are processed and the resulting samples are stored
 * in the output buffer.
 *
 * @param[in] ctx         The stretch context.
 * @param[in] samples     Input audio samples.
 * @param[in] num_samples Number of input samples.
 * @param[out] output     Output buffer where the stretched samples will be stored.
 * @param[in] speed       The stretch speed.
 *
 * @return The number of output samples generated.
 *
 * @note The output buffer should be large enough to hold the resulting samples,
 *       which can be got by `stretch_get_output_capacity()`.
 *
 * @see stretch_ctx_t
 */
int stretch_samples(struct stretch_ctx_t *ctx, const int16_t *samples,
                    int num_samples, int16_t *output, float speed);

/**
 * @brief Flushes the remaining samples to the output buffer.
 *
 * This function processes any remaining samples and writes them to the provided
 * output buffer. It is typically called after all input samples have been
 * processed to ensure that no samples are left unprocessed.
 *
 * @param[in] ctx       The stretch context structure.
 * @param[out] output   The output buffer where the processed samples will be written.
 *
 * @return Returns the number of samples written to the output buffer.
 *
 * @note The output buffer should be large enough to hold the resulting samples,
 *       which can be got by `stretch_get_output_capacity()`.
 */
int stretch_flush(struct stretch_ctx_t *ctx, int16_t *output);

/**
 * @brief Resets the stretch context.
 *
 * This function resets the state of the stretch context, clearing any previous
 * settings or configurations. After calling this function, the context is
 * ready to be used for new operations.
 *
 * @param ctx   The stretch context structure to be reset.
 *
 * @return void
 */
void stretch_reset(struct stretch_ctx_t *ctx);


#ifdef __cplusplus
}
#endif

#endif
