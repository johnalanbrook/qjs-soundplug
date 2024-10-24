#define SOKOL_AUDIO_IMPL
#include "sokol_audio.h"
#include "quickjs.h"

static JSValue cb;
static JSContext *gjs;

void stream_cb(float *buffer, int num_frames, int num_channels, void *user_data)
{
  if (JS_IsUndefined(cb)) return;
  JSValue gen = JS_Call(gjs, cb, JS_UNDEFINED, 0, NULL);
  size_t len;
  float *data = JS_GetArrayBuffer(gjs, &len, gen);
  memcpy(buffer, data, len);
}

static JSValue js_sokol_init(JSContext *js, JSValue self, int argc, JSValue *argv)
{
  /*saudio_desc desc;
  JS_ToFloat64(js, &desc.sample_rate, argv[0]);
  JS_ToFloat64(js, &desc.num_channels, argv[1]);
  JS_ToFloat64(js, &desc.buffer_frames, argv[2]);
  desc.stream_cb = stream_cb;
  
  saudio_setup(&desc);*/
  cb = JS_DupValue(js, argv[0]);
  
  return JS_UNDEFINED;
}

static const JSCFunctionListEntry js_sokol_audio_funcs[] = {
  JS_CFUNC_DEF("init", 3, js_sokol_init)
};

static int js_sokol_audio_init(JSContext *ctx, JSModuleDef *m) {
    JS_SetModuleExportList(ctx, m, js_sokol_audio_funcs, sizeof(js_sokol_audio_funcs)/sizeof(JSCFunctionListEntry));
    
    cb = JS_UNDEFINED;
    
    saudio_setup(&(saudio_desc){.stream_cb = stream_cb});
    gjs = ctx;

    return 0;
}

#ifdef JS_SHARED_LIBRARY
#define JS_INIT_MODULE js_init_module
#else
#define JS_INIT_MODULE js_init_module_sokol_audio
#endif

JSModuleDef *JS_INIT_MODULE(JSContext *ctx, const char *module_name) {
    JSModuleDef *m = JS_NewCModule(ctx, module_name, js_sokol_audio_init);
    if (!m) return NULL;
    JS_AddModuleExportList(ctx, m, js_sokol_audio_funcs, sizeof(js_sokol_audio_funcs)/sizeof(JSCFunctionListEntry));
    return m;
}
