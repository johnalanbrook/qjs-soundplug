#include "dsp.h"

#include "sound.h"
#include "limits.h"
#include "math.h"
#include "stdlib.h"
#include "iir.h"
#include "smbPitchShift.h"

#define PI 3.14159265

int SAMPLERATE = 48000;
int BUF_FRAMES = 2048;
int CHANNELS = 2;

void interleave(soundbyte *a, soundbyte *b, soundbyte *restrict stereo, int frames)
{
  for (int i = 0; i < frames; i++) {
    stereo[i*2] = a[i];
    stereo[i*2+1] = b[i];
  }
}

void deinterleave(soundbyte *restrict stereo, soundbyte *restrict out, int frames, int channels, int chout)
{
  chout--;
  for (int i = 0; i < frames; i++)
    out[i] = stereo[i*channels+chout];
}

void mono_to_stero(soundbyte *a, soundbyte *stereo, int frames)
{
  interleave(a,a,stereo, frames);
}

void mono_expand(soundbyte *restrict buffer, int to, int frames)
{
  soundbyte hold[frames];
  memcpy(hold, buffer, sizeof(soundbyte)*frames);

  for (int i = 0; i < frames; i++)
    for (int j = 0; j < to; j++)
      buffer[i*to+j] = hold[i];
}

void dsp_am_mod(soundbyte *in, soundbyte *out, int frames)
{
  for (int i = 0; i < frames*CHANNELS; i++) out[i] *= in[i];
}

/* Add b into a */
void sum_soundbytes(soundbyte *restrict a, soundbyte *restrict b, int samples)
{
  for (int i = 0; i < samples; i++) a[i] += b[i];
}

void norm_soundbytes(soundbyte *a, float lvl, int samples)
{
  float tar = lvl;
  float max = 0 ;
  for (int i = 0; i < samples; i++) max = fabsf(a[i]) > max ? fabsf(a[i]) : max;
  float mult = max/tar;
  scale_soundbytes(a, mult, samples);
}

void scale_soundbytes(soundbyte *a, float scale, int samples)
{
  if (scale == 1) return;
  for (int i = 0; i < samples; i++) a[i] *= scale;
}

void zero_soundbytes(soundbyte *restrict a, int samples) { memset(a, 0, sizeof(soundbyte)*samples); }

void set_soundbytes(soundbyte *a, soundbyte *b, int samples)
{
  zero_soundbytes(a, samples);
  sum_soundbytes(a,b,samples);
}

static int node_count = 0;

float sin_phasor(float p)
{
    return sin(2*PI*p);
}

float square_phasor(float p)
{
    return lround(p);
}

float saw_phasor(float p)
{
    return 2*p-1;
}

float tri_phasor(float p)
{
    return 4*(p * 0.5f ? p : (1-p)) - 1;
}

/*void filter_phasor(phasor *p, soundbyte *buffer, int frames)
{
  for (int i = 0; i < frames; i++) {
    buffer[i] = p->filter(p->phase) * p->amp;
    p->phase += p->freq/SAMPLERATE;
  }
  p->phase = p->phase - (int)p->phase;
  mono_expand(buffer, CHANNELS, frames);
}*/

void dsp_rectify(soundbyte *in, soundbyte *out, int frames)
{
  for (int i = 0; i < frames; i++) out[i] = fabsf(in[i]);
}

soundbyte sample_whitenoise()
{
  return ((float)rand()/(float)(RAND_MAX/2))-1;
}

void gen_whitenoise(void *data, soundbyte *out, int n)
{
  for (int i = 0; i < n; i++)
    out[i] = sample_whitenoise();
    
  mono_expand(out, CHANNELS, n);
}

void dsp_whitenoise(soundbyte *out)
{
}

