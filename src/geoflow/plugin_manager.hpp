#include <DLLoader.h>

#include <geoflow/geoflow.hpp>

namespace geoflow {

  class PluginManager {
    public:
    PluginManager() {};
    ~PluginManager() {
      // unload();
    };
    void load(std::string& plugin_directory, NodeRegisterMap& node_registers) {
      for(auto& p: fs::directory_iterator(plugin_directory)) {
        if (p.path().extension() == GF_PLUGIN_EXTENSION) {
          std::string path = p.path().string();
          
          std::cout << "Loading " << path << " ...\n";
          
          dloaders_.emplace(path, std::make_unique<DLLoader>(path));
          
          if (dloaders_[path]->DLOpenLib()) {
            node_registers.emplace( dloaders_[path]->DLGetInstance() );
            std::cout << "... success :)\n";
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