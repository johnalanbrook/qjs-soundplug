#ifndef DSP_H
#define DSP_H

typedef float soundbyte;

void scale_soundbytes(soundbyte *a, float scale, int frames);
void sum_soundbytes(soundbyte *a, soundbyte *b, int frames);
void zero_soundbytes(soundbyte *a, int frames);
void set_soundbytes(soundbyte *a, soundbyte *b, int frames);
void dsp_rectify(soundbyte *in, soundbyte *out, int frames);
void dsp_am_mod(soundbyte *in, soundbyte *out, int frames);
void dsp_hpf(soundbyte *in, soundbyte *out, int frames, float freq);
void dsp_lpf(soundbyte *in, soundbyte *out, int frames, float freq);
void dsp_adsr(soundbyte *in, soundbyte *out, int frames, unsigned int atk, unsigned int dec, unsigned int sus, unsigned int rls, float time);
void dsp_delay(soundbyte *in, soundbyte *out, int frames, double sec, double decay);
void dsp_fwd_delay(soundbyte *in, soundbyte *out, int frames, double sec, double decay);
void dsp_pitchshift(soundbyte *in, soundbyte *out, int frames, float octaves);
void dsp_compressor(soundbyte *in, soundbyte *out, int frames);
void dsp_limiter(float ceil);
void dsp_noise_gate(soundbyte *in, soundbyte *out, int frames, float floor);
void dsp_whitenoise(soundbyte *out);
void dsp_pinknoise();
void dsp_rednoise();
void dsp_reverb(soundbyte *in, soundbyte *out, int frames);
void dsp_sinewave(soundbyte *in, soundbyte *out, int frames, float amp, float freq);
void dsp_square(soundbyte *in, soundbyte *out, int frames, float amp, float freq, int sr, int ch);
void dsp_bitcrush(soundbyte *in, soundbyte *out, int frames, float sr, float res, float depth);
void dsp_mono(soundbyte *in, soundbyte *out, int n);
void pan_frames(soundbyte *in, soundbyte *out, float deg, int frames);

#endif
