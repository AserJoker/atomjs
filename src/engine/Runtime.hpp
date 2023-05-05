#pragma once
#include "util/ioutil.hpp"
#include "util/path.hpp"
#include <cstring>
#include <map>
#include <quickjs.h>
#include <sstream>
#include <stdexcept>
#include <stdint.h>

class Runtime {
public:
  struct ModuleDescription {
    JSModuleInitFunc *init;
    std::string name;
    std::vector<std::string> exports;
  };

  struct FunctionDefinition {
    JSCFunction *entry;
    int length;
    std::string name;
  };

  struct ObjectDefinition {
    std::map<std::string, JSValue> proto;
  };

  struct ClassDefinition {
    std::map<std::string, JSValue> proto;
    JSClassFinalizer *dispose;
    std::string name;
  };

private:
  JSRuntime *_runtime;
  JSContext *_context;

  std::map<std::string, std::string> _paths;

  static JSModuleDef *loadJS(JSContext *ctx, const std::vector<char> &data,
                             const char *name) {
    auto val = JS_Eval(ctx, data.data(), strlen(data.data()), name,
                       JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_COMPILE_ONLY);

    if (JS_IsException(val)) {
      return nullptr;
    }
    auto m = (JSModuleDef *)(JS_VALUE_GET_PTR(val));
    JS_FreeValue(ctx, val);
    return m;
  }

  static JSModuleDef *loadModule(JSContext *ctx, const std::vector<char> &data,
                                 const char *name) {
    auto val = JS_ReadObject(ctx, (uint8_t *)data.data(), data.size(),
                             JS_READ_OBJ_BYTECODE);
    auto m = (JSModuleDef *)(JS_VALUE_GET_PTR(val));
    JS_FreeValue(ctx, val);
    return m;
  }

  static char *normalize(JSContext *ctx, const char *source_name,
                         const char *target_name, void *runtime) {
    if (strcmp(source_name, "<internel>") == 0) {
      path result = path::absolute(std::string(target_name));
      char *buf = (char *)js_malloc(ctx, result.string().length() + 1);
      memcpy(buf, result.string().c_str(), result.string().length() + 1);
      return buf;
    }
    auto rt = (Runtime *)runtime;
    path source = std::string(source_name);
    path target = std::string(target_name);
    path result;
    if (target_name[0] == '.') {
      result = path::join({path::absolute(source).dirname(), target});
    } else {
      if (rt->_paths.contains(target_name)) {
        result = rt->_paths.at(target_name);
      } else {
        auto *buf = (char *)js_malloc(ctx, strlen(target_name) + 1);
        memcpy(buf, target_name, strlen(target_name) + 1);
        return buf;
      }
    }
    if (result.empty()) {
      result = target;
    }
    if (!result.isExist()) {
      auto dirname = result.dirname();
      auto filename = result.filename();
      if (path::join({dirname, filename + ".js"}).isExist()) {
        result = path::join({dirname, filename + ".js"});
      } else if (path::join({dirname, filename + ".module"}).isExist()) {
        result = path::join({dirname, filename + ".module"});
      } else if (path::join({result, std::string("index.js")}).isExist()) {
        result = path::join({result, std::string("index.js")});
      } else if (path::join({result, std::string("index.module")}).isExist()) {
        result = path::join({result, std::string("index.module")});
      }
    }
    result = path::absolute(result);
    auto s_result = result.string();
    char *buf = (char *)js_malloc(ctx, s_result.length() + 1);
    memcpy(buf, s_result.c_str(), s_result.length() + 1);
    return buf;
  }

  static JSModuleDef *loader(JSContext *ctx, const char *name, void *runtime) {
    auto data = ioutil::readAll(name);
    if (data.empty()) {
      JS_ThrowReferenceError(ctx, "could not load module '%s'", name);
      return nullptr;
    }
    if (std::string(name).ends_with(".module")) {
      return loadModule(ctx, data, name);
    }
    return loadJS(ctx, data, name);
  }

  void resolveError() {
    JSValue expr = JS_GetException(_context);
    auto message = JS_ToCString(_context, expr);
    JSValue vstack = JS_GetPropertyStr(_context, expr, "stack");
    auto stack = JS_ToCString(_context, vstack);
    std::stringstream ss;
    ss << message << "\n" << stack;
    JS_FreeCString(_context, stack);
    JS_FreeValue(_context, vstack);
    JS_FreeCString(_context, message);
    JS_FreeValue(_context, expr);
    throw std::runtime_error(ss.str());
  }

public:
  Runtime() : _runtime(nullptr), _context(nullptr) {
    _runtime = JS_NewRuntime();
    _context = JS_NewContext(_runtime);
    JS_SetModuleLoaderFunc(_runtime, normalize, &loader, this);
  }

  virtual ~Runtime() {
    JS_FreeContext(_context);
    JS_FreeRuntime(_runtime);
  }

  void exec(const std::string &source) {
    JSValue result = JS_Eval(_context, source.c_str(), source.length(),
                             "<internel>", JS_EVAL_TYPE_MODULE);
    if (JS_IsException(result)) {
      resolveError();
    }
    JS_FreeValue(_context, result);
  }

  void wait() {
    while (true) {
      JSContext *ctx = nullptr;
      auto res = JS_ExecutePendingJob(_runtime, &ctx);
      if (res == 0) {
        break;
      }
      if (res < 0) {
        resolveError();
      }
    }
  }

  void registerModule(const ModuleDescription &desc) {
    auto m = JS_NewCModule(_context, desc.name.c_str(), desc.init);
    for (auto &exp : desc.exports) {
      JS_AddModuleExport(_context, m, exp.c_str());
    }
  }

  void alias(const std::string &name, const std::string &p) {
    _paths[name] = p;
  }

  JSValue valueOf(const int &val) { return JS_NewInt32(_context, val); }
  JSValue valueOf(const double &val) { return JS_NewFloat64(_context, val); }
  JSValue valueOf(const std::string &val) {
    return JS_NewString(_context, val.c_str());
  }
  JSValue valueOf(const FunctionDefinition &def) {
    return JS_NewCFunction(_context, def.entry, def.name.c_str(), def.length);
  }
  JSValue valueOf(const ObjectDefinition &def) {
    JSValue obj = JS_NewObject(_context);
    for (auto &[key, val] : def.proto) {
      JS_SetPropertyStr(_context, obj, key.c_str(), val);
    }
    return obj;
  }
  JSClassID classOf(const ClassDefinition &def) {
    JSClassID cid = 0;
    JS_NewClassID(&cid);
    JSClassDef cdef;
    cdef.class_name = def.name.c_str();
    cdef.finalizer = def.dispose;
    cdef.call = nullptr;
    cdef.exotic = nullptr;
    cdef.gc_mark = nullptr;
    JS_NewClass(_runtime, cid, &cdef);
    JSValue proto = valueOf({def.proto});
    JS_SetClassProto(_context, cid, proto);
    return cid;
  }
};
#define JSFUNCTION(func)                                                       \
  JSValue func(JSContext *ctx, JSValue self, int argc, JSValue *argv)
