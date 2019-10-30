#pragma once
#include "ImGuiComponent.h"

namespace Lobster
{

	class ImGuiAbout : public ImGuiComponent
	{
	public:
		virtual void Show(bool* p_open) override
		{
			if (!ImGui::Begin("About Lobster Engine", p_open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse))
			{
				ImGui::End();
				return;
			}
			
			ImVec4 titleColor = ImVec4(0.8, 0.3, 0.1, 1.0);
			ImGui::TextColored(titleColor, "Lobster Engine");
			std::string platform = "";
#ifdef _WIN32
			platform = "Windows 32-bit";
#endif
#ifdef _WIN64
			platform = "Windows 64-bit";
#endif
#ifdef __linux__
			platform = "Linux 64-bit"
#endif
#ifdef __APPLE__
			platform = "MacOSX 64-bit";
#endif
			ImGui::TextColored(titleColor, "Version 0.0 (%s)", platform.c_str());
			ImGui::Spacing();

			ImGui::Text("Team: ");
			ImGui::Text(" * Andy, Chan Tsz Kin (tkchanat)");
			ImGui::Text(" * Argen, Yan Chiu Wai (cwyan)");
			ImGui::Text(" * Sunny, Tsang Siu Long (sltsangab)");
			ImGui::Text(" * Yuki, Lee Yuk Shan (ysleead)");
			ImGui::Spacing();

			ImGui::Text("Our GUI service is provided by Dear ImGui %s", ImGui::GetVersion());

			ImGui::End();
		}
	};

}
