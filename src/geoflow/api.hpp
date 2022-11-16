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

#include <geoflow/geoflow.hpp>
#include <geoflow/plugin_manager.hpp>

namespace geoflow {

  class GeoflowRunner {
    bool verbose = false;
    PluginManager plugin_manager;
    NodeRegisterMap node_registers;
    NodeManager flowchart;
    
    public:
    GeoflowRunner(std::string flowchart_json, bool verbose);
    ~GeoflowRunner();
    void run();

  };

}