#ifndef _audio_tts_h
#define _audio_tts_h

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define TTS_SAMPLE_RATE     (16000)

struct voice_definition;
struct tts_context;

/**
 * @brief Retrieves the context size based on the maximum number of syllables.
 *
 * This function calculates the context size required for text-to-speech (TTS) processing
 * given a specified maximum number of syllables.
 *
 * @param max_syllables     The maximum number of syllables to consider in the context.
 *
 * @return The calculated context size.
 *
 * @note The function assumes that the input value for max_syllables is non-negative.
 */
int tts_get_context_size(int max_syllables);

/**
 * @brief Retrieves the size of the scratch memory 1 required by the Text-to-Speech (TTS) engine.
 *
 * This function returns the size of the scratch memory buffer that the TTS engine needs
 * during synthesizing.
 *
 * Scratch memory 1 should not be modified elsewhere until `tts_synthesize` completes.`
 *
 * @return The size of the scratch memory required, in bytes.
 */
int tts_get_scratch_mem1_size(void);

/**
 * @brief Retrieves the size of the scratch memory 2 required by the Text-to-Speech (TTS) engine.
 *
 * This function returns the size of the scratch memory buffer that the TTS engine needs
 * during synthesizing.
 *
 * Scratch memory 2 can be used in the callback of `tts_synthesize`.
 *
 * @return The size of the scratch memory required, in bytes.
 */
int tts_get_scratch_mem2_size(void);

/**
 * @brief Initializes a Text-to-Speech (TTS) context.
 *
 * This function initializes a TTS context with the specified voice definition,
 * maximum number of syllables, and buffer. The TTS context is used to manage
 * and process TTS operations.
 *
 * To destroy the context, just free the buffer (`buf`).
 *
 * @param[in] voice         Pointer to the voice definition to be used for TTS.
 * @param[in] max_syllables Maximum number of syllables that can be processed.
 * @param[in] buf           Pointer to the buffer where TTS data will be stored.
 *
 * @return A pointer to the initialized TTS context on success, or NULL on failure.
 *
 * @note The caller is responsible for ensuring that the provided buffer is large
 *       enough (`tts_get_context_size(max_syllables)`) to hold the TTS context.
 */
struct tts_context *tts_init(const struct voice_definition *voice, int max_syllables, void *buf);

/**
 * @brief Resets the Text-to-Speech (TTS) context.
 *
 * This function resets the TTS context to its initial state, clearing any
 * previous settings or data. After calling this function, the TTS context
 * should be ready to process new text input.
 *
 * @param ctx               Pointer to the TTS context structure that needs to be reset.
 *
 * @return Returns 0 on success, or non-0 error code if the reset operation fails.
 *
 * @note This function should be called before starting a new TTS session.
 * @warning Ensure that the TTS context is properly initialized before calling this function.
 */
int tts_reset(struct tts_context *ctx);

/**
 * @brief Pushes a UTF-8 encoded string to current TTS (Text-to-Speech) session.
 *
 * This function adds the provided UTF-8 encoded string to the TTS engine for processing.
 * The string is expected to be null-terminated.
 *
 * Direct pinyin synthesis is also supported by quoting them with `[]` in `utf8_str`.
 *
 * @param ctx               Pointer to the TTS context structure.
 * @param utf8_str          Pointer to the null-terminated UTF-8 encoded string to be pushed.
 *
 * @return Returns 0 on success, or non-0 error code if the operation fails.
 *
 * @note The function does not modify the input string.
 * @warning The function assumes that the TTS context is properly initialized.
 */
int tts_push_utf8_str(struct tts_context *ctx, const char *utf8_str);

/**
 * @brief Pushes an integer to current TTS (Text-to-Speech) session.
 *
 * @param ctx               Pointer to the TTS context structure.
 * @param value             the integer value
 *
 * @return Returns 0 on success, or non-0 error code if the operation fails.
 *
 * @warning The function assumes that the TTS context is properly initialized.
 */
int tts_push_integer(struct tts_context *ctx, int64_t value);

/**
 * @brief Pushes the specified amount of yuan, jiao, and fen (元角分) into current TTS session.
 *
 * This function adds the given amount of yuan, jiao, and fen to the TTS context,
 * which will be synthesized as "X 元 Y 角 Z 分".
 *
 * @param ctx           Pointer to the TTS context structure.
 * @param yuan          The amount of yuan.
 * @param jiao          The amount of jiao.
 * @param fen           The amount of fen.
 *
 * @return Returns 0 on success, or error code on failure.
 */
int tts_push_yuan_jiao_fen(struct tts_context *ctx, int64_t yuan, uint8_t jiao, uint8_t fen);

/**
 * @brief Typedef for a function pointer that receives PCM samples synthesize by
 *        a TTS (Text-to-Speech) engine.
 *
 * This function pointer type is used to define a callback that will be invoked by the TTS engine
 * to deliver PCM (Pulse-Code Modulation) audio samples. The callback is responsible for processing
 * the received PCM samples, which are typically used for audio playback.
 *
 * @param ctx               Pointer to the TTS context structure.
 * @param pcm_samples       Pointer to an array of 16-bit PCM samples. These samples represent the audio
 *                          data generated by the TTS engine.
 * @param number            The number of PCM samples in the `pcm_samples` array.
 * @param acc_number        The accumulated number of already generated PCM samples in previous calls.
 *                          When this callback is invoked for the 1st time, `acc_number` is 0.
 * @param user_data         A pointer to user-defined data. This can be used to pass additional context or
 *                          state information to the callback function.
 *
 * @return                  An integer value indicating the result of the operation:
 *                          0: OK or success
 *                          other: abort synthesis.
 */
