#include "sound.h"
#include "limits.h"
#include "math.h"
#include "music.h"
#include "string.h"
#include "time.h"
#include <stdlib.h>

#include "stb_ds.h"

#define CHANNELS 2

#include "dsp.h"

#define POCKETMOD_IMPLEMENTATION
#include "pocketmod.h"

#define TSF_NO_STDIO
#define TSF_IMPLEMENTATION
#include "tsf.h"

#define TML_NO_STDIO
#define TML_IMPLEMENTATION
#include "tml.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#ifndef NFLAC
#define DR_FLAC_IMPLEMENTATION
#define DR_FLAC_NO_STDIO
#include "dr_flac.h"
#endif

#ifndef NMP3
#define DR_MP3_NO_STDIO
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"
#endif

#ifndef NQOA
#define QOA_IMPLEMENTATION
#include "qoa.h"
#endif

void short_to_float_array(const short *in, float *out, int frames, int channels)
{
  for (int i = 0; i < frames * channels; i++)
    out[i] = (float)in[i] / 32768.0f;
}

void float_to_short_array(float *in, short *out, int frames, int channels)
{
  for (int i = 0; i < frames*channels; i++)
    out[i] = (float)in[i]*32768;
}

void change_channels(struct pcm *w, int ch) {
  if (w->ch == ch) return;
  soundbyte *data = w->data;
  int samples = ch * w->frames;
  soundbyte *new = malloc(sizeof(soundbyte) * samples);

  if (ch > w->ch) {
    /* Sets all new channels equal to the first one */
    for (int i = 0; i < w->frames; i++) {
      for (int j = 0; j < ch; j++)
        new[i * ch + j] = data[i];
    }
  } else {
    /* Simple method; just use first N channels present in wav */
    for (int i = 0; i < w->frames; i++)
      for (int j = 0; j < ch; j++)
        new[i * ch + j] = data[i * ch + j];
  }

  free(w->data);
  w->data = new;
}

void resample_pcm(soundbyte *in, soundbyte *out, int in_frames, int out_frames, int channels)
{
  float ratio = (float)in_frames / out_frames;
  for (int i = 0; i < out_frames; i++) {
    // Find the position in the input buffer.
    float in_pos = i * ratio;
    int in_index = (int)in_pos;   // Get the integer part of the position.
    float frac = in_pos - in_index;  // Get the fractional part for interpolation.
    
    for (int ch = 0; ch < channels; ch++) {
      // Linear interpolation between two input samples.
      soundbyte sample1 = in[in_index * channels + ch];
      soundbyte sample2 = in[(in_index + 1) * channels + ch];
      out[i * channels + ch] = (soundbyte)((1.0f - frac) * sample1 + frac * sample2);
    }
  }
}

void change_samplerate(struct pcm *w, int rate) {
  if (rate == w->samplerate) return;
  float ratio = (float)rate / w->samplerate;
  int outframes = w->frames * ratio;
  soundbyte *resampled = malloc(w->ch*outframes*sizeof(soundbyte));
  resample_pcm(w->data, resampled, w->frames, outframes, w->ch);
  free(w->data);
  
  w->data = resampled;
  w->frames = outframes;
  w->samplerate = rate;
}

void filter_mod(pocketmod_context *mod, soundbyte *buffer, int frames)
{
  pocketmod_render(mod, buffer, frames*CHANNELS*sizeof(soundbyte));
}

struct pcm *make_pcm(void *raw, size_t rawlen, char *ext) {
  struct pcm *mwav = malloc(sizeof(*mwav));  

