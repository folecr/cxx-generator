#include "jsbutils.h"

using namespace jsb::utils;

void reportError(JSContext *cx, const char *message, JSErrorReport *report)
{
	js_log("%s:%u:%s\n",
			report->filename ? report->filename : "<no filename=\"filename\">",
			(unsigned int) report->lineno,
			message);
};

JSBool log(JSContext* cx, uint32_t argc, jsval *vp)
{
	if (argc > 0) {
		JSString *string = NULL;
		JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &string);
		if (string) {
			char *cstr = JS_EncodeString(cx, string);
			js_log(cstr);
		}
	}
	return JS_TRUE;
}

JSBool forceGC(JSContext *cx, uint32_t argc, jsval *vp)
{
	JSRuntime *rt = JS_GetRuntime(cx);
	JS_GC(rt);
	return JS_TRUE;
}

void removeAllRoots(JSContext *cx) {
    js_proxy_t *current, *tmp;
    HASH_ITER(hh, _js_native_global_ht, current, tmp) {
        JS_RemoveObjectRoot(cx, &current->obj);
        HASH_DEL(_js_native_global_ht, current);
        free(current);
    }
    HASH_ITER(hh, _native_js_global_ht, current, tmp) {
        HASH_DEL(_native_js_global_ht, current);
        free(current);
    }
    HASH_CLEAR(hh, _js_native_global_ht);
    HASH_CLEAR(hh, _native_js_global_ht);
    HASH_CLEAR(hh, _js_global_type_ht);
}

JSBool addRootJS(JSContext *cx, uint32_t argc, jsval *vp)
{
    if (argc == 1) {
        JSObject *o = NULL;
        if (JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &o) == JS_TRUE) {
            if (JS_AddNamedObjectRoot(cx, &o, "from-js") == JS_FALSE) {
                js_log("something went wrong when setting an object to the root");
            }
        }
        return JS_TRUE;
    }
    return JS_FALSE;
}

JSBool removeRootJS(JSContext *cx, uint32_t argc, jsval *vp)
{
    if (argc == 1) {
        JSObject *o = NULL;
        if (JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &o) == JS_TRUE) {
            JS_RemoveObjectRoot(cx, &o);
        }
        return JS_TRUE;
    }
    return JS_FALSE;
}

void executeJSFunctionWithName(JSContext *cx, JSObject *obj,
                                      const char *funcName, jsval &dataVal,
                                      jsval &retval) {
    JSBool hasAction;
    jsval temp_retval;

    if (JS_HasProperty(cx, obj, funcName, &hasAction) && hasAction) {
        if(!JS_GetProperty(cx, obj, funcName, &temp_retval)) {
            return;
        }
        if(temp_retval == JSVAL_VOID) {
            return;
        }
        JS_CallFunctionName(cx, obj, funcName,
                            1, &dataVal, &retval);
    }

}

JSBool executeScript(JSContext *cx, uint32_t argc, jsval *vp)
{
    if (argc >= 1) {
        jsval* argv = JS_ARGV(cx, vp);
        JSString* str = JS_ValueToString(cx, argv[0]);
        const char* path = JS_EncodeString(cx, str);
        JSBool res = false;
        res = ScriptingCore::getInstance()->runScript(path);
        JS_free(cx, (void*)path);
        return res;
    }
    return JS_TRUE;
}

JSBool jsb::utils::debug::dumpRoot(JSContext *unused_cx, uint32_t unused_argc, jsval *unused_vp)
{
    // JS_DumpNamedRoots is only available on DEBUG versions of SpiderMonkey.
    // Mac and Simulator versions were compiled with DEBUG.
#if DEBUG
    JSContext *_cx = ScriptingCore::getInstance()->getGlobalContext();
    JSRuntime *rt = JS_GetRuntime(_cx);
    JS_DumpNamedRoots(rt, dumpNamedRoot, NULL);
#endif
    return JS_TRUE;
}

JSBool jsb::utils::debug::jsNewGlobal(JSContext* cx, unsigned argc, jsval* vp)
{
    if (argc == 1) {
        jsval *argv = JS_ARGV(cx, vp);
        js::RootedObject *global = new js::RootedObject(cx, NewGlobalObject(cx));
        JS_WrapObject(cx, global->address());
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(*global));
        return JS_TRUE;
    }
    return JS_FALSE;
}
