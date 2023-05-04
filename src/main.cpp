#include "engine/PackageManager.hpp"
#include "engine/Runtime.hpp"
#include "module/io.hpp"
#include "util/cmdline.hpp"
#include <exception>
#include <fstream>
#include <quickjs.h>
#include <stdint.h>

int main(int argc, char **argv) {

  path result = path::join({"/a/b/c", "../.././b"});
  std::cout << result.string();
  return 0;
  try {
    int argc = 3;
    const char *argv[] = {"", "run", "demo"};
    cmdline cmd;
    std::vector<std::string> packages;
    std::vector<std::string> files;
    cmd.registerOthers(&files);
    cmd.registerArg({"package", "pack", "packages path"}, packages);
    if (argc < 2) {
      cmd.help();
      return 0;
    }
    cmd.parse(argc, (char **)argv, 2);
    Runtime rt;
    PackageManager pm;
    io::registerTo(&rt);
    pm.loadPackages(&rt, "./packages");
    std::string subCommand = argv[1];
    if (subCommand == "help") {
      cmd.help();
      return 0;
    }
    if (subCommand == "run") {
      for (auto &file : files) {
        if (pm.exist(file)) {
          pm.run(&rt, file);
        } else {
          path p = path::absolute(file);
          if (p.isDirExist()) {
            auto pack = pm.loadPackage(&rt, p.string());
            if (!pack.entry.empty()) {
              pm.run(&rt, pack.id);
            }
          } else {
            rt.exec(fmt::format("import '{}'", p.string()));
          }
        }
      }
    }
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}