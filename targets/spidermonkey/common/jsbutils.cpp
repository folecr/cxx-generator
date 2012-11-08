#include "uthash.h"
#include "jsbutils.h"
#include "jsdbgapi.h"
#include "spidermonkey_specifics.h"
#include "jsbScriptingCore.h"
#include <string>
#include <vector>
#include <map>

void jsb::utils::reportError(JSContext *cx, const char *message, JSErrorReport *report)
{
    // FIXME
	// js_log("%s:%u:%s\n",
	// 		report->filename ? report->filename : "<no filename=\"filename\">",
	// 		(unsigned int) report->lineno,
	// 		message);
};

JSBool jsb::utils::log(JSContext* cx, uint32_t argc, jsval *vp)
{
	if (argc > 0) {
		JSString *string = NULL;
		JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &string);
		if (string) {
			char *cstr = JS_EncodeString(cx, string);
            // FIXME
			// js_log(cstr);
		}
	}
	return JS_TRUE;
}

JSBool jsb::utils::forceGC(JSContext *cx, uint32_t argc, jsval *vp)
{
	JSRuntime *rt = JS_GetRuntime(cx);
	JS_GC(rt);
	return JS_TRUE;
}

void jsb::utils::removeAllRoots(JSContext *cx) {
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

JSBool jsb::utils::addRootJS(JSContext *cx, uint32_t argc, jsval *vp)
{
    if (argc == 1) {
        JSObject *o = NULL;
        if (JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "o", &o) == JS_TRUE) {
            if (JS_AddNamedObjectRoot(cx, &o, "from-js") == JS_FALSE) {
                // FIXME
                // js_log("something went wrong when setting an object to the root");
            }
        }
        return JS_TRUE;
    }
    return JS_FALSE;
}

JSBool jsb::utils::removeRootJS(JSContext *cx, uint32_t argc, jsval *vp)
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

void jsb::utils::executeJSFunctionWithName(JSContext *cx, JSObject *obj,
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

JSBool jsb::utils::executeScript(JSContext *cx, uint32_t argc, jsval *vp)
{
    if (argc >= 1) {
        jsval* argv = JS_ARGV(cx, vp);
        JSString* str = JS_ValueToString(cx, argv[0]);
        const char* path = JS_EncodeString(cx, str);
        JSBool res = false;
        // FIXME
        // res = ScriptingCore::getInstance()->runScript(path);
        JS_free(cx, (void*)path);
        return res;
    }
    return JS_TRUE;
}

static void _finalize(JSFreeOp *freeOp, JSObject *obj) {
    return;
}

static JSClass global_class = {
    "global", JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, _finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

JSObject* jsb::utils::NewGlobalObject(JSContext* cx)
{
	JSObject* glob = JS_NewGlobalObject(cx, &global_class, NULL);
	if (!glob) {
		return NULL;
	}
	JSAutoCompartment ac(cx, glob);
	JSBool ok = JS_TRUE;
	ok = JS_InitStandardClasses(cx, glob);
	if (ok)
		JS_InitReflect(cx, glob);
	if (ok)
		ok = JS_DefineDebuggerObject(cx, glob);
	if (!ok)
		return NULL;

    return glob;
}

JSBool jsb::utils::debug::dumpRoot(JSContext *unused_cx, uint32_t unused_argc, jsval *unused_vp)
{
    // JS_DumpNamedRoots is only available on DEBUG versions of SpiderMonkey.
    // Mac and Simulator versions were compiled with DEBUG.
#if DEBUG
    // JSContext *_cx = ScriptingCore::getInstance()->getGlobalContext();
    // JSRuntime *rt = JS_GetRuntime(_cx);
    // JS_DumpNamedRoots(rt, dumpNamedRoot, NULL);
#endif
    return JS_TRUE;
}

static std::vector<jsb::sc_register_sth> registrationList;
static std::map<std::string, js::RootedObject*> globals;

JSBool jsb::utils::debug::jsNewGlobal(JSContext* cx, unsigned argc, jsval* vp)
{
    if (argc == 1) {
        jsval *argv = JS_ARGV(cx, vp);
        JSString *jsstr = JS_ValueToString(cx, argv[0]);
        std::string key = JS_EncodeString(cx, jsstr);
        js::RootedObject *global = globals[key];
        if (!global) {
            JSObject* g = NewGlobalObject(cx);
            global = new js::RootedObject(cx, g);
            JS_WrapObject(cx, global->address());
            globals[key] = global;
            // register everything on the list on this new global object
			JSAutoCompartment ac(cx, g);
            for (std::vector<jsb::sc_register_sth>::iterator it = registrationList.begin(); it != registrationList.end(); it++) {
                jsb::sc_register_sth callback = *it;
                callback(cx, g);
            }
        }
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(*global));
        return JS_TRUE;
    }
    return JS_FALSE;
}
