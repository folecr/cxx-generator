//
//  jsbScriptingCore.cpp
//  testmonkey
//
//  Created by Rolando Abarca on 3/14/12.
//  Copyright (c) 2012 Zynga Inc. All rights reserved.
//

#include <iostream>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <map>
#include "jsbScriptingCore.h"
#include "jsdbgapi.h"
#include "jsbutils.h"

using namespace jsb;

js_proxy_t *_native_js_global_ht = NULL;
js_proxy_t *_js_native_global_ht = NULL;
js_type_class_t *_js_global_type_ht = NULL;
char *_js_log_buf = NULL;

std::vector<sc_register_sth> registrationList;

// port ~> socket map
std::map<int,int> ports_sockets;

static void registerDefaultClasses(JSContext* cx, JSObject* global);

void ScriptingCore::js_log(const char *format, ...) {
    if (_js_log_buf == NULL) {
        _js_log_buf = (char *)calloc(sizeof(char), 257);
    }
    va_list vl;
    va_start(vl, format);
    int len = vsnprintf(_js_log_buf, 256, format, vl);
    va_end(vl);
    if (len) {
        fprintf(stderr, "JS: %s\n", _js_log_buf);
    }
}

void sc_finalize(JSFreeOp *freeOp, JSObject *obj) {
    return;
}

