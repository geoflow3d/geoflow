#include <DLLoader.h>

#include <geoflow/geoflow.hpp>

namespace geoflow {

  class PluginManager {
    public:
    PluginManager() {};
    ~PluginManager() {
      // unload();
    };
    void load(std::string& plugin_directory, NodeRegisterMap& node_registers, bool verbose=false) {
      for(auto& p: fs::directory_iterator(plugin_directory)) {
        if (p.path().extension() == GF_PLUGIN_EXTENSION) {
          const std::string path = p.path().string();
          const std::string plugin_target_name = p.path().stem().string();
          
          std::cout << "Loading " << path << " ...\n";
          
          dloaders_.emplace(path, std::make_unique<DLLoader>(path, plugin_target_name));
          
          if (dloaders_[path]->DLOpenLib()) {
            auto [reg, success] = node_registers.emplace( dloaders_[path]->DLGetInstance() );
            if (verbose) {
              for (auto& [key, val] : reg->second->node_types) {
                std::cout << "loaded type: " << key << "\n";
              }
            std::cout << "... success :)\n";
            }
          } else {
            dloaders_.erase(path);
            std::cerr << "... failed :(\n";
          }
        }
      }
    }

    void unload() {
      for (auto& [path, loader] : dloaders_) {
        std::cout << "Unloading " << path << "\n";
        loader->DLCloseLib();
      }
    }

    private:
    typedef dlloader::DLLoader<geoflow::NodeRegister> DLLoader;
    std::unordered_map<std::string, std::unique_ptr<DLLoader>> dloaders_;

  };

}