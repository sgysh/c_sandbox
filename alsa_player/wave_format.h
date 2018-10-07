#ifndef WAVE_FORMAT_H_
#define WAVE_FORMAT_H_

#include <alsa/asoundlib.h>
#include <stdint.h>

#define FORMAT_CHUNK_PCM_SIZE (16)
#define FORMAT_CHUNK_EX_SIZE (18)
#define FORMAT_CHUNK_EXTENSIBLE_SIZE (40)

#define WAVE_FORMAT_PCM         (0x0001)
#define WAVE_FORMAT_EXTENSIBLE  (0xfffe)
#define WAVE_GUID_TAG  "\x00\x00\x00\x00\x10\x00\x80\x00\x00\xAA\x00\x38\x9B\x71"

/* Chunk ID */
static int8_t RIFF_ID[4] = {'R', 'I', 'F', 'F'};
static int8_t WAVE_ID[4] = {'W', 'A', 'V', 'E'};
static int8_t FMT_ID[4]  = {'f', 'm', 't', ' '};
static int8_t DATA_ID[4] = {'d', 'a', 't', 'a'};

/* Globally Unique IDentifier (GUID) */
typedef struct guid {
  uint16_t sub_format_code;
  uint8_t  wave_guid_tag[14] ;
} GUID;

typedef struct fmt_subchunk_data {
  uint16_t format_tag;
  uint16_t channels;
  uint32_t samples_per_sec;
  uint32_t avg_bytes_per_sec;
  uint16_t block_align;
  uint16_t bits_per_sample;
} fmt_subchunk_data_s;

typedef struct audio_info {
  int   fd;
  long  frame_size;
  snd_pcm_uframes_t buffer_size;
  snd_pcm_uframes_t period_size;

} audio_info_s;

#endif  // WAVE_FORMAT_H_

