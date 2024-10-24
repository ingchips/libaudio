#ifndef _audio_amr_wb_h
#define _audio_amr_wb_h

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* Frame size at 16kHz                        */
#define AMR_WB_PCM_FRAME_16k   320

/**
 * @defgroup AWR-WB bit stream format
 */
/**@{*/

/*
*    One word (2-byte) to indicate type of frame type.
*    One word (2-byte) to indicate frame type.
*    One word (2-byte) to indicate mode.
*    N words (2-byte) containing N bits (bit 0 = 0xff81, bit 1 = 0x007f).
*/
#define AMR_WB_BIT_STREAM_FORMAT_ETS            0

/*
*    One word (2-byte) for sync word (good frames: 0x6b21, bad frames: 0x6b20)
*    One word (2-byte) for frame length N.
*    N words (2-byte) containing N bits (bit 0 = 0x007f, bit 1 = 0x0081).
*/
#define AMR_WB_BIT_STREAM_FORMAT_ITU            1

/*
*   AMR-WB MIME/storage format, see RFC 3267 (sections 5.1 and 5.3) for details
*/
#define AMR_WB_BIT_STREAM_FORMAT_MIME_IETF      2
/**@}*/

/*
*   AMR-WB modes (i.e. bit-rate)
*/
#define AMR_WB_MODE_6600bps                       0
#define AMR_WB_MODE_8850bps                       1
#define AMR_WB_MODE_12650bps                      2
#define AMR_WB_MODE_14250bps                      3
#define AMR_WB_MODE_15850bps                      4
#define AMR_WB_MODE_18250bps                      5
#define AMR_WB_MODE_19850bps                      6
#define AMR_WB_MODE_23050bps                      7
#define AMR_WB_MODE_23850bps                      8

int amr_wb_get_frame_header_size(int bit_stream_format);

/**
 * @defgroup AWR-WB encoder
 */
/**@{*/

struct amr_wb_encoder;

/**
 * @brief Retrieves the context size required for the AMR-WB encoder.
 *
 * This function returns the size of the context structure needed by the AMR-WB encoder.
 *
 * @return The size of the context structure in bytes.
 */
int amr_wb_encoder_get_context_size(void);

/**
 * @brief Retrieves the size of the scratch memory required by the AMR-WB encoder.
 *
 * This function returns the size in bytes of the scratch memory that needs to be allocated
 * for the AMR-WB encoder to operate correctly. The scratch memory is used for internal
 * computations and temporary storage during the encoding process.
 *
 * @return The size of the scratch memory in bytes.
 */
int amr_wb_encoder_get_scratch_mem_size(void);

/**
 * @brief Initializes an AMR-WB encoder.
 *
 * This function initializes an AMR-WB encoder with the specified parameters.
 *
 * @param bit_stream_format     The format of the bit stream (see `AMR_WB_BIT_STREAM_FORMAT_xxx`).
 * @param allow_dtx             Flag indicating whether Discontinuous Transmission (DTX) is allowed.
 * @param mode                  The default encoding mode (See `AMR_WB_MODE_xxx`).
 * @param buf                   Pointer to the buffer where the context data will be stored.
 *
 * @return A pointer to the initialized AMR-WB encoder structure, or NULL if initialization fails.
 *
 * @note The buffer pointed to by `buf` should be large enough (`amr_wb_encoder_get_scratch_mem_size()`)
 * to hold the context data.
 *
 * @warning The function does not check the validity of the buffer size.
 */
struct amr_wb_encoder *amr_wb_encoder_init(int bit_stream_format, int allow_dtx, int mode, void *buf);

/**
 * @brief Encodes a frame of PCM samples into AMR-WB format.
 *
 * This function encodes a frame of 16-bit PCM audio samples.
 * The encoded frame is stored in the provided output buffer.
 *
 * @param[in] ctx           Pointer to the AMR-WB encoder context.
 * @param[in] pcm_samples   Pointer to the array of 16-bit PCM audio samples.
 * @param[out] output       Pointer to the buffer where the encoded AMR-WB frame will be stored.
 * @param[in,out] scratch   Pointer to a scratch buffer used internally by the encoder.
 *
 * @note The buffer pointed to by `scratch` should be large enough (`amr_wb_encoder_get_context_size()`).
 *
 * @return                  Length of encoded frame (header + payload) in bytes.
 */
int amr_wb_encoder_encode_frame(struct amr_wb_encoder *ctx, const int16_t *pcm_samples,
                                uint8_t *output, void *scratch);

