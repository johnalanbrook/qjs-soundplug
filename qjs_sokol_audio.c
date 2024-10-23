#include "sokol_audio.h"
#include "quickjs.h"

static const JSCFunctionListEntry js_sokol_audio_funcs[] = {};

static int js_sokol_audio_init(JSContext *ctx, JSModuleDef *m) {
    JS_SetModuleExportList(ctx, m, js_sokol_audio_funcs, sizeof(js_sokol_audio_funcs)/sizeof(JSCFunctionListEntry));
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
