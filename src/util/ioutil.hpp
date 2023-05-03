#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class ioutil {
public:
  static std::vector<char> readAll(const std::string &path) {
    std::ifstream input(path);
    if (input.is_open()) {
      input.seekg(0, std::ios::end);
      uint32_t size = input.tellg();
      std::vector<char> buf(size + 1);
      buf[size] = 0;
      input.seekg(0, std::ios::beg);
      input.read(buf.data(), buf.size() - 1);
      input.close();
      return buf;
    }
    return {};
  }
};