/**
 * @brief Switches mode and encodes a frame of PCM samples into AMR-WB format.
 *
 * This function firstly changes encoding mode then encodes a frame.
 * This function exposes the variable (or adaptive) rate feature of AMR-WB.
 *
 * @param[in] ctx           Pointer to the AMR-WB encoder context.
 * @param[in] mode          Set to a new encoding mode (See `AMR_WB_MODE_xxx`).
 * @param[in] pcm_samples   Pointer to the array of 16-bit PCM audio samples.
 * @param[out] output       Pointer to the buffer where the encoded AMR-WB frame will be stored.
 * @param[in,out] scratch   Pointer to a scratch buffer used internally by the encoder.
 *
 * @note The buffer pointed to by `scratch` should be large enough (`amr_wb_encoder_get_context_size()`).
 *
 * @return                  Length of encoded frame (header + payload) in bytes.
 */
int amr_wb_encoder_encode_frame2(struct amr_wb_encoder *ctx, int mode,
                                const int16_t *pcm_samples, uint8_t *output, void *scratch);

/**@}*/
/**
 * @defgroup AWR-WB decoder
 */
/**@{*/

struct amr_wb_decoder;

/**
 * @brief Retrieves the context size required for the AMR-WB decoder.
 *
 * This function returns the size of the context structure needed by the AMR-WB decoder.
 *
 * @return The size of the context structure in bytes.
 */
int amr_wb_decoder_get_context_size(void);

/**
 * @brief Retrieves the size of the scratch memory required by the AMR-WB decoder.
 *
 * This function returns the size in bytes of the scratch memory that needs to be allocated
 * for the AMR-WB decoder to operate correctly. The scratch memory is used for internal
 * computations and temporary storage during the decoding process.
 *
 * @return The size of the scratch memory in bytes.
 */
int amr_wb_decoder_get_scratch_mem_size(void);

/**
 * @brief Initializes an AMR-WB decoder.
 *
 * This function initializes an AMR-WB decoder with the specified parameters.
 *
 * @param bit_stream_format     The format of the bit stream (see `AMR_WB_BIT_STREAM_FORMAT_xxx`).
 * @param buf                   Pointer to the buffer where the context data will be stored.
 *
 * @return A pointer to the initialized AMR-WB decoder structure, or NULL if initialization fails.
 *
 * @note The buffer pointed to by `buf` should be large enough (`amr_wb_decoder_get_context_size()`)
 * to hold the context data.
 *
 * @warning The function does not check the validity of the buffer size.
 */
struct amr_wb_decoder *amr_wb_decoder_init(int bit_stream_format, void *buf);


/**
 * @brief Probes a frame header from the given stream
 *
 * This function decodes frame header and determines basic parameters of the frame.
 *
 * @param ctx               Pointer to the AMR-WB decoder context.
 * @param[in] stream        Pointer to the input stream of data to be probed.
 *
 * @return                  Payload length of the frame.
 *                          A negative value is returned when error occurs.
 */
int amr_wb_decoder_probe(struct amr_wb_decoder *ctx, const uint8_t *stream);

/**
 * @brief Decodes/Synthesizes a frame of AMR-WB encoded audio.
 *
 * This function decodes a single frame of AMR-WB encoded audio data and stores
 * the resulting PCM samples in the provided buffer. The function uses a scratch
 * buffer for intermediate computations.
 *
 * Example:
 *
 * ```c
 * const uint8_t *stream = start of an encoded stream
 * const int header_size = amr_wb_get_frame_header_size(...);
 * int payload_len = amr_wb_decoder_probe(ctx, stream);
 * stream += header_size; // move to the start of payload
 * int decoded_samples = amr_wb_decoder_decode_frame(ctx, stream + header_size, ....);
 *
 * // move to the start of next frame (header of next frame)
 * stream += payload_len;
 * ```
 *
 * @param[in]  ctx          Pointer to the AMR-WB decoder context.
 * @param[in]  payload      Pointer to the input AMR-WB encoded payload.
 * @param[out] synth_pcm    Pointer to the buffer where the decoded PCM samples will be stored.
 * @param[in]  scratch      Pointer to the scratch buffer used for intermediate computations.
 *
 * @return                  Returns number of PCM samples that are decoded.
 *                          (i.e. `AMR_WB_PCM_FRAME_16k`)
 */
int amr_wb_decoder_decode_frame(struct amr_wb_decoder *ctx, const uint8_t *payload, int16_t *synth_pcm,
                                void *scratch);

/**@}*/

#ifdef __cplusplus
}
#endif
#endif
