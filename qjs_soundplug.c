#define STB_HEXWAVE_IMPLEMENTATION
#include "stb_hexwave.h"
#include "quickjs.h"
// This essentially is a list of dsp functions that the javascript can call into
#include "dsp.h"

#include <stdio.h>

#define countof(x) (sizeof(x)/sizeof((x)[0]))

#define FNSIG (JSContext *js, JSValueConst this_val, int argc, JSValue *argv)
#define GETSIG (JSContext *js, JSValueConst this_val)
#define SETSIG (JSContext *js, JSValueConst this_val, JSValue val)

#define JSCLASS(TYPE) \
static JSClassID js_##TYPE##_class_id; \
static inline TYPE *js2##TYPE(JSContext *js, JSValue v) { \
  return JS_GetOpaque(v, js_##TYPE##_class_id); \
} \
static JSClassDef js_##TYPE##_class = { \
  #TYPE, \
  .finalizer = js_##TYPE##_finalizer \
}; \

double js2number(JSContext *js, JSValue v)
{
  double ret;
  JS_ToFloat64(js, &ret, v);
  return ret;
}

static void js_HexWave_finalizer(JSRuntime *rt, JSValue val);

JSCLASS(HexWave);

static void js_HexWave_finalizer(JSRuntime *rt, JSValue val)
{
  HexWave *hex = JS_GetOpaque(val, js_HexWave_class_id);
  free(hex);
}

static JSValue js_make_HexWave(JSContext *js, JSValue self, int argc, JSValue *argv)
{
  HexWave *hex = malloc(sizeof(*hex));
  hexwave_create(hex, 1, 0, 0, 0);
  JSValue obj = JS_NewObjectClass(js, js_HexWave_class_id);
  JS_SetOpaque(obj, hex);
  return obj;
}

static JSValue js_HexWave_generate_samples(JSContext *js, JSValue self, int argc, JSValue *argv)
{
  HexWave *hex = js2HexWave(js, self);
  size_t len;
  float *data = JS_GetArrayBuffer(js, &len, argv[0]);
  len /= sizeof(float);
  hexwave_generate_samples(data, len, hex, js2number(js, argv[1]));
  return JS_UNDEFINED;
}

static const JSCFunctionListEntry js_HexWave_funcs [] = {
  JS_CFUNC_DEF("sample", 2, js_HexWave_generate_samples)
};

static JSValue js_dsp_rectify(JSContext *js, JSValueConst self, int argc, JSValueConst *argv)
{
  soundbyte *out = JS_GetArrayBuffer(js, NULL, argv[0]);
  soundbyte *in = JS_GetArrayBuffer(js, NULL, argv[1]);
  dsp_rectify(in, out, js2number(js, argv[2]));
  return JS_UNDEFINED;
}

static const JSCFunctionListEntry js_soundplug_funcs[] = {
  JS_CFUNC_DEF("rectify", 3, js_dsp_rectify),
  JS_CFUNC_DEF("make_hex", 0, js_make_HexWave)
};

#define INITCLASS(TYPE) \
JS_NewClassID(&js_##TYPE##_class_id); \
JS_NewClass(JS_GetRuntime(js), js_##TYPE##_class_id, &js_##TYPE##_class); \
JSValue TYPE##_proto = JS_NewObject(js); \
JS_SetPropertyFunctionList(js, TYPE##_proto, js_##TYPE##_funcs, countof(js_##TYPE##_funcs)); \
JS_SetClassProto(js, js_##TYPE##_class_id, TYPE##_proto); \

static int js_soundplug_init(JSContext *js, JSModuleDef *m) {
    JS_SetModuleExportList(js, m, js_soundplug_funcs, sizeof(js_soundplug_funcs)/sizeof(JSCFunctionListEntry));
    
    INITCLASS(HexWave);
    
    hexwave_init(32,16,NULL);
    
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
