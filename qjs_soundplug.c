#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

QJSCLASS(dsp_node)
QJSCLASS(pcm)

#define DSP_PROTO(TYPE) \
static JSValue TYPE##_proto; \
static TYPE *js2##TYPE (JSValue v) { \
  dsp_node *node = js2dsp_node(v); \
  return node->data; \
} \

DSP_PROTO(bitcrush)
DSP_PROTO(delay)
DSP_PROTO(sound)
DSP_PROTO(compressor)
DSP_PROTO(phasor)
DSP_PROTO(adsr)

JSC_GETSET(dsp_node, pass, boolean)
JSC_GETSET(dsp_node, off, boolean)
JSC_GETSET(dsp_node, gain, number)
JSC_GETSET(dsp_node, pan, number)

JSC_CCALL(dsp_node_plugin, plugin_node(js2dsp_node(self), js2dsp_node(argv[0])))
JSC_CCALL(dsp_node_unplug, unplug_node(js2dsp_node(self)))

static const JSCFunctionListEntry js_dsp_node_funcs[] = {
  CGETSET_ADD(dsp_node, pass),
  CGETSET_ADD(dsp_node, off),
  CGETSET_ADD(dsp_node, gain),
  CGETSET_ADD(dsp_node, pan),
  MIST_FUNC_DEF(dsp_node, plugin, 1),
  MIST_FUNC_DEF(dsp_node, unplug, 0),
};

JSC_GETSET(sound, loop, boolean)
JSC_GETSET(sound, frame, number)
JSC_CCALL(sound_frames, return number2js(js2sound(self)->data->frames))

static const JSCFunctionListEntry js_sound_funcs[] = {
  CGETSET_ADD(sound, loop),
  CGETSET_ADD(sound, frame),
  MIST_FUNC_DEF(sound, frames, 0),
};

JSC_GET(pcm, ch, number)
JSC_GET(pcm, samplerate, number)
JSC_GET(pcm, frames, number)
JSC_CCALL(pcm_format,
  pcm_format(js2pcm(self), js2number(argv[0]), js2number(argv[1]));
)

JSC_SCALL(pcm_save_qoa,
  save_qoa(str, js2pcm(self));
)

JSC_SCALL(pcm_save_wav,
  save_wav(str, js2pcm(self));
)

static const JSCFunctionListEntry js_pcm_funcs[] = {
  MIST_GET(pcm, ch),
  MIST_GET(pcm, samplerate),
  MIST_GET(pcm, frames),
  MIST_FUNC_DEF(pcm, format, 2),
  MIST_FUNC_DEF(pcm, save_qoa, 1),
  MIST_FUNC_DEF(pcm, save_wav, 1)
};

JSC_CCALL(dspsound_noise, return dsp_node2js(dsp_whitenoise()))
JSC_CCALL(dspsound_pink, return dsp_node2js(dsp_pinknoise()))
JSC_CCALL(dspsound_red, return dsp_node2js(dsp_rednoise()))
JSC_CCALL(dspsound_pitchshift, return dsp_node2js(dsp_pitchshift(js2number(argv[0]))))
JSC_CCALL(dspsound_noise_gate, return dsp_node2js(dsp_noise_gate(js2number(argv[0]))))

static JSValue dspnum;

JSC_CCALL(dspsound_limiter,
  ret = dsp_node2js(dsp_limiter(js2number(argv[0])));
  JS_SetPrototype(js, ret, dspnum);  
)

static const JSCFunctionListEntry js_compressor_funcs[] = {

};
JSC_CCALL(dspsound_compressor, return dsp_node2js(dsp_compressor()))

JSC_GETSET(bitcrush, sr, number)
JSC_GETSET(bitcrush, depth, number)
static const JSCFunctionListEntry js_bitcrush_funcs[] = {
  CGETSET_ADD(bitcrush, sr),
  CGETSET_ADD(bitcrush, depth)
};
JSC_CCALL(dspsound_crush,
  ret = dsp_node2js(dsp_bitcrush(js2number(argv[0]), js2number(argv[1])));
  JS_SetPrototype(js, ret, bitcrush_proto);
)

