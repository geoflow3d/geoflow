#pragma once

#include <geoflow/parameters.hpp>

namespace geoflow {
  void draw_global_parameter(Parameter* param);

  bool draw_parameter(Parameter* param);

	void draw_parameters(NodeHandle& node);
}