typedef int (*f_tts_receive_pcm_samples)(struct tts_context *ctx, const int16_t *pcm_samples, int number, int acc_number, void *user_data);

/**
 * @brief (Method #1) Synthesizes text-to-speech (TTS) audio and sends PCM samples to a callback function.
 *
 * This function takes a TTS context, a callback function to receive PCM samples,
 * user-provided data, and a scratch buffer. It synthesizes the text-to-speech audio
 * and invokes the callback function with the generated PCM samples.
 *
 * Pseudo code of `tts_synthesize`:
 *
 * ```
 * tts_synthesize(...)
 * {
 *     acc_number = 0;
 *     while (!aborted && !done)
 *     {
 *         synthesize n samples;
 *         r = rx_samples(samples, n, acc_number);
 *         if (r != 0) return r;
 *         acc_number += n;
 *     }
 * }
 * ```
 *
 * @param[in] ctx           Pointer to the TTS context structure.
 * @param[in] rx_samples    Callback function to receive PCM samples.
 *                          When a non-0 value is returned by `rx_samples`, synthesis is aborted.
 * @param[in] user_data     User-provided data to be passed to the callback function.
 * @param[in] scratch1      Scratch memory 1 for internal use during synthesis.
 * @param[in] scratch2      Scratch memory 2 for internal use during synthesis.
 *
 * @return Returns 0 on success, or non-0 error code on failure.
 */
int tts_synthesize(struct tts_context *ctx, f_tts_receive_pcm_samples rx_samples,
    void *user_data, void *scratch1, void *scratch2);

/**
 * @brief Aborts the Text-to-Speech (TTS) operation.
 *
 * This function works asynchronously: it sets an internal abort flag to true, and returns.
 *
 * @param ctx Pointer to the TTS context structure.
 */
void tts_abort(struct tts_context *ctx);

struct tts_synthesizer;

/**
 * @brief A function pointer type for a TTS (Text-To-Speech) synthesizer callback.
 *
 * This callback is used by the TTS synthesizer to notify the caller about certain events
 * or to pass synthesized data.
 *
 * @param synthesizer       A pointer to the TTS synthesizer instance that triggered the callback.
 * @param user_data         A pointer to user-defined data passed during the registration of the callback.
 *
 * @return An integer value as the return code of the callback. This could indicate success, failure,
 *         or other specific status codes depending on the implementation.
 *
 * @note   `synthesizer` only exists within the scope of this function. When this
 *         function returns, the synthesizer instance is destroyed.
 *
 * @see tts_synthesizer_run
 */
typedef int (*f_tts_synthesizer_callback)(struct tts_synthesizer *synthesizer, void *user_data);

/**
 * @brief (Method #2) Run the TTS (Text-To-Speech) synthesizer.
 *
 * This function initializes the TTS synthesizer with the provided context and callbacks.
 * The callback can then call TTS synthesizer APIs to get PCM samples.
 *
 * @param ctx               A pointer to the TTS context. This context holds the necessary information
 *                          for the synthesizer to operate.
 * @param callback          A callback function that will be invoked by the synthesizer.
 * @param user_data         User-provided data to be passed to the callback function.
 * @param scratch1          Scratch memory 1 for internal use during synthesis.
 * @param scratch2          Scratch memory 2 for internal use during synthesis.
 *
 * @return the result of `callback`
 *
 * @see f_tts_synthesizer_callback
 */
int tts_synthesizer_run(struct tts_context *ctx, f_tts_synthesizer_callback callback,
    void *user_data, void *scratch1, void *scratch2);

/**
 * @brief (Method #2) Continue the TTS (Text-to-Speech) synthesis process.
 *
 * This function continues the synthesis process from the given `tts_synthesizer`
 * instance and synthesize a number of samples in the output buffer.
 *
 * This function can only be called in the callback of `tts_synthesizer_run`.
 *
 * @param synthesizer       Pointer to the TTS synthesizer instance.
 * @param[out] number       Number of samples in the output buffer is stored.
 *
 * @return Pointer to the output audio buffer containing the synthesized audio
 *         samples in int16_t format, or NULL if the synthesis is complete.
 *
 * @note The memory pointed to by the returned pointer is managed by the
 *       synthesizer and should not be changed by the caller.
 */
const int16_t *tts_synthesizer_continue(struct tts_synthesizer *synthesizer, int *number);

/**
 * @brief (Method #2) Restart the TTS synthesizer.
 *
 * This function restarts the TTS (Text-to-Speech) synthesizer. It resets the
 * internal state of the synthesizer and gets ready to start from the very
 * beginning.
 *
 * @param synthesizer   A pointer to the TTS synthesizer structure.
 *
 * @note  Developers do not need to call this in the `f_tts_synthesizer_callback`.
 *        If the wave needs to be generated again from the beginning, then this
 *        function can be used.
 */
void tts_synthesizer_restart(struct tts_synthesizer *synthesizer);

/**
 * @brief Tune the Text-to-Speech (TTS) engine.
 *
 * @param[in] ctx           Pointer to the TTS context structure.
 * @param[in] tune          The tuning parameter to be applied.
 *                          Default: 8
 *
 * @return void
 *
 * @warning Incorrect values for the tuning parameter may result in undesirable voice output or engine malfunction.
 */
void tts_tune(struct tts_context *ctx, uint8_t tune);

#ifdef __cplusplus
}
#endif

#endif
