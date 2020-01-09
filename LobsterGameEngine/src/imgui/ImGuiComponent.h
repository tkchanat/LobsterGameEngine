#pragma once
#include "Application.h"
#include <imgui.h>

namespace Lobster
{

	// Just an interface class for all custom imgui components
	class ImGuiComponent
	{
	public:
		inline Scene* GetScene() { return Application::GetInstance()->m_scene; }
		virtual void Show(bool* p_open) = 0;
	};

}
