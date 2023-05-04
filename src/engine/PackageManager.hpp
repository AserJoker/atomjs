#pragma once
#include "Runtime.hpp"
#include "util/ioutil.hpp"
#include <cjson/cJSON.h>
#include <fmt/format.h>
class PackageManager {
private:
  struct Package {
    std::string path;
    std::string index;
    std::string entry;
    std::string id;
    std::string version;
    std::map<std::string, std::string> dependences;
    bool load;
  };
  std::map<std::string, Package> _packages;

public:
  Package loadManifest(const std::string &p) {
    Package package;
    auto data = ioutil::readAll(p);
    auto root = cJSON_Parse(data.data());
    if (!root) {
      throw std::runtime_error("failed to resolve package");
    }
    auto index = cJSON_GetObjectItemCaseSensitive(root, "index");
    if (index) {
      package.index = index->valuestring;
    }
    auto entry = cJSON_GetObjectItemCaseSensitive(root, "entry");
    if (entry) {
      package.entry = entry->valuestring;
    }
    auto id = cJSON_GetObjectItemCaseSensitive(root, "id");
    if (id) {
      package.id = id->valuestring;
    }
    auto version = cJSON_GetObjectItemCaseSensitive(root, "version");
    if (version) {
      package.version = version->valuestring;
    }
    auto dependences = cJSON_GetObjectItemCaseSensitive(root, "dependences");
    if (dependences && cJSON_IsObject(dependences)) {
      auto c = dependences->child;
      while (c) {
        if (cJSON_IsString(c)) {
          package.dependences[c->string] = c->valuestring;
        }
        c = c->next;
      }
    }
    cJSON_free(root);
    return package;
  }
  Package loadPackage(Runtime *rt, const std::string &packagePath) {
    auto manifestPath = path::join({packagePath, "manifest.json"});
    if (manifestPath.isExist()) {
      auto package = loadManifest(manifestPath.string());
      package.path = packagePath;
      package.load = false;
      _packages[package.id] = package;
      return package;
    }
    return {};
  }
  void loadPackages(Runtime *rt, const std::string &packageRoot) {
    auto directory = std::filesystem::directory_iterator(
        path::absolute(packageRoot).string());
    for (auto &item : directory) {
      if (item.is_directory()) {
        loadPackage(rt, item.path().string());
      }
    }
    for (auto &[id, package] : _packages) {
      if (!package.index.empty()) {
        rt->alias(id, path::join({package.path, package.index}).string());
      }
    }
  }
  void run(Runtime *rt, const std::string &id) {
    if (_packages.contains(id)) {
      auto package = _packages.at(id);
      if (!package.entry.empty()) {
        auto entry = path::join({package.path, package.entry});
        rt->exec(fmt::format("import '{}';", entry.string()));
      }
    }
  }
  bool exist(const std::string &pack) { return _packages.contains(pack); }
};