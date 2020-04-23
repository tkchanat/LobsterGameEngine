#include "pch.h"
#include "GameObject.h"
#include "imgui/ImGuiProperties.h"
#include "system/UndoSystem.h"

namespace Lobster
{

	std::hash<uintptr_t> GameObject::hashFunc;
    
    GameObject::GameObject(const char* name) :
        m_name(name),
		m_parent(nullptr)
    {
		m_id = hashFunc((unsigned long long) this);
    }
    
    GameObject::~GameObject()
    {
		// Delete all enabled components
		for (Component* component : m_components) {
			delete component;
			component = nullptr;
		}
		// Delete all children
		for (GameObject* child : m_children) {
			delete child;
			child = nullptr;
		}
    }

	void GameObject::ToggleVirtualDelete() {
		b_isVirtuallyDeleted = !b_isVirtuallyDeleted;
		
		//	First call the function for its child
		for (GameObject* child : m_children) child->ToggleVirtualDelete();

		//	Then call the function for its components.
		for (Component* comp : m_components) {
			if (b_isVirtuallyDeleted) {
				comp->VirtualDelete();
			} else {
				comp->VirtualCreate();
			}
		}
	}

	void GameObject::Destroy()
	{
		if (m_parent) {
			m_parent->RemoveChild(this);
        }
		else {
			// TODO search in first layer of scene
		}
	}

	void GameObject::OnBegin() {
		// initialize transform
		transform.UpdateMatrix();
		// initialize all enabled components
		for (Component* component : m_components) {
			if (component->IsEnabled()) {
				component->OnBegin();
			}
		}
		// initialize all children
		for (GameObject* child : m_children) {
			child->OnBegin();
		}
	}

    void GameObject::OnUpdate(double deltaTime)
    {
		//  First, update transform
		transform.UpdateMatrix();

        //  Update all enabled components
        for(Component* component : m_components)
        {
            if(component->IsEnabled() || dynamic_cast<PhysicsComponent*>(component))
            {
                component->OnUpdate(deltaTime);
            }
        }

		// Update all children
		for (GameObject* child : m_children) {
			child->OnUpdate(deltaTime);
		}
    }

	// This function is called when the object is selected in ImGui::Properties
	// It will create the ImGui widgets according to its property
	void GameObject::OnImGuiRender() {
		static bool test_bool;
		ImGui::Checkbox("", &test_bool);
		ImGui::SameLine();
		ImGui::Text(m_name.c_str());
		static char rename[128];
		static bool nothing = false;
		ImGui::SameLine();
		if (ImGui::Button("Rename")) {
			ImGui::OpenPopup("Rename Game Object");
		}
		ImGui::Separator();
		ImGui::Text("ID: %X (%ld)", GetId(), GetId());
		//ImGui::Button("Clone"); // TODO implement this
		//ImGui::SameLine();

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

		//	TODO: ask for a component name perhaps?
		//	We will always keep one physics component and one collider.
		//	They shall be used for selecting objects on scene (by ray-casting)
		//	Therefore, "Add Rigidbody" here will just enable the object instead of creating it;
		//	"Add Collider" here will do the same for the first collider.

		// Note: this part will be moved to ImGuiProperties::Show()
		PhysicsComponent* physics = GetComponent<PhysicsComponent>();
		if (!physics || !physics->IsEnabled()) {
			if (ImGui::Button("Add Rigidbody")) {
				physics->SetEnabled(true);

			}
		} else {
			if (ImGui::Button("Add Collider")) {
				BoxCollider* box = new BoxCollider(physics);
				box->SetOwner(this);
				box->SetOwnerTransform(&transform);
				physics->AddCollider(box);
			}
		}

		// Transform
		transform.OnImGuiRender(this);

		//  Show information of all components, and all physics components should wait
		for (Component* component : m_components)
		{
			ImGui::PushID(component);
			component->OnImGuiRender();

			// Remove the component upon the cross button click
			if (!component->IsShowing()) {
				component->SetShowing(true);
				RemoveComponent(component);
				UndoSystem::GetInstance()->Push(new DestroyComponentCommand(component, this));
			}
			ImGui::Separator();
			ImGui::PopID();
		}
	}