static JSClass global_class = {
    "global", JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, sc_finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

ScriptingCore::ScriptingCore()
{
    // set utf8 strings internally (we don't need utf16)
    JS_SetCStringsAreUTF8();
    this->addRegisterCallback(registerDefaultClasses);
}

void ScriptingCore::string_report(jsval val) {
    if (JSVAL_IS_NULL(val)) {
        js_log("val : (JSVAL_IS_NULL(val)");
        // return 1;
    } else if ((JSVAL_IS_BOOLEAN(val)) &&
               (JS_FALSE == (JSVAL_TO_BOOLEAN(val)))) {
        js_log("val : (return value is JS_FALSE");
        // return 1;
    } else if (JSVAL_IS_STRING(val)) {
        JSString *str = JS_ValueToString(this->getGlobalContext(), val);
        if (NULL == str) {
            js_log("val : return string is NULL");
        } else {
            js_log("val : return string =\n%s\n",
                 JS_EncodeString(this->getGlobalContext(), str));
        }
    } else if (JSVAL_IS_NUMBER(val)) {
        double number;
        if (JS_FALSE ==
            JS_ValueToNumber(this->getGlobalContext(), val, &number)) {
            js_log("val : return number could not be converted");
        } else {
            js_log("val : return number =\n%f", number);
        }
    }
}

// FIXME - evalString() unused. Remove?
//
// JSBool ScriptingCore::evalString(const char *string, jsval *outVal, const char *filename)
// {
//     jsval rval;
//     const char *fname = (filename ? filename : "NULL");
//     JSScript* script = JS_CompileScript(cx, global, string, strlen(string), filename, 1);
//     if (script) {
//         JSBool evaluatedOK = JS_ExecuteScript(cx, global, script, &rval);
//         if (JS_FALSE == evaluatedOK) {
//             js_log("(evaluatedOK == JS_FALSE)");
//         } else {
//             this->string_report(rval);
//         }

//         return evaluatedOK;
//     }
//     return false;
// }

void ScriptingCore::start() {
    // for now just this
    this->createGlobalContext();
}

void ScriptingCore::addRegisterCallback(sc_register_sth callback) {
    registrationList.push_back(callback);
}

JSObject* NewGlobalObject(JSContext* cx)
{
    JSObject* glob = JS_NewGlobalObject(cx, &global_class, NULL);
    if (!glob) {
        return NULL;
    }

    JSAutoEnterCompartment ac;
    ac.enter(cx, glob);

    if (!JS_InitStandardClasses(cx, glob))
        return NULL;
    if (!JS_InitReflect(cx, glob))
        return NULL;
    if (!JS_DefineDebuggerObject(cx, glob))
        return NULL;

    return glob;
}

void ScriptingCore::createGlobalContext() {
    if (this->cx && this->rt) {
        ScriptingCore::removeAllRoots(this->cx);
        JS_DestroyContext(this->cx);
        JS_DestroyRuntime(this->rt);
        this->cx = NULL;
        this->rt = NULL;
    }
    this->rt = JS_NewRuntime(10 * 1024 * 1024);
    this->cx = JS_NewContext(rt, 10240);
    JS_SetOptions(this->cx, JSOPTION_TYPE_INFERENCE);
    JS_SetVersion(this->cx, JSVERSION_LATEST);
    JS_SetOptions(this->cx, JS_GetOptions(this->cx) & ~JSOPTION_METHODJIT);
    JS_SetOptions(this->cx, JS_GetOptions(this->cx) & ~JSOPTION_METHODJIT_ALWAYS);
    JS_SetErrorReporter(this->cx, ScriptingCore::reportError);
    this->global = NewGlobalObject(this->cx);//JS_NewCompartmentAndGlobalObject(cx, &global_class, NULL);
    for (std::vector<sc_register_sth>::iterator it = registrationList.begin(); it != registrationList.end(); it++) {
        sc_register_sth callback = *it;
        callback(this->cx, this->global);
    }
}

JSBool ScriptingCore::runScript(const char *path)
{
    cocos2d::CCFileUtils *futil = cocos2d::CCFileUtils::sharedFileUtils();
    if (!path) {
        return false;
    }
    std::string rpath;
    if (path[0] == '/') {
        rpath = path;
    } else {
        rpath = futil->fullPathFromRelativePath(path);
    }

    JSScript* script = JS_CompileUTF8File(cx, global, rpath.c_str());
    jsval rval;
    JSBool evaluatedOK = false;
    if (script) {
        JSAutoEnterCompartment ac;
        ac.enter(cx, global);
        evaluatedOK = JS_ExecuteScript(cx, global, script, &rval);
        if (JS_FALSE == evaluatedOK) {
            fprintf(stderr, "(evaluatedOK == JS_FALSE)\n");
        }
    }
    return evaluatedOK;
}

ScriptingCore::~ScriptingCore()
{
    JS_DestroyContext(cx);
    JS_DestroyRuntime(rt);
    JS_ShutDown();
    if (_js_log_buf) {
        free(_js_log_buf);
        _js_log_buf = NULL;
    }
}

JSBool ScriptingCore::setReservedSpot(uint32_t i, JSObject *obj, jsval value) {
	JS_SetReservedSlot(obj, i, value);
	return JS_TRUE;
}

static void dumpNamedRoot(const char *name, void *addr,  JSGCRootType type, void *data)
{
    ScriptingCore::js_log("There is a root named '%s' at %p\n", name, addr);
}

static void rootObject(JSContext *cx, JSObject *obj) {
    JS_AddNamedObjectRoot(cx, &obj, "unnamed");
}


static void unRootObject(JSContext *cx, JSObject *obj) {
    JS_RemoveObjectRoot(cx, &obj);
}

long long jsval_to_long_long(JSContext *cx, jsval v) {
    JSObject *tmp = JSVAL_TO_OBJECT(v);
    if (JS_IsTypedArrayObject(tmp, cx) && JS_GetTypedArrayByteLength(tmp, cx) == 8) {
        uint32_t *data = (uint32_t *)JS_GetUint32ArrayData(tmp, cx);
        long long r = (long long)(*data);
        return r;
    }
    return 0;
}

std::string jsval_to_std_string(JSContext *cx, jsval v) {
    JSString *tmp = JS_ValueToString(cx, v);
    char *rawStr = JS_EncodeString(cx, tmp);
    std::string ret = std::string(rawStr);
    JS_free(cx, rawStr);
    return ret;
}

const char* jsval_to_c_string(JSContext *cx, jsval v) {
    JSString *tmp = JS_ValueToString(cx, v);
    return JS_EncodeString(cx, tmp);
}

jsval long_long_to_jsval(JSContext* cx, long long v) {
    JSObject *tmp = JS_NewUint32Array(cx, 2);
    uint32_t *data = (uint32_t *)JS_GetArrayBufferViewData(tmp, cx);
    data[0] = ((uint32_t *)(&v))[0];
    data[1] = ((uint32_t *)(&v))[1];
    return OBJECT_TO_JSVAL(tmp);
}

jsval std_string_to_jsval(JSContext* cx, std::string& v) {
    JSString *str = JS_NewStringCopyZ(cx, v.c_str());
    return STRING_TO_JSVAL(str);
}

jsval c_string_to_jsval(JSContext* cx, const char* v) {
    JSString *str = JS_NewStringCopyZ(cx, v);
    return STRING_TO_JSVAL(str);
}

// socket code
// open a socket, bind it to a port and start listening, all at once :)
JSBool jsSocketOpen(JSContext* cx, unsigned argc, jsval* vp)
{
    if (argc == 2) {
        jsval* argv = JS_ARGV(cx, vp);
        int port = JSVAL_TO_INT(argv[0]);
        JSObject* callback = JSVAL_TO_OBJECT(argv[1]);

        int s;
        s = ports_sockets[port];
        if (!s) {
            char myname[256];
            struct sockaddr_in sa;
            struct hostent *hp;
            memset(&sa, 0, sizeof(struct sockaddr_in));
            gethostname(myname, 256);
            hp = gethostbyname(myname);
            sa.sin_family = hp->h_addrtype;
            sa.sin_port = htons(port);
            if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
                JS_ReportError(cx, "error opening socket");
                return JS_FALSE;
            }
            int optval = 1;
            if ((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) < 0) {
                close(s);
                JS_ReportError(cx, "error setting socket options");
                return JS_FALSE;
            }
            if ((bind(s, (const struct sockaddr *)&sa, sizeof(struct sockaddr_in))) < 0) {
                close(s);
                JS_ReportError(cx, "error binding socket");
                return JS_FALSE;
            }
            listen(s, 1);
            int clientSocket;
            if ((clientSocket = accept(s, NULL, NULL)) > 0) {
                ports_sockets[port] = clientSocket;
                jsval fval = OBJECT_TO_JSVAL(callback);
                jsval jsSocket = INT_TO_JSVAL(clientSocket);
                jsval outVal;
                JS_CallFunctionValue(cx, NULL, fval, 1, &jsSocket, &outVal);
            }
        } else {
            // just call the callback with the client socket
            jsval fval = OBJECT_TO_JSVAL(callback);
            jsval jsSocket = INT_TO_JSVAL(s);
            jsval outVal;
            JS_CallFunctionValue(cx, NULL, fval, 1, &jsSocket, &outVal);
        }
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(s));
    }
    return JS_TRUE;
}

