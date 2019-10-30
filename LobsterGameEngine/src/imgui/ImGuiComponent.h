#pragma once
#include <imgui.h>

namespace Lobster
{

	// Just an interface class for all custom imgui components
	class ImGuiComponent
	{
	public:
		virtual void Show(bool* p_open) = 0;
	};

}
