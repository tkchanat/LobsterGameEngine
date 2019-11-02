#include "pch.h"
#include "GameObject.h"
#include "imgui/ImGuiProperties.h"
#include "system/UndoSystem.h"

namespace Lobster
{

	std::hash<uintptr_t> GameObject::hashFunc;
    
    GameObject::GameObject(const char* name) :
        m_name(name)
    {
		m_id = hashFunc((unsigned long long) this);
    }
    
    GameObject::~GameObject()
    {
		//  Delete all enabled components
		for (Component* component : m_components)
		{
			if(component)	delete component;
			component = nullptr;
		}
    }
    
    void GameObject::OnUpdate(double deltaTime)
    {
		//  First, update transform
		transform.UpdateMatrix();

        //  Update all enabled components
        for(Component* component : m_components)
        {
            if(component->IsEnabled())
            {
                component->OnUpdate(deltaTime);
            }
        }
    }

	// This function is called when the object is selected in ImGui::Properties
	// It will create the ImGui widgets according to its property
	void GameObject::OnImGuiRender(Scene* scene) {
		ImGui::Text("Object: %s", m_name.c_str());
		ImGui::Text("ID: %X", GetId());
		if (ImGui::Button("Remove")) {
			scene->RemoveGameObject(m_name);
			ImGuiProperties::selectedObj = nullptr;
		}
		ImGui::SameLine();
		ImGui::Button("Clone"); // TODO implement this
		ImGui::SameLine();
		static char rename[128];
		static bool nothing = false;
		if (ImGui::Button("Rename")) {
			sprintf(rename, m_name.c_str());
			ImGui::OpenPopup("Rename Game Object");
		}

		ImGui::SetNextWindowSize(ImVec2(240, 120));
		if (ImGui::BeginPopupModal("Rename Game Object")) {
			ImGui::Text("Input a new name:");
			ImGui::InputText("", rename, IM_ARRAYSIZE(rename), ImGuiInputTextFlags_CallbackAlways, \
				[](ImGuiInputTextCallbackData* data) -> int {
				GameObject* obj = (GameObject*)data->UserData;
				strcpy(rename, data->Buf);
				return 0;
			}, this);
			if (nothing) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 0, 0, 255));
				ImGui::Text("The object name cannot be empty.");
				ImGui::PopStyleColor();
			}
			ImGui::SetCursorPos(ImVec2(30, 90));
			if (ImGui::Button(" OK ")) {
				if (!strlen(rename)) {
					nothing = true;
				}
				else {
					m_name = rename;
					rename[0] = '\0';
					nothing = false;
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::SameLine();
			ImGui::SetCursorPosX(90);
			if (ImGui::Button("Cancel")) {
				nothing = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		//	Check if transform is active, ie: we are trying to change the value of transform.
		bool isChanging = false;
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::DragFloat3("Position", glm::value_ptr(transform.WorldPosition), 0.05f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			isChanging = isChanging || ImGui::IsItemActive();
			ImGui::DragFloat3("Rotation", glm::value_ptr(transform.LocalEulerAngles), 1.0f, -360.0f, 360.0f);
			isChanging = isChanging || ImGui::IsItemActive();
			ImGui::DragFloat3("Scale", glm::value_ptr(transform.LocalScale), 0.05f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			isChanging = isChanging || ImGui::IsItemActive();
		}

		//	Keep track of transform before we start transforming.
		if (!b_isChanging) {
			m_transPrev = transform;
		}

		//	Now, keep track of whether we are changing, and send undo event after change.
		if (!b_isChanging && isChanging) {
			b_isChanging = true;
		} else if (b_isChanging && !isChanging) {
			b_isChanging = false;

			//	Only send undo event if transform changed.
			if (m_transPrev.GetMatrix() != transform.GetMatrix()) {
				UndoSystem::GetInstance()->Push(new TransformCommand(this, m_transPrev, transform));
			}
		}

		//	General options for Physics.
		if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Combo("Body Type", &m_physicsType, PhysicsBody::PhysicsBodyTypes, 2);


			PhysicsBody* physics = GetPhysicsBody();

			//	Change Rigid Body type if needed
			if (ImGui::IsItemActive()) {
				if (m_physicsType == 0 && dynamic_cast<Rigidbody*>(physics)) {
					//	Case 1 - user changed from non-rigid to rigid.
					delete physics;
					physics = new Rigidbody(m_mesh->GetBound());
					m_components[m_physicsIndex] = physics;
				} else {
					//	TODO: implement else if for changing from rigid to non-rigid.
				}
			}

			physics->SetEnabled();
		}

		//  Show information of all components
		for (Component* component : m_components)
		{
			component->OnImGuiRender();
		}
	}
}
