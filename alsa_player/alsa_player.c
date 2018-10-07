#include <alsa/asoundlib.h>
#include <getopt.h>

#include "wave_format.h"

#define DEFAULT_DEVICE "plughw:0,0"

typedef struct config {
  const char *file_path;
  char       *device;
  int         resample;
} config_s;

int wave_read_header(fmt_subchunk_data_s *fmtdata, audio_info_s *audio_info) {
  uint32_t chunk_id;
  uint32_t chunk_size;
  GUID sub_format;

  lseek(audio_info->fd, 0, SEEK_SET);

  /* read RIFF chunk */
  read(audio_info->fd, &chunk_id, sizeof(uint32_t));
  read(audio_info->fd, &chunk_size, sizeof(uint32_t));
  if (chunk_id != *(uint32_t *)RIFF_ID) {
    fprintf(stderr, "not RIFF\n");
    return EXIT_FAILURE;
  }

  /* read WAVE ID */
  read(audio_info->fd, &chunk_id, sizeof(uint32_t));
  if (chunk_id != *(uint32_t *)WAVE_ID) {
    fprintf(stderr, "not WAVE\n");
    return EXIT_FAILURE;
  }

  /* read fmt sub-chunk and data sub-chunk */
  while (1) {
    read(audio_info->fd, &chunk_id, sizeof(uint32_t));
    read(audio_info->fd, &chunk_size, sizeof(uint32_t));
    if (chunk_id == *(uint32_t *)FMT_ID) {
      if ((chunk_size != FORMAT_CHUNK_PCM_SIZE)  && (chunk_size != FORMAT_CHUNK_EX_SIZE)
         && (chunk_size != FORMAT_CHUNK_EXTENSIBLE_SIZE)) {
        fprintf(stderr, "invalid chunk size %d\n", chunk_size);
        return EXIT_FAILURE;
      }

      /* read format information */
      read(audio_info->fd, fmtdata, FORMAT_CHUNK_PCM_SIZE);
      if ((fmtdata->format_tag != WAVE_FORMAT_PCM) && (fmtdata->format_tag != WAVE_FORMAT_EXTENSIBLE)) {
        fprintf(stderr, "format code %x, not PCM\n", fmtdata->format_tag);
        return EXIT_FAILURE;
      }

      switch(chunk_size) {
        case FORMAT_CHUNK_EXTENSIBLE_SIZE:
          lseek(audio_info->fd, 8, SEEK_CUR);
          read(audio_info->fd, &sub_format, sizeof(GUID));
          if ( sub_format.sub_format_code != WAVE_FORMAT_PCM) {
            fprintf(stderr, "ex sub-format code %x, not LPCM\n",  sub_format.sub_format_code);
            return EXIT_FAILURE;
          } else if (memcmp(sub_format.wave_guid_tag, WAVE_GUID_TAG, 14) != 0) {
            fprintf(stderr, "GUID tag is not WAVE_GUID_TAG\n");
            return EXIT_FAILURE;
          } else {
            printf("chunk size %d, WAVEFORMATEXTENSIBLE LPCM\n", chunk_size);
          }
          break;
        case FORMAT_CHUNK_EX_SIZE:
          lseek(audio_info->fd, 2, SEEK_CUR);
          printf("chunk size %d, WAVEFORMATEX LPCM\n", chunk_size);
          break;
        default:
          printf("chunk size %d, standard WAVE LPCM\n", chunk_size);
          break;
      }
    } else if (chunk_id == *(uint32_t *)DATA_ID) {
      audio_info->frame_size = (long)chunk_size / (long)fmtdata->block_align;
      break;
    } else {
      /* skip */
      lseek(audio_info->fd, (off_t)chunk_size, SEEK_CUR);
    }
  }
  return 0;
}

snd_pcm_format_t get_format_type(uint16_t bits_per_sample) {
  snd_pcm_format_t format = SND_PCM_FORMAT_UNKNOWN;

  switch (bits_per_sample) {
    case 16:
      format = SND_PCM_FORMAT_S16_LE;
      break;
    case 24:
      format = SND_PCM_FORMAT_S24_3LE;
      break;
    case 32:
      format = SND_PCM_FORMAT_S32_LE;
      break;
    default:
      abort();
      break;
  }

  return format;
}

