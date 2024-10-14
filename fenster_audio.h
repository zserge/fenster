#ifndef FENSTER_AUDIO_H
#define FENSTER_AUDIO_H

#ifndef FENSTER_SAMPLE_RATE
#define FENSTER_SAMPLE_RATE 44100
#endif

#ifndef FENSTER_AUDIO_BUFSZ
#define FENSTER_AUDIO_BUFSZ 8192
#endif

// Platform-agnostic structure
struct fenster_audio {
  void *audio_data;
  float buf[FENSTER_AUDIO_BUFSZ];
  size_t pos;
};

#ifndef FENSTER_API
#define FENSTER_API extern
#endif

FENSTER_API int fenster_audio_open(struct fenster_audio *f);
FENSTER_API int fenster_audio_available(struct fenster_audio *f);
FENSTER_API void fenster_audio_write(struct fenster_audio *f, float *buf, size_t n);
FENSTER_API void fenster_audio_close(struct fenster_audio *f);

#ifndef FENSTER_HEADER

#if defined(__APPLE__)
#include <AudioToolbox/AudioQueue.h>
#include <dispatch/dispatch.h>

typedef struct {
  AudioQueueRef queue;
  dispatch_semaphore_t drained;
  dispatch_semaphore_t full;
} MacAudioData;

static void fenster_audio_cb(void *p, AudioQueueRef q, AudioQueueBufferRef b) {
  struct fenster_audio *fa = (struct fenster_audio *)p;
  MacAudioData *mad = (MacAudioData *)fa->audio_data;
  dispatch_semaphore_wait(mad->full, DISPATCH_TIME_FOREVER);
  memmove(b->mAudioData, fa->buf, sizeof(fa->buf));
  dispatch_semaphore_signal(mad->drained);
  AudioQueueEnqueueBuffer(q, b, 0, NULL);
}

FENSTER_API int fenster_audio_open(struct fenster_audio *fa) {
  MacAudioData *mad = (MacAudioData *)malloc(sizeof(MacAudioData));
  if (!mad) return -1;
  fa->audio_data = mad;

  AudioStreamBasicDescription format = {0};
  format.mSampleRate = FENSTER_SAMPLE_RATE;
  format.mFormatID = kAudioFormatLinearPCM;
  format.mFormatFlags = kLinearPCMFormatFlagIsFloat | kAudioFormatFlagIsPacked;
  format.mBitsPerChannel = 32;
  format.mFramesPerPacket = format.mChannelsPerFrame = 1;
  format.mBytesPerPacket = format.mBytesPerFrame = 4;
  mad->drained = dispatch_semaphore_create(1);
  mad->full = dispatch_semaphore_create(0);
  AudioQueueNewOutput(&format, fenster_audio_cb, fa, NULL, NULL, 0, &mad->queue);
  for (int i = 0; i < 2; i++) {
    AudioQueueBufferRef buffer = NULL;
    AudioQueueAllocateBuffer(mad->queue, FENSTER_AUDIO_BUFSZ * 4, &buffer);
    buffer->mAudioDataByteSize = FENSTER_AUDIO_BUFSZ * 4;
    memset(buffer->mAudioData, 0, buffer->mAudioDataByteSize);
    AudioQueueEnqueueBuffer(mad->queue, buffer, 0, NULL);
  }
  AudioQueueStart(mad->queue, NULL);
  return 0;
}

FENSTER_API void fenster_audio_close(struct fenster_audio *fa) {
  MacAudioData *mad = (MacAudioData *)fa->audio_data;
  AudioQueueStop(mad->queue, false);
  AudioQueueDispose(mad->queue, false);
  free(mad);
}

FENSTER_API int fenster_audio_available(struct fenster_audio *fa) {
  MacAudioData *mad = (MacAudioData *)fa->audio_data;
  if (dispatch_semaphore_wait(mad->drained, DISPATCH_TIME_NOW))
    return 0;
  return FENSTER_AUDIO_BUFSZ - fa->pos;
}

FENSTER_API void fenster_audio_write(struct fenster_audio *fa, float *buf, size_t n) {
  MacAudioData *mad = (MacAudioData *)fa->audio_data;
  while (fa->pos < FENSTER_AUDIO_BUFSZ && n > 0) {
    fa->buf[fa->pos++] = *buf++, n--;
  }
  if (fa->pos >= FENSTER_AUDIO_BUFSZ) {
    fa->pos = 0;
    dispatch_semaphore_signal(mad->full);
  }
}

