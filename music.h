#ifndef MUSIC_H
#define MUSIC_H

#include "tsf.h"
#include "tml.h"
#include "dsp.h"
#include <stddef.h>

typedef struct dsp_midi_song {
  float bpm;
  double time;
  tml_message *midi;
  tsf *sf;
} midi;

tsf *make_soundfont(const char *path, void *raw, size_t rawlen, int samplerate);

#endif