  if (!strcmp(ext, "wav"))
    mwav->data = drwav_open_memory_and_read_pcm_frames_f32(raw, rawlen, &mwav->ch, &mwav->samplerate, &mwav->frames, NULL);
  else if (!strcmp(ext, "flac")) {
  #ifndef NFLAC  
    mwav->data = drflac_open_memory_and_read_pcm_frames_f32(raw, rawlen, &mwav->ch, &mwav->samplerate, &mwav->frames, NULL);
  #endif
  }
  else if (!strcmp(ext, "mp3")) {
  #ifndef NMP3  
    drmp3_config cnf;
    mwav->data = drmp3_open_memory_and_read_pcm_frames_f32(raw, rawlen, &cnf, &mwav->frames, NULL);
    mwav->ch = cnf.channels;
    mwav->samplerate = cnf.sampleRate;
  #endif
  }
  else if (!strcmp(ext, "qoa")) {
  #ifndef NQOA
    qoa_desc qoa;
    short *qoa_data = qoa_decode(raw, rawlen, &qoa);
    mwav->ch = qoa.channels;
    mwav->samplerate = qoa.samplerate;
    mwav->frames = qoa.samples/mwav->ch;
    mwav->data = malloc(sizeof(soundbyte) * mwav->frames * mwav->ch);
    short_to_float_array(qoa_data, mwav->data, mwav->frames,mwav->ch);
    free(qoa_data);
  #endif
  } else {
    free (raw);
    free(mwav);
    return NULL;
  }
  free(raw);

  return mwav;
}

void pcm_format(pcm *pcm, int samplerate, int channels)
{
  change_samplerate(pcm, samplerate);
  change_channels(pcm, channels);
}

void save_qoa(char *file, pcm *pcm)
{
  qoa_desc q;
  short *out = malloc(sizeof(short)*pcm->ch*pcm->frames);
  float_to_short_array(pcm->data, out, pcm->frames, pcm->ch);
  q.channels = pcm->ch;
  q.samples = pcm->frames;
  q.samplerate = pcm->samplerate;
  int encoded = qoa_write(file, out, &q);
  free(out);
}

void save_wav(char *file, pcm *pcm)
{
  drwav wav;
  drwav_data_format fmt = {0};
  fmt.format = DR_WAVE_FORMAT_PCM;
  fmt.channels = pcm->ch;
  fmt.sampleRate = pcm->samplerate;
  fmt.bitsPerSample = 32;
  drwav_int32 *out = malloc(sizeof(*out)*pcm->ch*pcm->frames);
  drwav_f32_to_s32(out, pcm->data, pcm->frames*pcm->ch);
  drwav_init_file_write_sequential_pcm_frames(&wav, file, &fmt, pcm->frames, NULL);
  drwav_write_pcm_frames(&wav, pcm->frames, out);
  drwav_uninit(&wav);
  free(out);
}

void pcm_free(pcm *pcm)
{
  free(pcm->data);
  free(pcm);
}

void sound_fillbuf(struct sound *s, soundbyte *buf, int n) {
  int frames = s->data->frames - s->frame;
  if (frames == 0) return;
  int end = 0;
  if (frames > n)
    frames = n;
  else
    end = 1;

  soundbyte *in = s->data->data;
  
  for (int i = 0; i < frames; i++) {
    for (int j = 0; j < CHANNELS; j++)
      buf[i * CHANNELS + j] = in[s->frame*CHANNELS + j];

    s->frame++;
  }

  if(end) {
    if (s->loop)
      s->frame = 0;
  }
}

void free_source(struct sound *s)
{
  free(s);
}

int sound_finished(const struct sound *s) {
  return s->frame == s->data->frames;
}

float short2db(short val) {
  return 20 * log10(abs(val) / SHRT_MAX);
}

short db2short(float db) {
  return pow(10, db / 20.f) * SHRT_MAX;
}

short short_gain(short val, float db) {
  return (short)(pow(10, db / 20.f) * val);
}

float float2db(float val) { return 20 * log10(fabsf(val)); }
float db2float(float db) { return pow(10, db/20); }

float fgain(float val, float db) {
  return pow(10,db/20.f)*val;
}

float pct2db(float pct) {
  if (pct <= 0) return -72.f;

  return 10 * log2(pct);
}

float pct2mult(float pct) {
  if (pct <= 0) return 0.f;

  return pow(10, 0.5 * log2(pct));
}
