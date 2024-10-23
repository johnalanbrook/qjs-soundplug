#define STB_DS_IMPLEMENTATION

#include "quickjs.h"
// This essentially is a list of dsp functions that the javascript can call into
#include "dsp.h"

double js2number(JSContext *js, JSValue v)
{
  double ret;
  JS_ToFloat64(js, &ret, v);
  return ret;
}

static JSValue js_dsp_rectify(JSContext *js, JSValueConst self, int argc, JSValueConst *argv)
{
  soundbyte *out = JS_GetArrayBuffer(js, NULL, argv[0]);
  soundbyte *in = JS_GetArrayBuffer(js, NULL, argv[1]);
  dsp_rectify(in, out, js2number(js, argv[2]));
  return JS_UNDEFINED;
}


static const JSCFunctionListEntry js_soundplug_funcs[] = {
  JS_CFUNC_DEF("rectify", 3, js_dsp_rectify),
};

static int js_soundplug_init(JSContext *ctx, JSModuleDef *m) {
    JS_SetModuleExportList(ctx, m, js_soundplug_funcs, sizeof(js_soundplug_funcs)/sizeof(JSCFunctionListEntry));
    return 0;
}

#ifdef JS_SHARED_LIBRARY
#define JS_INIT_MODULE js_init_module
#else
#define JS_INIT_MODULE js_init_module_soundplug
#endif

JSModuleDef *JS_INIT_MODULE(JSContext *ctx, const char *module_name) {
    JSModuleDef *m = JS_NewCModule(ctx, module_name, js_soundplug_init);
    if (!m) return NULL;
    JS_AddModuleExportList(ctx, m, js_soundplug_funcs, sizeof(js_soundplug_funcs)/sizeof(JSCFunctionListEntry));
    return m;
}
