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

	void GameObject::Destroy()
	{
		if (m_parent) {
			auto index = std::find(m_parent->m_children.begin(), m_parent->m_children.end(), this);
			if (index == m_parent->m_children.end()) {
				throw std::runtime_error("Attempting to destroy an invalid GameObject");
				return;
			}
			m_parent->m_children.erase(index);
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
		ImGui::Separator();
		ImGui::Text("ID: %X", GetId());
		ImGui::SameLine();
		//ImGui::Button("Clone"); // TODO implement this
		//ImGui::SameLine();
		static char rename[128];
		static bool nothing = false;
		if (ImGui::Button("Rename")) {
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
		ImGui::Separator();

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

		//  Show information of all components, and all physics components should wait
		for (Component* component : m_components)
		{
			ImGui::PushID(component);
			component->OnImGuiRender();
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

	void GameObject::OnSimulationBegin() {
		for (Component* component : m_components) {
			component->OnSimulationBegin();
		}
	}

	void GameObject::OnSimulationEnd() {
		for (Component* component : m_components) {
			component->OnSimulationEnd();
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

	std::pair<glm::vec3, glm::vec3> GameObject::GetBound() {
		if (GetComponent<MeshComponent>()) {
			return GetComponent<MeshComponent>()->GetBound();
		}
		else {
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
		LOG("{} collided with {}", GetName(), other->GetName());
	}

	void GameObject::OnEnter(GameObject* other) {
		LOG("{} and {} entered each other", GetName(), other->GetName());
	}

	void GameObject::OnOverlap(GameObject* other) {
		LOG("{} and {} is overlapping each other", GetName(), other->GetName());
	}

	void GameObject::OnLeave(GameObject* other) {
		LOG("{} and {} left each other", GetName(), other->GetName());
	}

	bool GameObject::IsOverlap(GameObject* other) {
		//	Check if object given is in m_collided.
		return (std::find(m_collided.begin(), m_collided.end(), other) != m_collided.end());
	}
}
