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
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <optional>
#include <cstdlib>
#include <cstring>
#include "osdialog.hpp"

std::optional<std::string> osdialog_file(osdialog_file_action action, const char* filename, const char* filters) {
  osdialog_filters* filters_c=nullptr;
  if (filters)
    filters_c = osdialog_filters_parse(filters);
  
  std::optional<std::string> result;
  char *filepath_c = osdialog_file(action, nullptr, filename, filters_c);
  if (filepath_c) {
    result = std::string(filepath_c);
    std::free(filepath_c);
    osdialog_filters_free(filters_c);
  }
  return result;
};

namespace ImGui {

  bool FilePicker(osdialog_file_action action, std::string& picked_file, const char* filters) {
    bool changed = false;
    if (ImGui::Button("Open")) {
      auto result = osdialog_file(action, nullptr, filters);
      if (result.has_value()) {
        picked_file = result.value();
        changed = true;
      }
    }
    ImGui::SameLine();
    ImGui::InputText("", &picked_file);
    return changed;
  }
}