static const JSCFunctionListEntry js_adsr_funcs[] = {

};

static const JSCFunctionListEntry js_phasor_funcs[] = {

};

JSC_CCALL(dspsound_lpf, return dsp_node2js(dsp_lpf(js2number(argv[0]))))
JSC_CCALL(dspsound_hpf, return dsp_node2js(dsp_hpf(js2number(argv[0]))))

JSC_GETSET(delay, ms_delay, number)
JSC_GETSET(delay, decay, number)
static const JSCFunctionListEntry js_delay_funcs[] = {
  CGETSET_ADD(delay, ms_delay),
  CGETSET_ADD(delay, decay),
};
JSC_CCALL(dspsound_delay,
  ret = dsp_node2js(dsp_delay(js2number(argv[0]), js2number(argv[1])));
  JS_SetPrototype(js, ret, delay_proto);
)

JSC_CCALL(dspsound_fwd_delay, return dsp_node2js(dsp_fwd_delay(js2number(argv[0]), js2number(argv[1]))))
JSC_CCALL(dspsound_source,
  ret = dsp_node2js(dsp_source(js2pcm(argv[0])));
  JS_SetPrototype(js, ret, sound_proto);
)
JSC_CCALL(dspsound_mix, return dsp_node2js(make_node(NULL,NULL,NULL)))
JSC_CCALL(dspsound_master, return dsp_node2js(masterbus))
JSC_CCALL(dspsound_plugin_node, plugin_node(js2dsp_node(argv[0]), js2dsp_node(argv[1]));)
JSC_SCALL(dspsound_mod, ret = dsp_node2js(dsp_mod(str))) 
JSC_SSCALL(dspsound_midi, ret = dsp_node2js(dsp_midi(str, make_soundfont(str2))))

static const JSCFunctionListEntry js_dspsound_funcs[] = {
  MIST_FUNC_DEF(dspsound, noise, 0),
  MIST_FUNC_DEF(dspsound, pink, 0),
  MIST_FUNC_DEF(dspsound, red, 0),
  MIST_FUNC_DEF(dspsound, pitchshift, 1),
  MIST_FUNC_DEF(dspsound, noise_gate, 1),
  MIST_FUNC_DEF(dspsound, limiter, 1),
  MIST_FUNC_DEF(dspsound, compressor, 0),
  MIST_FUNC_DEF(dspsound, crush, 2),
  MIST_FUNC_DEF(dspsound, lpf, 1),
  MIST_FUNC_DEF(dspsound, hpf, 1),
  MIST_FUNC_DEF(dspsound, delay, 2),
  MIST_FUNC_DEF(dspsound, fwd_delay, 2),
  MIST_FUNC_DEF(dspsound, source, 1),
  MIST_FUNC_DEF(dspsound, mix, 0),
  MIST_FUNC_DEF(dspsound, master, 0),
  MIST_FUNC_DEF(dspsound, plugin_node, 2),
  MIST_FUNC_DEF(dspsound, midi, 2),
  MIST_FUNC_DEF(dspsound, mod, 1)
};

static int js_enet_init(JSContext *ctx, JSModuleDef *m) {
  PREP_PARENT(bitcrush, dsp_node);
  PREP_PARENT(delay, dsp_node);
  PREP_PARENT(sound, dsp_node);
  PREP_PARENT(compressor, dsp_node);
  PREP_PARENT(phasor, dsp_node);
  PREP_PARENT(adsr, dsp_node);
}

#ifdef JS_SHARED_LIBRARY
#define JS_INIT_MODULE js_init_module
#else
#define JS_INIT_MODULE js_init_module_soundplug
#endif

JSModuleDef *JS_INIT_MODULE(JSContext *ctx, const char *module_name) {
    JSModuleDef *m = JS_NewCModule(ctx, module_name, js_enet_init);
    if (!m)
        return NULL;
    JS_AddModuleExportList(ctx, m, js_enet_funcs, countof(js_enet_funcs));
    return m;
}