JSBool jsSocketRead(JSContext* cx, unsigned argc, jsval* vp)
{
    if (argc == 1) {
        jsval* argv = JS_ARGV(cx, vp);
        int s = JSVAL_TO_INT(argv[0]);
        char buff[1024];
        JSString* outStr = JS_NewStringCopyZ(cx, "");

        size_t bytesRead;
        while ((bytesRead = read(s, buff, 1024)) > 0) {
            JSString* newStr = JS_NewStringCopyN(cx, buff, bytesRead);
            outStr = JS_ConcatStrings(cx, outStr, newStr);
            // break on new line
            if (buff[bytesRead-1] == '\n') {
                break;
            }
        }
        JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(outStr));
    } else {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
    }
    return JS_TRUE;
}

JSBool jsSocketWrite(JSContext* cx, unsigned argc, jsval* vp)
{
    if (argc == 2) {
        jsval* argv = JS_ARGV(cx, vp);
        int s;
        const char* str;

        s = JSVAL_TO_INT(argv[0]);
        JSString* jsstr = JS_ValueToString(cx, argv[1]);
        str = JS_EncodeString(cx, jsstr);

        write(s, str, strlen(str));

        JS_free(cx, (void*)str);
    }
    return JS_TRUE;
}

JSBool jsSocketClose(JSContext* cx, unsigned argc, jsval* vp)
{
    if (argc == 1) {
        jsval* argv = JS_ARGV(cx, vp);
        int s = JSVAL_TO_INT(argv[0]);
        close(s);
    }
    return JS_TRUE;
}

static void registerDefaultClasses(JSContext* cx, JSObject* global) {
    if (!JS_InitStandardClasses(cx, global)) {
        ScriptingCore::js_log("error initializing the standard classes");
    }

    //
    // Javascript controller (__jsc__)
    //
    JSObject *jsc = JS_NewObject(cx, NULL, NULL, NULL);
    jsval jscVal = OBJECT_TO_JSVAL(jsc);
    JS_SetProperty(cx, global, "__jsc__", &jscVal);

    JS_DefineFunction(cx, jsc, "garbageCollect", jsb::utils::forceGC, 0, JSPROP_READONLY | JSPROP_PERMANENT | JSPROP_ENUMERATE );
    JS_DefineFunction(cx, jsc, "dumpRoot", jsb::utils::debug::dumpRoot, 0, JSPROP_READONLY | JSPROP_PERMANENT | JSPROP_ENUMERATE );
    JS_DefineFunction(cx, jsc, "addGCRootObject", jsb::utils::addRootJS, 1, JSPROP_READONLY | JSPROP_PERMANENT | JSPROP_ENUMERATE );
    JS_DefineFunction(cx, jsc, "removeGCRootObject", jsb::utils::removeRootJS, 1, JSPROP_READONLY | JSPROP_PERMANENT | JSPROP_ENUMERATE );

    // register some global functions
    JS_DefineFunction(cx, global, "require", jsb::utils::executeScript, 1, JSPROP_READONLY | JSPROP_PERMANENT);
    JS_DefineFunction(cx, global, "log", jsb::utils::log, 0, JSPROP_READONLY | JSPROP_PERMANENT);
    JS_DefineFunction(cx, global, "forceGC", jsb::utils::forceGC, 0, JSPROP_READONLY | JSPROP_PERMANENT);
    // should be used only for debug
    JS_DefineFunction(cx, global, "newGlobal", jsb::utils::debug::jsNewGlobal, 1, JSPROP_READONLY | JSPROP_PERMANENT);

    // register the server socket
    JS_DefineFunction(cx, global, "_socketOpen", jsSocketOpen, 1, JSPROP_READONLY | JSPROP_PERMANENT);
    JS_DefineFunction(cx, global, "_socketWrite", jsSocketWrite, 1, JSPROP_READONLY | JSPROP_PERMANENT);
    JS_DefineFunction(cx, global, "_socketRead", jsSocketRead, 1, JSPROP_READONLY | JSPROP_PERMANENT);
    JS_DefineFunction(cx, global, "_socketClose", jsSocketClose, 1, JSPROP_READONLY | JSPROP_PERMANENT);
}
