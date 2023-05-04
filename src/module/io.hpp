#pragma once
#include "engine/Runtime.hpp"
#include "quickjs.h"
#include <fcntl.h>

class io {
private:
  static JSValue println(JSContext *ctx, JSValue self, int argc,
                         JSValue *argv) {
    auto msg = JS_ToCString(ctx, argv[0]);
    std::cout << msg << std::endl;
    JS_FreeCString(ctx, msg);
    return JS_UNDEFINED;
  }

  static int init(JSContext *ctx, JSModuleDef *m) {
    JS_SetModuleExport(ctx, m, "println",
                       JS_NewCFunction(ctx, &println, "println", 1));
    return 0;
  }

public:
  static void registerTo(Runtime *rt) {
    rt->registerModule({.init = &init, .name = "io", .exports = {"println"}});
  }
};