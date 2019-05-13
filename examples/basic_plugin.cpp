#include "basic_nodes.hpp"
#include "imgui_internal.h"

#if defined(__linux__) || defined(__APPLE__)
extern "C"
{
	geoflow::NodeRegister *allocator()
	{
    auto R = new geoflow::NodeRegister("Arithmetic");
    R->register_node<geoflow::nodes::arithmetic::AdderNode>("Adder");
    R->register_node<geoflow::nodes::arithmetic::NumberNode>("Number");
    R->register_node<geoflow::nodes::arithmetic::NumberNodeI>("NumberI");
		return R;
	}

	void deleter(geoflow::NodeRegister *ptr)
	{
		delete ptr;
	}

	void SetImGuiContext(ImGuiContext* ctx) {
		ImGui::SetCurrentContext(ctx);
	}
}
#endif

// #ifdef WIN32
// extern "C"
// {
// 	__declspec (dllexport) Bespin *allocator()
// 	{
// 		return new Bespin();
// 	}

// 	__declspec (dllexport) void deleter(Bespin *ptr)
// 	{
// 		delete ptr;
// 	}
// }
// #endif