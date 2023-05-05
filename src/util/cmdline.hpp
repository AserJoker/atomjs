#pragma once
#include <fmt/format.h>
#include <functional>
#include <map>
#include <sstream>
#include <string>
class cmdline {
public:
  struct ArgInfo {
    std::string name;
    std::string snapshot;
    std::string descrition;
  };

private:
  std::map<std::string, std::function<void(const std::string &)>> _args;

  std::vector<ArgInfo> _infos;

  std::vector<std::string> *_others;

public:
  void parse(int argc, char **argv, int offset = 1) {
    for (int index = offset; index < argc; index++) {
      std::string arg = argv[index];
      if (arg.starts_with("--")) {
        std::string flag = arg.substr(2);
        auto split = flag.find("=");
        if (split == std::string::npos) {
          std::string val = argv[index + 1];
          if (val.starts_with("-")) {
            val = "";
          } else {
            index++;
          }
          if (_args.contains(flag)) {
            _args[flag](val);
          }
        } else {
          auto val = flag.substr(split + 1);
          flag = flag.substr(0, split);
          if (_args.contains(flag)) {
            _args[flag](val);
          }
        }
      } else if (arg.starts_with("-")) {
        std::string flag = arg.substr(1);
        auto split = flag.find("=");
        if (split == std::string::npos) {
          for (auto it = _infos.begin(); it != _infos.end(); it++) {
            if (it->snapshot == flag) {
              flag = it->name;
              break;
            }
          }
          std::string val = argv[index + 1];
          if (val.starts_with("-")) {
            val = "";
          } else {
            index++;
          }
          if (_args.contains(flag)) {
            _args[flag](val);
          }
        } else {
          auto val = flag.substr(split + 1);
          flag = flag.substr(0, split);
          for (auto it = _infos.begin(); it != _infos.end(); it++) {
            if (it->snapshot == flag) {
              flag = it->name;
              break;
            }
          }
          if (_args.contains(flag)) {
            _args[flag](val);
          }
        }
      } else {
        if (_others) {
          _others->push_back(arg);
        }
      }
    }
  }
  void registerArg(const ArgInfo &info, bool *receive) {
    _infos.push_back(info);
    _args[info.name] = [=](const std::string &val) -> void {
      *receive = val == "" || val == "true";
    };
  }
  template <class T> void registerArg(const ArgInfo &info, T *receive) {
    _infos.push_back(info);
    _args[info.name] = [=](const std::string &val) -> void {
      std::stringstream ss(val);
      ss >> *receive;
    };
  }
  template <class T>
  void registerArg(const ArgInfo &info, std::vector<T> &receive) {
    _infos.push_back(info);
    _args[info.name] = [&](const std::string &val) -> void {
      std::stringstream ss(val);
      T value;
      ss >> value;
      receive.push_back(value);
    };
  }
  void registerOthers(std::vector<std::string> *others) { _others = others; }

  void help() {
    fmt::print("Usage: atom version|help|run|compile [options] files ...\n");
    fmt::print("Options:\n");
    for (auto &arg : _infos) {
      std::string flag;
      if (arg.snapshot.empty()) {
        flag = arg.name;
      } else {
        flag = "-" + arg.snapshot + ",-" + arg.name;
      }
      while (flag.length() < 30) {
        flag += " ";
      }
      fmt::print("  {}{}\n", flag, arg.descrition);
    }
  }
};