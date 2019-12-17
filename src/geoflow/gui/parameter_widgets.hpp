#pragma once

#include <geoflow/parameters.hpp>
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "osdialog.hpp"

namespace geoflow {
	void draw_parameters(NodeHandle& node) {
		node->before_gui();
		for(auto& [name, param] : node->parameters) {
			bool changed = false;
			if( auto valptr = std::get_if<ParamInt>(&param) ) {
				if (valptr->visible()) {
					changed = ImGui::DragInt(valptr->get_label().c_str(), &valptr->get());
				}
			} else if( auto valptr = std::get_if<ParamFloat>(&param) ) {
				if (valptr->visible()) {
					changed = ImGui::DragFloat(valptr->get_label().c_str(), &valptr->get(), 0.1);
				}
			} else if( auto valptr = std::get_if<ParamDouble>(&param) ) {
				if (valptr->visible()) {
					changed = ImGui::DragScalar(valptr->get_label().c_str(), ImGuiDataType_Double, &valptr->get(), 0.1);
				}
			} else if( auto valptr = std::get_if<ParamBoundedFloat>(&param) ) {
				if (valptr->visible()) {
					changed = ImGui::SliderFloat(valptr->get_label().c_str(), &valptr->get(), valptr->min(), valptr->max());
				}
			} else if( auto valptr = std::get_if<ParamBoundedDouble>(&param) ) {
				if (valptr->visible()) {
					const double dmin = valptr->min(), dmax = valptr->max();
					changed = ImGui::SliderScalar(valptr->get_label().c_str(), ImGuiDataType_Double, &valptr->get(), &dmin, &dmax);
				}
			} else if( auto valptr = std::get_if<ParamFloatRange>(&param) ) {
				if (valptr->visible()) {
					changed = ImGui::DragFloatRange2(valptr->get_label().c_str(), &valptr->get().first, &valptr->get().second);
				}
			// } else if( auto valptr = std::get_if<ParamFloatRange>(&param) ) {
			// 	if (valptr->visible()) {
			// 		changed = ImGui::DragScalarN(valptr->get_label().c_str(), ImGuiDataType_Double, 2, &valptr->get().first, &valptr->get().second);
			// 	}
			} else if( auto valptr = std::get_if<ParamIntRange>(&param) ) {
				if (valptr->visible()) {
					changed = ImGui::DragIntRange2(valptr->get_label().c_str(), &valptr->get().first, &valptr->get().second);
				}
			} else if( auto valptr = std::get_if<ParamBoundedInt>(&param) ) {
				if (valptr->visible()) {
					changed = ImGui::SliderInt(valptr->get_label().c_str(), &valptr->get(), valptr->min(), valptr->max());
				}
			} else if( auto valptr = std::get_if<ParamString>(&param) ) {
				if (valptr->visible()) {
					changed = ImGui::InputText(valptr->get_label().c_str(), &valptr->get());
				}
			} else if( auto valptr = std::get_if<ParamPath>(&param) ) {
				if (valptr->visible()) {
					changed = ImGui::FilePicker(OSDIALOG_OPEN, valptr->get());
					ImGui::SameLine();
					ImGui::Text("%s",valptr->get_label().c_str());
				}
			} else if( auto valptr = std::get_if<ParamBool>(&param) ) {
				if (valptr->visible()) {
					changed = ImGui::Checkbox(name.c_str(), &valptr->get());
				}
			} else {
				ImGui::Text("%s", name.c_str());
			}
			if(changed) {
				node->on_change_parameter(name, param);
			}
		}
	};
}