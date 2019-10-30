#pragma once
#include "ImGuiComponent.h"

namespace Lobster
{

	class ImGuiDemoWindow : public ImGuiComponent
	{
	public:
		virtual void Show(bool* p_open) override
		{
			ImGui::ShowDemoWindow(p_open);
		}
	};

}
