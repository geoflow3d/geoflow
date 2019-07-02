#pragma once

#include <geoflow/parameters.hpp>
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "osdialog.hpp"

namespace geoflow {
	void draw_parameters(ParameterMap& parameters) {
		for(auto& [name, param] : parameters) {
			if( auto valptr = std::get_if<ParamInt>(&param) ) {
				ImGui::InputInt(name.c_str(), &valptr->get());
			} else if( auto valptr = std::get_if<ParamFloat>(&param) ) {
				ImGui::InputFloat(name.c_str(), &valptr->get());
			} else if( auto valptr = std::get_if<ParamFloatRange>(&param) ) {
				ImGui::SliderFloat(name.c_str(), &valptr->get(), valptr->min(), valptr->max());
			} else if( auto valptr = std::get_if<ParamIntRange>(&param) ) {
				ImGui::SliderInt(name.c_str(), &valptr->get(), valptr->min(), valptr->max());
			} else if( auto valptr = std::get_if<ParamPath>(&param) ) {
        ImGui::FilePicker(OSDIALOG_OPEN, valptr->get());
				ImGui::SameLine();
				ImGui::Text("%s",name.c_str());
			} else if( auto valptr = std::get_if<ParamBool>(&param) ) {
				ImGui::Checkbox(name.c_str(), &valptr->get());
			} else {
				ImGui::Text("%s", name.c_str());
			}
		}
	};
}