int set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *hwparams, config_s cfg, fmt_subchunk_data_s fmtdata, audio_info_s *audio_info) {
  int ret, dir;

  ret = snd_pcm_hw_params_any(handle, hwparams);
  if (ret < 0) {
    fprintf(stderr, "HW setting error: %s\n", snd_strerror(ret));
    return ret;
  }
  ret = snd_pcm_hw_params_set_rate_resample(handle, hwparams, cfg.resample);
  if (ret < 0) {
    fprintf(stderr, "resample error: %s\n", snd_strerror(ret));
    return ret;
  }

  ret = snd_pcm_hw_params_set_access(handle, hwparams,
                                       SND_PCM_ACCESS_RW_INTERLEAVED);
  if (ret < 0) {
    fprintf(stderr, "access type setting error %s\n", snd_strerror(ret));
    return ret;
  }

  ret = snd_pcm_hw_params_set_format(handle, hwparams, get_format_type(fmtdata.bits_per_sample));
  if (ret < 0) {
    fprintf(stderr, "sample format setting error: %s\n", snd_strerror(ret));
    fprintf(stderr, "support format:\n");
    for (int fmt = 0; fmt <= SND_PCM_FORMAT_LAST; fmt++) {
      if (snd_pcm_hw_params_test_format(handle, hwparams, (snd_pcm_format_t)fmt) == 0)
        fprintf(stderr, "- %s\n", snd_pcm_format_name((snd_pcm_format_t)fmt));
    }
    return ret;
  }

  ret = snd_pcm_hw_params_set_channels(handle, hwparams, (unsigned int)fmtdata.channels);
  if (ret < 0) {
    fprintf(stderr, "unsupport the number of channels (%hu): %s\n", fmtdata.channels, snd_strerror(ret));
    return ret;
  }
  uint32_t rate_near = fmtdata.samples_per_sec;
  ret = snd_pcm_hw_params_set_rate_near(handle, hwparams, &rate_near, 0);
  if (ret < 0) {
    fprintf(stderr, "unsupport %uHz: %s\n", fmtdata.samples_per_sec, snd_strerror(ret));
    return ret;
  }
  if (rate_near != fmtdata.samples_per_sec) {
    fprintf(stderr, "not match (request %uHz, getting value %uHz)\n", fmtdata.samples_per_sec, rate_near);
    return -EINVAL;
  }

  unsigned int buffer_time_us = 0;
  unsigned int period_time_us = 0;
  ret = snd_pcm_hw_params_get_buffer_time_max(hwparams, &buffer_time_us, &dir);
  if (buffer_time_us > 500000)
    buffer_time_us = 500000;
  if (buffer_time_us > 0) {
    period_time_us = buffer_time_us / 4;
  } else {
    fprintf(stderr, "error: invalid buffer_time_us\n");
    return -EINVAL;
  }

  ret = snd_pcm_hw_params_set_buffer_time_near(handle, hwparams, &buffer_time_us, &dir);
  if (ret < 0) {
    fprintf(stderr, "buffer time setting error %i : %s\n", buffer_time_us, snd_strerror(ret));
    return ret;
  }

  ret = snd_pcm_hw_params_set_period_time_near(handle, hwparams, &period_time_us, &dir);
  if (ret < 0) {
    fprintf(stderr, "period time setting error %i : %s\n", period_time_us, snd_strerror(ret));
    return ret;
  }

  ret = snd_pcm_hw_params(handle, hwparams);
  if (ret < 0) {
    fprintf(stderr, "HW param setting error %s\n", snd_strerror(ret));
    return ret;
  }

  ret = snd_pcm_hw_params_get_buffer_size(hwparams, &audio_info->buffer_size);
  if (ret < 0) {
    fprintf(stderr, "cannot get buffer size : %s\n", snd_strerror(ret));
    return ret;
  }
  ret = snd_pcm_hw_params_get_period_size(hwparams, &audio_info->period_size, &dir);
  if (ret < 0) {
    fprintf(stderr, "cannot get period size : %s\n", snd_strerror(ret));
    return ret;
  }
  return 0;
}

