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
        ImGui::DragInt(valptr->get_label().c_str(), &valptr->get());
    } else if( auto* valptr = dynamic_cast<ParameterByValue<float>*>(param)) {
        ImGui::DragFloat(valptr->get_label().c_str(), &valptr->get(), 0.1);
    } else if( auto* valptr = dynamic_cast<ParameterByValue<std::string>*>(param)) {
        ImGui::InputText(valptr->get_label().c_str(), &valptr->get());
    } else if( auto* valptr = dynamic_cast<ParameterByValue<bool>*>(param)) {
        ImGui::Checkbox(valptr->get_label().c_str(), &valptr->get());
    }
  };

  bool draw_parameter(Parameter* param) {
    bool changed=false;
    if( auto* valptr = dynamic_cast<ParamInt*>(param)) {
        changed = ImGui::DragInt(valptr->get_label().c_str(), &valptr->get());
    } else if( auto* valptr = dynamic_cast<ParamFloat*>(param)) {
        changed = ImGui::DragFloat(valptr->get_label().c_str(), &valptr->get(), 0.1);
    } else if( auto* valptr = dynamic_cast<ParamDouble*>(param)) {
        changed = ImGui::DragScalar(valptr->get_label().c_str(), ImGuiDataType_Double, &valptr->get(), 0.1);
    } else if( auto* valptr = dynamic_cast<ParamBoundedFloat*>(param)) {
        changed = ImGui::SliderFloat(valptr->get_label().c_str(), &valptr->get(), valptr->min(), valptr->max());
    } else if( auto* valptr = dynamic_cast<ParamBoundedDouble*>(param)) {
        const double dmin = valptr->min(), dmax = valptr->max();
        changed = ImGui::SliderScalar(valptr->get_label().c_str(), ImGuiDataType_Double, &valptr->get(), &dmin, &dmax);
    } else if( auto* valptr = dynamic_cast<ParamFloatRange*>(param)) {
        changed = ImGui::DragFloatRange2(valptr->get_label().c_str(), &valptr->get().first, &valptr->get().second);
    // } else if( auto* valptr = dynamic_cast<ParamFloatRange*>(param)) {
    // 		changed = ImGui::DragScalarN(valptr->get_label().c_str(), ImGuiDataType_Double, 2, &valptr->get().first, &valptr->get().second);
    } else if( auto* valptr = dynamic_cast<ParamIntRange*>(param)) {
        changed = ImGui::DragIntRange2(valptr->get_label().c_str(), &valptr->get().first, &valptr->get().second);
    } else if( auto* valptr = dynamic_cast<ParamBoundedInt*>(param)) {
        changed = ImGui::SliderInt(valptr->get_label().c_str(), &valptr->get(), valptr->min(), valptr->max());
    } else if( auto* valptr = dynamic_cast<ParamString*>(param)) {
        changed = ImGui::InputText(valptr->get_label().c_str(), &valptr->get());
    } else if( auto* valptr = dynamic_cast<ParamPath*>(param)) {
        #ifdef GF_BUILD_GUI_FILE_DIALOGS
          changed = ImGui::FilePicker(OSDIALOG_OPEN, valptr->get());
          ImGui::SameLine();
          ImGui::Text("%s",valptr->get_label().c_str());
        #else
          changed = ImGui::InputText(valptr->get_label().c_str(), &valptr->get());
        #endif
    } else if( auto* valptr = dynamic_cast<ParamBool*>(param)) {
        changed = ImGui::Checkbox(valptr->get_label().c_str(), &valptr->get());
    } else if( auto* valptr = dynamic_cast<ParamStrMap*>(param)) {
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
      ImGui::Text("%s", valptr->get_label().c_str());
    }
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