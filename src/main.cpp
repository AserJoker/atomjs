#include "engine/Runtime.hpp"
#include "module/io.hpp"
#include <exception>
#include <fstream>
#include <quickjs.h>
#include <stdint.h>

int main(int argc, char **argv) {
  try {
    Runtime rt;
    io::registerTo(&rt);
    rt.exec("import './packages/demo/src/index.js';");
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  return 0;
}