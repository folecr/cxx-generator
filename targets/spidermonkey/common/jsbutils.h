#ifndef __JSB_UTILS_H__
#define __JSB_UTILS_H__

#include "jsapi.h"
#include "jsfriendapi.h"

namespace jsb {

    namespace utils {

        /**
         * @param cx
         * @param message
         * @param report
         */
        void reportError(JSContext *cx, const char *message, JSErrorReport *report);

        /**
         * Log something using CCLog
         * @param cx
         * @param argc
         * @param vp
         */
        JSBool log(JSContext *cx, uint32_t argc, jsval *vp);

        /**
         * Force a cycle of GC
         * @param cx
         * @param argc
         * @param vp
         */
        JSBool forceGC(JSContext *cx, uint32_t argc, jsval *vp);

        void removeAllRoots(JSContext *cx);

        JSBool addRootJS(JSContext *cx, uint32_t argc, jsval *vp);

        JSBool removeRootJS(JSContext *cx, uint32_t argc, jsval *vp);

        void executeJSFunctionWithName(JSContext *cx, JSObject *obj,
                                       const char *funcName, jsval &dataVal,
                                       jsval &retval);

        /**
         * run a script from script :)
         */
        static JSBool executeScript(JSContext *cx, uint32_t argc, jsval *vp);

        namespace debug {

            JSBool dumpRoot(JSContext *cx, uint32_t argc, jsval *vp);

            JSBool jsNewGlobal(JSContext* cx, unsigned argc, jsval* vp);
        }
    }
}

#endif // __JSB_UTILS_H__