	void GameObject::Serialize(cereal::JSONOutputArchive& oarchive)
	{
		//LOG("Serializing GameObject {}", m_name);
		oarchive(*this);
	}

	void GameObject::Deserialize(cereal::JSONInputArchive& iarchive)
	{
		//LOG("Deserializing GameObject {}", m_name);
		try {
			iarchive(*this);
		}
		catch (std::exception e) {
			LOG("Deserializing GameObject {} failed. Reason: {}", m_name, e.what());
		}
	}

	void GameObject::OnEnd() {
		for (Component* component : m_components) {
			component->OnEnd();
		}
	}

	GameObject * GameObject::AddComponent(Component * component)
	{
		//  TODO:
		//  One game object can only have one mesh component
		//	Sunny: Function returns this directly when we add the second one now. Need discussion on deletion tho
		//	Also discuss about whether we need to keep m_mesh.

		//	3 conditions of not creating a new component.
		//	1. If we are creating MeshComponent and one already exists;
		//	2. If we are creating PhysicsComponent, and no MeshComponent found;
		//	3. If we are creating PhysicsComponent and one already exists.
		//if (dynamic_cast<MeshComponent*>(component) && m_mesh || dynamic_cast<PhysicsComponent*>(component) && (!m_mesh || m_physics)) 
			//return this;

		// Key:
		// No duplicates for the following component types:
		// MeshComponent, PhysicsComponent
		if (dynamic_cast<MeshComponent*>(component) && GetComponent<MeshComponent>())
			return this;
		if (dynamic_cast<PhysicsComponent*>(component) && GetComponent<PhysicsComponent>())
			return this;

		component->SetOwner(this);
		component->SetOwnerTransform(&transform);

		m_components.push_back(component);		
		component->OnAttach();
		return this;
	}

	GameObject * GameObject::AddChild(GameObject * child)
	{
		child->m_parent = this;
		m_children.push_back(child);
		return this;
	}

	void GameObject::RemoveChild(GameObject* child) {
		m_children.erase(std::remove(m_children.begin(), m_children.end(), child));
	}

	void GameObject::RemoveChildByName(std::string& name) {
		m_children.erase(std::remove_if(m_children.begin(), m_children.end(), [&](GameObject* c) -> bool {
			return (c->GetName() == name);
		}), m_children.end());
	}

	std::pair<glm::vec3, glm::vec3> GameObject::GetBound() {
		if (GetComponent<MeshComponent>()) {
			return GetComponent<MeshComponent>()->GetBound();
		}
		else {
			// the size ImGuiGizmo icon
			return { glm::vec3(-1, -1, -1), glm::vec3(1, 1, 1) };
		}
	}

	void GameObject::RemoveComponent(Component* comp) {
		int i = 0;
		for (Component* component : m_components) {
			if (comp == component) m_components.erase(m_components.begin() + i);
			i++;
		}

		//delete comp;
	}


	bool GameObject::Intersects(GameObject* other) {
		PhysicsComponent* physics = GetComponent<PhysicsComponent>();
		PhysicsComponent* otherPhysics = other->GetComponent<PhysicsComponent>();

		return physics && physics->IsEnabled() && otherPhysics && otherPhysics->IsEnabled() && physics->Intersects(otherPhysics);
	}

	void GameObject::OnCollide(GameObject* other) {
		//LOG("{} collided with {}", GetName(), other->GetName());
	}

	void GameObject::OnEnter(GameObject* other) {
		//LOG("{} and {} entered each other", GetName(), other->GetName());
	}

	void GameObject::OnOverlap(GameObject* other) {
		//LOG("{} and {} is overlapping each other", GetName(), other->GetName());
	}

	void GameObject::OnLeave(GameObject* other) {
		//LOG("{} and {} left each other", GetName(), other->GetName());
	}

	bool GameObject::IsOverlap(GameObject* other) {
		//	Check if object given is in m_collided.
		return (std::find(m_collided.begin(), m_collided.end(), other) != m_collided.end());
	}
}
