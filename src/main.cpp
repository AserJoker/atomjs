#include "engine/PackageManager.hpp"
#include "engine/Runtime.hpp"
#include "module/io.hpp"
#include "util/cmdline.hpp"
#include <exception>
#include <fstream>
#include <quickjs.h>
#include <stdint.h>

struct CmdLineOption {
  std::vector<std::string> files;
  std::vector<std::string> packages;
};

typedef void (*demo)();

void subCommandRun(CmdLineOption opt, Runtime *rt, PackageManager &pm) {
  for (auto &pack : opt.packages) {
    pm.loadPackages(rt, pack);
  }
  for (auto &file : opt.files) {
    if (pm.exist(file)) {
      pm.run(rt, file);
    } else {
      path p = path::absolute(file);
      if (p.isDirExist()) {
        auto pack = pm.loadPackage(rt, p.string());
        if (!pack.entry.empty()) {
          pm.run(rt, pack.id);
        }
      } else {
        rt->exec(fmt::format("import '{}'", p.string()));
      }
    }
  }
  rt->wait();
}
void subCommandCompile(CmdLineOption opt, Runtime *rt, PackageManager &pm) {}

int main(int argc, char **argv) {
  try {
    int argc = 3;
    const char *argv[] = {"", "run", "demo"};
    cmdline cmd;
    CmdLineOption opt;
    cmd.registerOthers(&opt.files);
    cmd.registerArg({"package", "pack", "packages path"}, opt.packages);
    if (argc < 2) {
      cmd.help();
      return 0;
    }
    std::string subCommand = argv[1];
    cmd.parse(argc, (char **)argv, 2);
    Runtime rt;
    PackageManager pm;
    io::registerTo(&rt);
    pm.loadPackages(&rt, "./packages");
    if (subCommand == "help") {
      cmd.help();
      return 0;
    }
    if (subCommand == "run") {
      subCommandRun(opt, &rt, pm);
      return 0;
    }
    if (subCommand == "compile") {
      return 0;
    }
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}