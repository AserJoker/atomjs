#pragma once
#include "engine/Runtime.hpp"
#include "quickjs.h"
#include <cstdio>
#include <fcntl.h>
#include <functional>

class io {
private:
  static JSValue writeline(JSContext *ctx, JSValue self, int argc,
                           JSValue *argv) {
    auto msg = JS_ToCString(ctx, argv[0]);
    std::cout << msg << std::endl;
    JS_FreeCString(ctx, msg);
    return JS_UNDEFINED;
  }

  static JSValue readline(JSContext *ctx, JSValue self, int argc,
                          JSValue *argv) {
    std::string result;
    std::getline(std::cin, result);
    return JS_NewString(ctx, result.c_str());
  }

  static int init(JSContext *ctx, JSModuleDef *m) {
    JS_SetModuleExport(ctx, m, "writeline",
                       JS_NewCFunction(ctx, &writeline, "writeline", 1));
    JS_SetModuleExport(ctx, m, "readline",
                       JS_NewCFunction(ctx, &readline, "readline", 0));
    return 0;
  }

public:
  static void registerTo(Runtime *rt) {
    rt->registerModule(
        {.init = &init, .name = "io", .exports = {"writeline", "readline"}});
  }
};