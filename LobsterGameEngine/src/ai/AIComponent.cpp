#include "pch.h"
#include "AIComponent.h"
#include "objects/GameObject.h"

namespace Lobster {

	AIComponent::AIComponent() : Component(AI_COMPONENT) {
		nodeEditor = new ImGuiNodeGraphEditor();
	}

	AIComponent::~AIComponent() {
		delete nodeEditor;
	}

	void AIComponent::SetGameObjectName(std::string name) {
		nodeEditor->SetGameObjectName(name);
	}

	void AIComponent::OnImGuiRender() {
		if (ImGui::CollapsingHeader("AI Component", &m_show, ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("TODO...");
			if (ImGui::Button("Node Graph Editor")) {
				m_showNodeEditor = true;
			}
			if (m_showNodeEditor) {
				nodeEditor->Show(&m_showNodeEditor);
			}			
		}
	}

}