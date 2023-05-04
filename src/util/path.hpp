#pragma once
#include <filesystem>
#include <string>
#include <vector>

class path {
private:
  std::vector<std::string> _parts;
  void append(const std::string &part) {
    if (part == "." || part.empty()) {
      return;
    }
    if (part == "..") {
      if (!_parts.empty() && _parts[_parts.size() - 1] != "..") {
        _parts.resize(_parts.size() - 1);
        return;
      }
    }
    _parts.push_back(part);
  }

public:
  path(const std::string &p = "") {
    std::string part;
    if (p[0] == '/') {
      append("/");
    }
    for (auto &c : p) {
      if (c == '/' || c == '\\') {
        append(part);
        part = "";
      } else {
        part += c;
      }
    }
    append(part);
  }
  path(const char *p) : path(std::string(p)) {}
  std::string string() const {
    std::string result;
    for (auto &part : _parts) {
      if (!result.empty() && !result.ends_with("/")) {
        result += "/";
      }
      result += part;
    }
    return result;
  }
  std::string filename() const { return _parts[_parts.size() - 1]; }
  path dirname() const {
    path result;
    for (auto &part : _parts) {
      if (result._parts.size() < _parts.size() - 1) {
        result.append(part);
      }
    }
    return result;
  }
  uint32_t length() const { return _parts.size(); }
  std::string &operator[](uint32_t val) { return _parts[val]; }

  void clear() { _parts.clear(); }

  bool empty() const { return _parts.empty(); }

  bool isExist() const { return std::filesystem::exists(string()); }
  bool isDirExist() const {
    return isExist() && std::filesystem::is_directory(string());
  }
  bool isAbsolute() const {
    return !_parts.empty() &&
           (_parts[0] == "/" || _parts[0].find(":") != std::string::npos);
  }
  static path absolute(const path &p) {
    path result = std::filesystem::current_path().string();
    if (!p._parts.empty()) {
      if (p._parts[0] == "/" || p._parts[0].find(":") != std::string::npos) {
        return p;
      }
    }
    for (auto &part : p._parts) {
      result.append(part);
    }
    return result;
  }
  static path join(const std::vector<path> &paths) {
    path result;
    for (auto &pa : paths) {
      if (pa.isAbsolute()) {
        result.clear();
      }
      for (auto &part : pa._parts) {
        result.append(part);
      }
    }
    return result;
  }
};