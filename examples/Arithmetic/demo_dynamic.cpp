// This file is part of Geoflow
// Copyright (C) 2018-2019  Ravi Peters, 3D geoinformation TU Delft

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
#include "DLLoader.h"
#include <iostream>

using namespace geoflow;

static const std::string path = "/Users/ravi/git/geoflow/debug/examples/libbasic_plugin.dylib";

int main(void) {
	dlloader::DLLoader<geoflow::NodeRegister> dlloader(path);

	std::cout << "Loading " << path << std::endl;
	dlloader.DLOpenLib();

	std::shared_ptr<geoflow::NodeRegister> R = dlloader.DLGetInstance();

  NodeManager N;
  auto adder = N.create_node(*R, "Adder");
  auto number = N.create_node(*R, "Number");
  auto adder2 = N.create_node(*R, "Adder");
  auto number2 = N.create_node(*R, "Number");
  
  connect(number->output("result"), adder->input("in1"));
  connect(number, adder, "result", "in2");
  connect(adder, adder2, "result", "in1");
  connect(adder, adder2, "result", "in2");

  number->set_param("number_value", (int) 5);

  N.dump_json("out.json");

  bool success = N.run(number);
  if (success){
    std::cout << "Result: " << adder2->output("result").get<float>() << "\n";
  } else {
    std::cout << "No result\n";
  }

  std::cout << "Unloading " << path << std::endl;
	dlloader.DLCloseLib();
}