int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams, audio_info_s audio_info) {
  int ret;

  ret = snd_pcm_sw_params_current(handle, swparams);
  if (ret < 0) {
    fprintf(stderr, "error(SW param) %s\n", snd_strerror(ret));
    return ret;
  }

  ret = snd_pcm_sw_params_set_start_threshold(handle, swparams, (audio_info.buffer_size / audio_info.period_size) * audio_info.period_size);
  if (ret < 0) {
    fprintf(stderr, "threshold setting error %s\n", snd_strerror(ret));
    return ret;
  }

  ret = snd_pcm_sw_params_set_avail_min(handle, swparams, audio_info.period_size);
  if (ret < 0) {
    fprintf(stderr, "avail min setting error: %s\n", snd_strerror(ret));
    return ret;
  }

  ret = snd_pcm_sw_params(handle, swparams);

  if (ret < 0) {
    fprintf(stderr, "SW param setting error %s\n", snd_strerror(ret));
    return ret;
  }
  return 0;
}

int write_audio(snd_pcm_t *handle, fmt_subchunk_data_s fmtdata, audio_info_s audio_info) {
  unsigned char *buf_ptr;
  unsigned short frame_bytes =  fmtdata.block_align;
  const long sound_frames = audio_info.frame_size;
  long frames_bytes, frame_count, play_frames = 0;
  long read_frames, res_frames = sound_frames;
  int ret = 0;

  unsigned char *frame_block = (unsigned char *)malloc(audio_info.period_size * frame_bytes);
  if (frame_block == NULL) {
    fprintf(stderr, "error: malloc\n");
    ret = EXIT_FAILURE;
    goto ending;
  }

  frames_bytes = (long)(audio_info.period_size * frame_bytes);
  while (res_frames > 0) {
    read_frames = (long)(read(audio_info.fd, frame_block, (size_t)frames_bytes)/frame_bytes);
    frame_count = read_frames;
    buf_ptr = frame_block;
    while (frame_count > 0) {
      ret = (int)snd_pcm_writei(handle, buf_ptr, (snd_pcm_uframes_t)frame_count);
      if (ret == -EAGAIN)
        continue;
      if (ret < 0) {
        if (snd_pcm_recover(handle, ret, 0) < 0) {
          fprintf(stderr, "write error %s\n", snd_strerror(ret));
          goto ending;
        }
        break;  /* skip */
      }
      buf_ptr += ret * frame_bytes;
      frame_count -= ret;
    }
    play_frames += read_frames;

    if ((res_frames = sound_frames - play_frames) <= (long)audio_info.period_size)
      frames_bytes = (long)(res_frames * frame_bytes);
  }
  snd_pcm_drop(handle);
  printf("%lu frames\n", play_frames);
  ret = 0;

 ending:
  if (frame_block != NULL)
    free(frame_block);

  return ret;
}

void show_info(config_s cfg, fmt_subchunk_data_s fmtdata, audio_info_s audio_info, snd_pcm_t *handle, snd_output_t *output) {
  printf("*** file information ***\n");
  printf("file: %s\n", cfg.file_path);
  printf("sampling rate: %uHz\n", fmtdata.samples_per_sec);
  printf("channels: %hu\n", fmtdata.channels);
  printf("bits: %hu\n", fmtdata.bits_per_sample);
  printf("time: %.0lfs\n", (double)audio_info.frame_size / (double)fmtdata.samples_per_sec);
  printf("\n");

  printf("*** PCM info ***\n");
  snd_pcm_dump(handle, output);
  printf("\n");

  printf("*** ALSA parameter ***\n");
  printf("format: %s\n", snd_pcm_format_name(get_format_type(fmtdata.bits_per_sample)));
  printf("device: %s\n", cfg.device);
  printf("\n");
}

