// This file is part of Geoflow
// Copyright (C) 2018-2022 Ravi Peters

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
                    
          dloaders_.emplace(path, std::make_unique<DLLoader>(path, plugin_target_name));
          
          if (dloaders_[path]->DLOpenLib(verbose)) {
            if (verbose) std::cout << "Loaded " << path << std::endl;
            auto [reg, success] = node_registers.emplace( dloaders_[path]->DLGetInstance() );
          } else {
            dloaders_.erase(path);
          }
        }
      }
    }

    void unload(bool verbose=false) {
      for (auto& [path, loader] : dloaders_) {
        if (verbose) std::cout << "Unloading " << path << "\n";
        loader->DLCloseLib(verbose);
      }
    }

    private:
    typedef dlloader::DLLoader<geoflow::NodeRegister> DLLoader;
    std::unordered_map<std::string, std::unique_ptr<DLLoader>> dloaders_;

  };

}