#elif defined(_WIN32)
#include <windows.h>
#include <mmsystem.h>

typedef struct {
  HWAVEOUT wo;
  WAVEHDR hdr[2];
  int16_t bufs[2][FENSTER_AUDIO_BUFSZ];
} WindowsAudioData;

FENSTER_API int fenster_audio_open(struct fenster_audio *fa) {
  WindowsAudioData *wad = (WindowsAudioData *)malloc(sizeof(WindowsAudioData));
  if (!wad) return -1;
  fa->audio_data = wad;

  WAVEFORMATEX wfx = {WAVE_FORMAT_PCM, 1, FENSTER_SAMPLE_RATE, FENSTER_SAMPLE_RATE * 2, 2, 16, 0};
  waveOutOpen(&wad->wo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);
  for (int i = 0; i < 2; i++) {
    wad->hdr[i].lpData = (LPSTR)wad->bufs[i];
    wad->hdr[i].dwBufferLength = FENSTER_AUDIO_BUFSZ * 2;
    waveOutPrepareHeader(wad->wo, &wad->hdr[i], sizeof(WAVEHDR));
    waveOutWrite(wad->wo, &wad->hdr[i], sizeof(WAVEHDR));
  }
  return 0;
}

FENSTER_API int fenster_audio_available(struct fenster_audio *fa) {
  WindowsAudioData *wad = (WindowsAudioData *)fa->audio_data;
  for (int i = 0; i < 2; i++)
    if (wad->hdr[i].dwFlags & WHDR_DONE)
      return FENSTER_AUDIO_BUFSZ;
  return 0;
}

FENSTER_API void fenster_audio_write(struct fenster_audio *fa, float *buf, size_t n) {
  WindowsAudioData *wad = (WindowsAudioData *)fa->audio_data;
  for (int i = 0; i < 2; i++) {
    if (wad->hdr[i].dwFlags & WHDR_DONE) {
      for (unsigned j = 0; j < n; j++) {
        wad->bufs[i][j] = (int16_t)(buf[j] * 32767);
      }
      waveOutWrite(wad->wo, &wad->hdr[i], sizeof(WAVEHDR));
      return;
    }
  }
}

FENSTER_API void fenster_audio_close(struct fenster_audio *fa) {
  WindowsAudioData *wad = (WindowsAudioData *)fa->audio_data;
  waveOutClose(wad->wo);
  free(wad);
}

#elif defined(__linux__)
int snd_pcm_open(void **, const char *, int, int);
int snd_pcm_set_params(void *, int, int, int, int, int, int);
int snd_pcm_avail(void *);
int snd_pcm_writei(void *, const void *, unsigned long);
int snd_pcm_recover(void *, int, int);
int snd_pcm_close(void *);

typedef struct {
  void *pcm;
} LinuxAudioData;

FENSTER_API int fenster_audio_open(struct fenster_audio *fa) {
  LinuxAudioData *lad = (LinuxAudioData *)malloc(sizeof(LinuxAudioData));
  if (!lad) return -1;
  fa->audio_data = lad;

  if (snd_pcm_open(&lad->pcm, "default", 0, 0))
    return -1;
  int fmt = (*(unsigned char *)(&(uint16_t){1})) ? 14 : 15;
  return snd_pcm_set_params(lad->pcm, fmt, 3, 1, FENSTER_SAMPLE_RATE, 1, 100000);
}

FENSTER_API int fenster_audio_available(struct fenster_audio *fa) {
  LinuxAudioData *lad = (LinuxAudioData *)fa->audio_data;
  int n = snd_pcm_avail(lad->pcm);
  if (n < 0)
    snd_pcm_recover(lad->pcm, n, 0);
  return n;
}

FENSTER_API void fenster_audio_write(struct fenster_audio *fa, float *buf, size_t n) {
  LinuxAudioData *lad = (LinuxAudioData *)fa->audio_data;
  int r = snd_pcm_writei(lad->pcm, buf, n);
  if (r < 0)
    snd_pcm_recover(lad->pcm, r, 0);
}

FENSTER_API void fenster_audio_close(struct fenster_audio *fa) {
  LinuxAudioData *lad = (LinuxAudioData *)fa->audio_data;
  snd_pcm_close(lad->pcm);
  free(lad);
}

#endif

#endif /* FENSTER_HEADER */
#endif /* FENSTER_AUDIO_H */
