#pragma once

#include <string>
#include <optional>
#include "osdialog.h"

std::optional<std::string> osdialog_file(osdialog_file_action action, const char* filename=NULL, const char* filters=NULL);

namespace ImGui {
  bool FilePicker(osdialog_file_action action, std::string& picked_file, const char* filters=NULL);
}