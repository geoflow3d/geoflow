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
#include "../geoflow.hpp"
#include "parameter_widgets.hpp"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#ifdef GF_BUILD_GUI_FILE_DIALOGS
  #include "osdialog.hpp"
#endif

namespace geoflow {
  void draw_global_parameter(Parameter* param) {
    if( auto* valptr = dynamic_cast<ParameterByValue<int>*>(param)) {
        ImGui::InputText("?", &valptr->get_help());
        ImGui::SameLine();
        ImGui::DragInt(valptr->get_label().c_str(), &valptr->get());
    } else if( auto* valptr = dynamic_cast<ParameterByValue<float>*>(param)) {
        ImGui::InputText("?", &valptr->get_help());
        ImGui::SameLine();
        ImGui::DragFloat(valptr->get_label().c_str(), &valptr->get(), 0.1);
    } else if( auto* valptr = dynamic_cast<ParameterByValue<std::string>*>(param)) {
        ImGui::InputText("?", &valptr->get_help());
        ImGui::SameLine();
        ImGui::InputText(valptr->get_label().c_str(), &valptr->get());
    } else if( auto* valptr = dynamic_cast<ParameterByValue<bool>*>(param)) {
        ImGui::InputText("?", &valptr->get_help());
        ImGui::SameLine();
        ImGui::Checkbox(valptr->get_label().c_str(), &valptr->get());
    }
  };
  static void HelpMarker(const char* desc)
  {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
  }

  bool draw_parameter(Parameter* param) {
    bool changed=false;
    if( ParamTypeInt == param->get_ptype() ) {
        auto* valptr = static_cast<ParamInt*>(param);
        changed = ImGui::DragInt(valptr->get_label().c_str(), &valptr->get());
    } else if( ParamTypeBool == param->get_ptype() ) {
        auto* valptr = static_cast<ParamBool*>(param);
        changed = ImGui::Checkbox(valptr->get_label().c_str(), &valptr->get());
    } else if( ParamTypeBoundedFloat == param->get_ptype() ) {
        auto* valptr = static_cast<ParamBoundedFloat*>(param);
        changed = ImGui::SliderFloat(valptr->get_label().c_str(), &valptr->get(), valptr->min(), valptr->max());
    } else if( ParamTypeBoundedDouble == param->get_ptype() ) {
        auto* valptr = static_cast<ParamBoundedDouble*>(param);
        const double dmin = valptr->min(), dmax = valptr->max();
        changed = ImGui::SliderScalar(valptr->get_label().c_str(), ImGuiDataType_Double, &valptr->get(), &dmin, &dmax);
    } else if( ParamTypeFloatRange == param->get_ptype() ) {
        auto* valptr = static_cast<ParamFloatRange*>(param);
        changed = ImGui::DragFloatRange2(valptr->get_label().c_str(), &valptr->get().first, &valptr->get().second);
    // } else if( ParamTypeFloatRange == param->get_ptype() ) {
    //     auto* valptr = static_cast<ParamFloatRange*>(param);
    // 		changed = ImGui::DragScalarN(valptr->get_label().c_str(), ImGuiDataType_Double, 2, &valptr->get().first, &valptr->get().second);
    } else if( ParamTypeIntRange == param->get_ptype() ) {
        auto* valptr = static_cast<ParamIntRange*>(param);
        changed = ImGui::DragIntRange2(valptr->get_label().c_str(), &valptr->get().first, &valptr->get().second);
    } else if( ParamTypeBoundedInt == param->get_ptype() ) {
        auto* valptr = static_cast<ParamBoundedInt*>(param);
        changed = ImGui::SliderInt(valptr->get_label().c_str(), &valptr->get(), valptr->min(), valptr->max());
    } else if( ParamTypeFloat == param->get_ptype() ) {
        auto* valptr = static_cast<ParamFloat*>(param);
        changed = ImGui::DragFloat(valptr->get_label().c_str(), &valptr->get(), 0.1);
    } else if( ParamTypeDouble == param->get_ptype() ) {
        auto* valptr = static_cast<ParamDouble*>(param);
        changed = ImGui::DragScalar(valptr->get_label().c_str(), ImGuiDataType_Double, &valptr->get(), 0.1);
    } else if( ParamTypePath == param->get_ptype() ) {
        auto* valptr = static_cast<ParamPath*>(param);
        #ifdef GF_BUILD_GUI_FILE_DIALOGS
          changed = ImGui::FilePicker(OSDIALOG_OPEN, valptr->get());
          ImGui::SameLine();
          ImGui::Text("%s",valptr->get_label().c_str());
        #else
          changed = ImGui::InputText(valptr->get_label().c_str(), &valptr->get());
        #endif
    } else if( ParamTypeString == param->get_ptype() ) {
        auto* valptr = static_cast<ParamString*>(param);
        changed = ImGui::InputText(valptr->get_label().c_str(), &valptr->get());
    } else if( ParamTypeText == param->get_ptype() ) {
        auto* valptr = static_cast<ParamString*>(param);
        changed = ImGui::InputTextMultiline(valptr->get_label().c_str(), &valptr->get(), ImVec2(ImGui::GetTextLineHeight() * 32, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput);
    } else if( param->is_type(typeid(StrMap))) {
        auto* valptr = static_cast<ParamStrMap*>(param);;
        if (ImGui::TreeNode(valptr->get_label().c_str())) {
          auto& mapvalues = valptr->get();
          for (auto it=mapvalues.begin(); it!=mapvalues.end(); ) {
            ImGui::InputTextWithHint(it->first.c_str(), "Value", &(it->second));
            ImGui::SameLine();
            ImGui::PushID(it->first.c_str());
            if(ImGui::Button("Remove")) {
              mapvalues.erase(it++);
            } else {
              ++it;
            }
            ImGui::PopID();
          }
          static ImGuiComboFlags flags = 0;
          if (ImGui::BeginCombo("Create item", "select..", flags)) // The second parameter is the label previewed before opening the combo.
          {
            for (auto& key_option : valptr->key_options_)
            {
              if (ImGui::Selectable(key_option.c_str(), false)) {
                mapvalues.insert({key_option, ""});
              }
            }
            ImGui::EndCombo();
          }
          ImGui::TreePop();
        }
    } else {
      ImGui::Text("%s", param->get_label().c_str());
    }
    ImGui::SameLine();
    HelpMarker(param->get_help().c_str());
    return changed;
  };

	void draw_parameters(NodeHandle& node) {
		node->before_gui();
		for(auto&& [name, param] : node->parameters) {
      ImGui::PushID(param.get());
      if(ImGui::Button("G")) {
        ImGui::OpenPopup("GlobalSelector"); 
      }
      ImGui::SameLine();

      if (ImGui::BeginPopup("GlobalSelector"))
      {
        for (auto& [name, gparam] : node->get_manager().global_flowchart_params)
        {
          if(gparam->is_type_compatible(*param)) {
            if (ImGui::MenuItem(gparam->get_label().c_str())) {
              param->set_master(gparam);
            }
          }
        }
        if (ImGui::MenuItem("Clear")) {
          param->clear_master();
        }
        ImGui::EndPopup();
      }

      if(param->has_master())
        ImGui::Text("%s = %s", param->get_label().c_str(), param->get_master().lock()->get_label().c_str());
      else {
        bool changed = draw_parameter(param.get());
        if(changed) {
          node->on_change_parameter(name, *param);
        }
      }
      ImGui::PopID();
		}
	};
}