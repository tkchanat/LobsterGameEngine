#pragma once
#include "Application.h"
#include "ImGuiComponent.h"
#include "layer/EditorLayer.h"
#include "graphics/Scene.h"
#include "objects/GameObject.h"
#include "system/UndoSystem.h"
#include "physics/Rigidbody.h"

namespace Lobster 
{

	class ImGuiHierarchy : public ImGuiComponent 
	{
	public:
		void ShowPopupContext(GameObject* gameObject) {
			if (ImGui::BeginPopupContextItem()) {
				EditorLayer::s_selectedGameObject = gameObject;
				if (ImGui::MenuItem("Add Child", "")) {
					int cnt = gameObject->GetChildrenCount() + 1;
					char name[16]; sprintf(name, "Child %d", cnt);
					GameObject* child = new GameObject(name);
					PhysicsComponent* phys = new Rigidbody();
					phys->SetEnabled(false);
					gameObject->AddChild(child->AddComponent(phys));
					UndoSystem::GetInstance()->Push(new CreateChildCommand(child, gameObject));
				}
				// TODO: Clone GameObject
				if (ImGui::MenuItem("Clone", "", false)) {
					ImGui::CloseCurrentPopup();
				}
				if (!gameObject->GetComponent<CameraComponent>()) {
					if (ImGui::MenuItem("Destroy", "", false)) {
						if (EditorLayer::s_selectedGameObject->GetParent()) {
							EditorLayer::s_selectedGameObject->Destroy();
							UndoSystem::GetInstance()->Push(new DestroyChildCommand(EditorLayer::s_selectedGameObject, EditorLayer::s_selectedGameObject->GetParent()));
						}
						else {
							GetScene()->RemoveGameObject(EditorLayer::s_selectedGameObject);
							UndoSystem::GetInstance()->Push(new DestroyObjectCommand(EditorLayer::s_selectedGameObject, GetScene()));
						}
						EditorLayer::s_selectedGameObject->ToggleVirtualDelete();
						EditorLayer::s_selectedGameObject = nullptr;
					}
				}
				if (ImGui::MenuItem("Cancel", "", false)) {
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
		void ShowChildren(const std::vector<GameObject*>& children) {
			for (int i = 0; i < children.size(); ++i) {
				GameObject* gameObject = children[i];

				ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth;
				if (EditorLayer::s_selectedGameObject == gameObject) node_flags |= ImGuiTreeNodeFlags_Selected;

				if (gameObject->GetChildrenCount() > 0)
				{
					// With Children
					bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, gameObject->GetName().c_str());
					ShowPopupContext(gameObject);
					if (ImGui::IsItemClicked())
						EditorLayer::s_selectedGameObject = gameObject;
					if (node_open)
					{
						ShowChildren(gameObject->GetChildren());
						ImGui::TreePop();
					}
				}
				else
				{
					// No Children (Tree Leaves)
					node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
					ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, gameObject->GetName().c_str());
					ShowPopupContext(gameObject);
					if (ImGui::IsItemClicked())
						EditorLayer::s_selectedGameObject = gameObject;
				}
			}
		}
		virtual void Show(bool* p_open) override
		{
			if (ImGui::Begin("Hierarchy", nullptr))
			{
				// The Region for displaying a list of game components
				ShowChildren(GetScene()->m_gameObjects);
			}
			ImGui::End();
		}
	};

}
