//
//  jsbScriptingCore.h
//  testmonkey
//
//  Created by Rolando Abarca on 3/14/12.
//  Copyright (c) 2012 Zynga Inc. All rights reserved.
//

#ifndef __JSB_SCRIPTING_CORE_H__
#define __JSB_SCRIPTING_CORE_H__

#include <assert.h>
#include "uthash.h"
#include "jsapi.h"
#include "jsfriendapi.h"
#include "spidermonkey_specifics.h"

typedef void (*sc_register_sth)(JSContext* cx, JSObject* global);

namespace jsb {

class ScriptingCore
{

public:

    static ScriptingCore *getInstance() {
        static ScriptingCore instance;
        return &instance;
    };

    /**
     * initialize everything
     */
    void start();

    /**
     * will eval the specified string
     * @param string The string with the javascript code to be evaluated
     * @param outVal The jsval that will hold the return value of the evaluation.
     * Can be NULL.
     */
    // FIXME - evalString() unused. Remove?
    // JSBool evalString(const char *string, jsval *outVal, const char *filename = NULL);

    /**
     * will run the specified string
     * @param string The path of the script to be run
     */
    virtual JSBool runScript(const char *path);

    /**
     * run a script from script :)
     */
    static JSBool executeScript(JSContext *cx, uint32_t argc, jsval *vp);

    /**
     * Will create a new context. If one is already there, it will destroy the old context
     * and create a new one.
     */
    void createGlobalContext();

    /**
     * @return the global context
     */
    JSContext* getGlobalContext() {
        return cx;
    };

    /**
     * will add the register_sth callback to the list of functions that need to be called
     * after the creation of the context
     */
    void addRegisterCallback(sc_register_sth callback);

    /**
     * @param cx
     * @param message
     * @param report
     */
    static void reportError(JSContext *cx, const char *message, JSErrorReport *report);

    static void js_log(const char *format, ...);

    /**
     * Log something using CCLog
     * @param cx
     * @param argc
     * @param vp
     */
    static JSBool log(JSContext *cx, uint32_t argc, jsval *vp);

    JSBool setReservedSpot(uint32_t i, JSObject *obj, jsval value);

    /**
     * Force a cycle of GC
     * @param cx
     * @param argc
     * @param vp
     */
    static JSBool forceGC(JSContext *cx, uint32_t argc, jsval *vp);

    static void removeAllRoots(JSContext *cx);

    static JSBool dumpRoot(JSContext *cx, uint32_t argc, jsval *vp);

    static JSBool addRootJS(JSContext *cx, uint32_t argc, jsval *vp);

    static JSBool removeRootJS(JSContext *cx, uint32_t argc, jsval *vp);

    static void executeJSFunctionWithName(JSContext *cx, JSObject *obj,
                                          const char *funcName, jsval &dataVal,
                                          jsval &retval);

    ~ScriptingCore();

protected:

    JSRuntime *rt;
    JSContext *cx;
    JSObject  *global;

    ScriptingCore();

    void string_report(jsval val);

};

} //namespace jsb

// some utility functions
// to native
long long jsval_to_long_long(JSContext *cx, jsval v);
std::string jsval_to_std_string(JSContext *cx, jsval v);
// you should free this pointer after you're done with it
const char* jsval_to_c_string(JSContext *cx, jsval v);
// from native
jsval long_long_to_jsval(JSContext* cx, long long v);
jsval std_string_to_jsval(JSContext* cx, std::string& v);
jsval c_string_to_jsval(JSContext* cx, const char* v);

// this is a server socket
JSBool jsSocketOpen(JSContext* cx, unsigned argc, jsval* vp);
JSBool jsSocketRead(JSContext* cx, unsigned argc, jsval* vp);
JSBool jsSocketWrite(JSContext* cx, unsigned argc, jsval* vp);
JSBool jsSocketClose(JSContext* cx, unsigned argc, jsval* vp);

#endif // __JSB_SCRIPTING_CORE_H__