void run(config_s cfg) {
  int ret = 0;
  fmt_subchunk_data_s fmtdata;
  audio_info_s audio_info;

  audio_info.fd = open(cfg.file_path, O_RDONLY, 0);
  if (audio_info.fd == -1) {
    fprintf(stderr, "file open error\n");
    goto ending;
  }

  if (wave_read_header(&fmtdata, &audio_info) != 0) {
    fprintf(stderr, "error: read header\n");
    goto ending;
  }

  if (fmtdata.bits_per_sample > 32) {
    fprintf(stderr, "unsupport bit depth%d\n", fmtdata.bits_per_sample);
    goto ending;
  }

  snd_output_t *output = NULL;
  ret = snd_output_stdio_attach(&output, stdout, 0);
  if (ret < 0) {
    fprintf(stderr, "ALSA log output error %s\n", snd_strerror(ret));
    goto ending;
  }

  snd_pcm_t *handle = NULL;
  if ((ret = snd_pcm_open(&handle, cfg.device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    fprintf(stderr, "PCM open error: %s\n", snd_strerror(ret));
    goto ending;
  }

  snd_pcm_hw_params_t *hwparams;
  snd_pcm_hw_params_alloca(&hwparams);
  if ((ret = set_hwparams(handle, hwparams, cfg, fmtdata, &audio_info)) < 0) {
    fprintf(stderr, "hwparams setting error: %s\n", snd_strerror(ret));
    printf("*** PCM HW dump ***\n");
    snd_pcm_hw_params_dump(hwparams, output);
    printf("\n");
    goto ending;
  }

  snd_pcm_sw_params_t *swparams;
  snd_pcm_sw_params_alloca(&swparams);
  if ((ret = set_swparams(handle, swparams, audio_info)) < 0) {
    fprintf(stderr, "swparams setting error: %s\n", snd_strerror(ret));
    printf("*** PCM SW dump ***\n");
    snd_pcm_sw_params_dump(swparams, output);
    printf("\n");
    goto ending;
  }

  show_info(cfg, fmtdata, audio_info, handle, output);

  ret = write_audio(handle, fmtdata, audio_info);
  if (ret != 0) {
    fprintf(stderr, "transfer error\n");
  }

ending:
  if (output != NULL)
    snd_output_close(output);

  if (handle != NULL)
    snd_pcm_close(handle);

  snd_config_update_free_global();

  if (audio_info.fd != -1)
    close(audio_info.fd);
}

void usage(void) {
  int i;
  printf("usage: alsa_player [option]... file\n"
         "-D,--device=NAME   device\n"
         "-n,--noresample    no resample\n"
         "\n");
  printf("sample format:");
  for (i = 0; i < SND_PCM_FORMAT_LAST; i++) {
    const char *s = snd_pcm_format_name((snd_pcm_format_t)i);
    if (s)
      printf(" %s", s);
  }
  printf("\n");
}

int parse_args(int argc, char *argv[], config_s *cfg) {
  cfg->file_path = NULL;
  cfg->device    = NULL;
  cfg->resample  =    1;

  int c;
  const struct option long_option[] =
  {
    {    "device", 1, NULL, 'D'},
    {"noresample", 0, NULL, 'n'},
    {           0, 0,    0,   0},
  };
  while ((c = getopt_long(argc, argv, "D:n", long_option, NULL)) != -1) {
    switch (c) {
      case 'D':
        cfg->device = strdup(optarg);
        break;
      case 'n':
        cfg->resample = 0;
        break;
      default:
        return -1;
    }
  }

  if (optind > argc - 1)
    return -1;
  cfg->file_path = argv[optind];

  if (!cfg->device)
    cfg->device = DEFAULT_DEVICE;

  return 0;
}

void config_cleanup(config_s cfg) {
  if (cfg.device && strncmp(cfg.device, DEFAULT_DEVICE, strlen(DEFAULT_DEVICE)))
    free(cfg.device);
}

int main(int argc, char *argv[]) {
  int ret = 0;
  config_s cfg;

  ret = parse_args(argc, argv, &cfg);
  if (ret < 0) {
    usage();
    config_cleanup(cfg);
    return ret;
  }

  run(cfg);

  config_cleanup(cfg);
  return 0;
}