void gen_pinknoise(struct dsp_iir *pink, soundbyte *out, int n)
{
  gen_whitenoise(NULL, out, n);

  for (int i = 0; i < n*CHANNELS; i++) {
    soundbyte sum = 0;
    for (int j = 0; j < 6; j++) {
      pink->x[j] = pink->x[j]*pink->b[j] + out[i]*pink->a[j];
      sum += pink->x[j];
    }
    pink->x[6] = out[i] * 0.115926;
    
    out[i] = sum + out[i] * 0.5362 + pink->x[6];
    out[i] *= 0.11;
  }

    /* *  https://www.firstpr.com.au/dsp/pink-noise/
    b0 = 0.99886 * b0 + white * 0.0555179;
    b1 = 0.99332 * b1 + white * 0.0750759;
    b2 = 0.96900 * b2 + white * 0.1538520;
    b3 = 0.86650 * b3 + white * 0.3104856;
    b4 = 0.55000 * b4 + white * 0.5329522;
    b5 = -0.7616 * b5 - white * 0.0168980;
    pink = b0 + b1 + b2 + b3 + b4 + b5 + b6 + white * 0.5362;
    b6 = white * 0.115926;
    */
}

void dsp_pinknoise()
{
  struct dsp_iir *pink = malloc(sizeof(*pink));
  *pink = make_iir(6);
  float pinka[7] = {0.0555179, 0.0750759, 0.1538520, 0.3104856, 0.5329522, -0.0168980, 0.115926};  
  float pinkb[7] = {0.99886, 0.99332, 0.969, 0.8665, 0.55, -0.7616, 0.115926};  
  memcpy(pink->a, pinka, 7*sizeof(float));
  memcpy(pink->b, pinkb, 7*sizeof(float));
}

void filter_rednoise(soundbyte *restrict last, soundbyte *out, int frames)
{
  gen_whitenoise(NULL, out, frames);
  for (int i = 0; i < frames*CHANNELS; i++) {
    out[i] = (*last + (0.02*out[i]))/1.02; 
    *last = out[i];
    out[i] *= 3.5;
  }
}

void dsp_rednoise()
{
  soundbyte *last = malloc(sizeof(soundbyte));
  *last = 0;
}

void filter_pitchshift(float *restrict octaves, soundbyte *buffer, int frames)
{
  soundbyte ch1[frames];
  for (int i = 0; i < frames; i++)
    ch1[i] = buffer[i*CHANNELS];
  smbPitchShift(*octaves, frames, 1024, 4,  SAMPLERATE, ch1, buffer);
  mono_expand(buffer, CHANNELS, frames);
}

soundbyte iir_filter(struct dsp_iir iir, soundbyte val)
{
  iir.y[0] = 0.0;

  iir.x[0] = val;

  for (int i = 0; i < iir.n; i++)
    iir.y[0] += iir.a[i] * iir.x[i];

  for (int i = 1; i < iir.n; i++)
    iir.y[0] -= iir.b[i] * iir.y[i];

  /* Shift values in */
  for (int i = iir.n-1; i > 0; i--) {
    iir.x[i] = iir.x[i-1];
    iir.y[i] = iir.y[i-1];
  }
  
  return iir.y[0];
}

void filter_iir(struct dsp_iir *iir, soundbyte *buffer, int frames)
{
  for (int i = 0; i < frames; i++) {
    soundbyte v = iir_filter(*iir, buffer[i*CHANNELS]);
    for (int j = 0; j < CHANNELS; j++) buffer[i*CHANNELS+j] = v;
  }
}

void dsp_delay(soundbyte *in, soundbyte *out, int frames, double sec, double decay)
{
/*  for (int i = 0; i < frames*CHANNELS; i++) {
    ringpush(d->ring, buf[i]);  
    buf[i] += ringshift(d->ring)*d->decay;
  }*/
}

/* Get decay constant for a given pole */
/* Samples to decay 1 time constant is exp(-1/timeconstant) */
double tau2pole(double tau)
{
    return exp(-1/(tau*SAMPLERATE));
}

