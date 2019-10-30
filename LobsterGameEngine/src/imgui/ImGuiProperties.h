#pragma once
#include <string>
#include "ImGuiComponent.h"
#include "layer/EditorLayer.h"

#include "Application.h"
#include "objects/GameObject.h"
#include "imgui/ImGuiConsole.h"

namespace Lobster
{

	class ImGuiProperties : public ImGuiComponent
	{
	private:
		Scene* scene;	
	public:
		static GameObject* selectedObj;
		ImGuiProperties(Scene* scene) : ImGuiComponent(), scene(scene) {}
		virtual void Show(bool* p_open) override
		{
			if (!ImGui::Begin("Properties", nullptr))
			{
				ImGui::End();
				return;
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));

			static int selected = -1;
			// The Region for displaying a list of game components
			ImGui::BeginChild("Components", ImVec2(0, 150), true);
			{
				int i = 0;
				for (GameObject* obj : scene->GetGameObjects()) {
					char buf[64];
                    // sprintf(buf, "%x", obj->GetId());
					strcpy(buf, obj->GetName().c_str());
					ImGui::PushID(obj->GetId());
					if (ImGui::Selectable(buf, selected == i)) {						
						selectedObj = obj;
						selected = i;						
					}				
					ImGui::PopID();
					i++;
				}
			}
			ImGui::EndChild();			

			ImGui::SameLine();
			ImGui::Separator();

			if (selectedObj != nullptr)
			{
				selectedObj->OnImGuiRender(scene);
			}

			//if (ImGui::Button("Log Something")) {
			//	ImGuiConsole::log.AddLog("This is called by invoking ImGuiConsole::log.AddLog() anywhere in the project \n");
			//}

			ImGui::PopStyleVar();
			ImGui::End();
		}
	};
}
