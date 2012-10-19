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
#include "cocos2d.h"
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
    JSBool evalString(const char *string, jsval *outVal, const char *filename = NULL);

    /**
     * will run the specified string
     * @param string The path of the script to be run
     */
    JSBool runScript(const char *path, JSObject* glob = NULL, JSContext* cx_ = NULL);

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

    void registerDefaultClasses(JSContext* cx, JSObject* global);

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

    ~ScriptingCore();

protected:

    JSRuntime *rt;
    JSContext *cx;
    JSObject  *global;

    ScriptingCore();

    void string_report(jsval val);

    static void executeJSFunctionWithName(JSContext *cx, JSObject *obj,
                                          const char *funcName, jsval &dataVal,
                                          jsval &retval);

};

} //namespace jsb

// some utility functions
// to native
long long jsval_to_long_long(JSContext *cx, jsval v);
std::string jsval_to_std_string(JSContext *cx, jsval v);
// you should free this pointer after you're done with it
const char* jsval_to_c_string(JSContext *cx, jsval v);
cocos2d::CCPoint jsval_to_ccpoint(JSContext *cx, jsval v);
cocos2d::CCRect jsval_to_ccrect(JSContext *cx, jsval v);
cocos2d::CCSize jsval_to_ccsize(JSContext *cx, jsval v);
cocos2d::ccGridSize jsval_to_ccgridsize(JSContext *cx, jsval v);
cocos2d::ccColor4B jsval_to_cccolor4b(JSContext *cx, jsval v);
cocos2d::ccColor4F jsval_to_cccolor4f(JSContext *cx, jsval v);
cocos2d::ccColor3B jsval_to_cccolor3b(JSContext *cx, jsval v);
cocos2d::CCArray* jsval_to_ccarray(JSContext* cx, jsval v);
jsval ccarray_to_jsval(JSContext* cx, cocos2d::CCArray *arr);
// from native
jsval long_long_to_jsval(JSContext* cx, long long v);
jsval std_string_to_jsval(JSContext* cx, std::string& v);
jsval c_string_to_jsval(JSContext* cx, const char* v);
jsval ccpoint_to_jsval(JSContext* cx, cocos2d::CCPoint& v);
jsval ccrect_to_jsval(JSContext* cx, cocos2d::CCRect& v);
jsval ccsize_to_jsval(JSContext* cx, cocos2d::CCSize& v);
jsval ccgridsize_to_jsval(JSContext* cx, cocos2d::ccGridSize& v);
jsval cccolor4b_to_jsval(JSContext* cx, cocos2d::ccColor4B& v);
jsval cccolor4f_to_jsval(JSContext* cx, cocos2d::ccColor4F& v);
jsval cccolor3b_to_jsval(JSContext* cx, cocos2d::ccColor3B& v);

// this is a server socket
JSBool jsSocketOpen(JSContext* cx, unsigned argc, jsval* vp);
JSBool jsSocketRead(JSContext* cx, unsigned argc, jsval* vp);
JSBool jsSocketWrite(JSContext* cx, unsigned argc, jsval* vp);
JSBool jsSocketClose(JSContext* cx, unsigned argc, jsval* vp);

#endif // __JSB_SCRIPTING_CORE_H__