void dsp_adsr(soundbyte *in, soundbyte *out, int frames, unsigned int atk, unsigned int dec, unsigned int sus, unsigned int rls, float time)
{
  soundbyte val;

  for (int i = 0; i < frames; i++) {
/*    if (time > rls) {
      out = 0.f;
      goto fin;
     }

    if (time > sus) {
      // Release phase
      out = rls_t * out;
       goto fin;
    }

    if (time > dec) {
      // Sustain phase
      out = sus_pwr;
      goto fin;
    }

    if (time > atk) {
      // Decay phase
      out = (1 - dec_t) * sus_pwr + dec_t * out;
      goto fin;
    }

    // Attack phase
    out = (1-atk_t) + atk_t * out;
*/    
    fin:

//    val = SHRT_MAX * out;
//    out[i*CHANNELS] = out[i*CHANNELS+1] = val;
//    time += (double)(1000.f / SAMPLERATE);
    }
}

void dsp_noise_gate(soundbyte *in, soundbyte *out, int frames, float floor)
{
  for (int i = 0; i < frames*CHANNELS; i++) out[i] = fabsf(in[i]) < floor ? 0.0 : out[i];
}

void filter_limiter(float *restrict ceil, soundbyte *restrict out, int n)
{
  for (int i = 0; i < n*CHANNELS; i++) out[i] = fabsf(out[i]) > *ceil ?  *ceil : out[i];
}

void dsp_compressor(soundbyte *in, soundbyte *out, int frames)
{
/*    float val;
    float db;
    db = comp->target * (val - comp->threshold) / comp->ratio;

    for (int i = 0; i < n; i++) {
      val = float2db(out[i*CHANNELS]);

      if (val < comp->threshold) {
        comp->target = comp->rls_tau * comp->target;
        val += db;
      } else {
        comp->target = (1 - comp->atk_tau) + comp->atk_tau * comp->target; // TODO: Bake in the 1 - atk_tau
        val -= db;
      }

    // Apply same compression to both channels
    out[i*CHANNELS] = out[i*CHANNELS+1] = db2float(val) * ( out[i*CHANNELS] > 0 ? 1 : -1);
    }*/
}

void pan_frames(soundbyte *in, soundbyte *out, float deg, int frames)
{
  if (deg == 0.f) return;
  if (deg < -1) deg = -1.f;
  else if (deg > 1) deg = 1.f;

  float db1, db2;

  if (deg > 0) {
    db1 = pct2db(1 - deg);
    db2 = pct2db(deg);
    for (int i = 0; i < frames; i++) {
      soundbyte L = out[i*2];
      soundbyte R = out[i*2+1];
      out[i*2] = fgain(L, db1);
      out[i*2+1] = (R + fgain(L, db2))/2;
    }
  } else {
    db1 = pct2db(1 + deg);
    db2 = pct2db(-1*deg);
    for (int i = 0; i < frames; i++) {
      soundbyte L = out[i*2];
      soundbyte R = out[i*2+1];
      out[i*2+1] = fgain(R,db1);
      out[i*2] = fgain(L, db1) + fgain(R, db2);
    }
  }
}

void dsp_mono(soundbyte *in, soundbyte *out, int n)
{
    for (int i = 0; i < n; i++) {
        soundbyte val = (out[i*CHANNELS] + out[i*CHANNELS+1]) / 2;

        for (int j = 0; j < CHANNELS; j++)
            out[i*CHANNELS+j] = val;
    }
}

#define ROUND(f) ((float)((f>0.0)?floor(f+0.5):ceil(f-0.5)))
void dsp_bitcrush(soundbyte *in, soundbyte *out, int frames, float sr, float res, float depth)
{
  int max = pow(2,depth) - 1;
  int step = SAMPLERATE/sr;

  int i = 0;
  while (i < frames) {
    float left = ROUND((out[0]+1.0)*max)/(max-1.0);
    float right = ROUND((out[1]+1.0)*max)/(max-1.0);
    
    for (int j = 0; j < step && i < frames; j++) {
      out[0] = left;
      out[1] = right;
      out += CHANNELS;
      i++;
    }
  }
}
