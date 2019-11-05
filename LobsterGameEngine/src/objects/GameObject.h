#pragma once

#include <typeinfo>
#include "Transform.h"
#include "components/ComponentCollection.h"
#include "system/filesystem.h"

#include "graphics/Scene.h"
#include "physics/PhysicsComponentCollection.h"

namespace Lobster
{
	//	This class is the building block of a scene.
	//	This acts as a node with a list of components attached, and have a spatial relationship with the world origin.
    class GameObject
    {
    public:
		//	The world / model matrix of the game object
        Transform transform;

    private:
		static std::hash<uintptr_t> hashFunc;
		unsigned long long m_id;
        std::string m_name;
        std::vector<Component*> m_components;
		//	Shortcut to access the vector of colliders.
		std::vector<ColliderComponent*> m_colliders;
		//	Shortcut to access the mesh component.
		MeshComponent* m_mesh = nullptr;
		//	Shortcut to access the physics component.
		PhysicsComponent* m_physics = nullptr;
		//	Boolean to indicate whether we are changing object in previous frame.
		bool b_isChanging = false;
		//	Transform object to store previous state of game object prior to change.
		Transform m_transPrev;

		//	Helper function to get the bounding box for an object.
		inline std::pair<glm::vec3, glm::vec3> GetBound() const {
			return (m_mesh ? m_mesh->GetBound() : std::pair<glm::vec3, glm::vec3>({ glm::vec3(-0.5, -0.5, -0.5), glm::vec3(0.5, 0.5, 0.5) }));
		}

		template<typename T, typename ...Args> Component* CreateComponent(Args&&... args);

    public:
        GameObject(const char* name);
        ~GameObject();
        void OnUpdate(double deltaTime);
		//	To update ImGui components that describes this game object's attributes
		virtual void OnImGuiRender(Scene* scene);
        template<typename T, typename ...Args> GameObject* AddComponent(Args&&... args);
        template<typename T> T* GetComponent();
		inline PhysicsComponent* GetPhysicsComponent() const { return m_physics; }
		//	Colliders are undefined without a physics component. Return only if we have physics component defined.
		inline std::vector<ColliderComponent*> GetColliders() const { return (m_physics ? m_colliders : std::vector<ColliderComponent*>()); }
		inline unsigned long long GetId() { return m_id; }
        inline std::string GetName() const { return m_name; }
		//	RemoveComponent removes the component in vector and deletes comp afterwards.
		void RemoveComponent(Component* comp);
    };
    
    //=========================================
    //  Template Implementations
    //=========================================
	template<typename T, typename ...Args>
	inline Component * GameObject::CreateComponent(Args&&... args)
	{
		return new T(std::forward<Args>(args)...);
	}

	template<typename T, typename ...Args>
	inline GameObject * GameObject::AddComponent(Args&&... args)
	{
		//  TODO:
		//  One game object can only have one mesh component
		//	Sunny: Function returns this directly when we add the second one now. Need discussion on deletion tho
		//	Also discuss about whether we need to keep m_mesh.

		//	3 conditions of not creating a new component.
		//	1. If we are creating MeshComponent and one already exists;
		//	2. If we are creating PhysicsComponent, and no MeshComponent found;
		//	3. If we are creating PhysicsComponent and one already exists.
		if (typeid(T) == typeid(MeshComponent) && m_mesh || std::is_base_of<PhysicsComponent, T>::value && (!m_mesh || m_physics)) return this;

		T* newComponent = new T(std::forward<Args>(args)...);
		newComponent->SetOwner(this);
		newComponent->SetOwnerTransform(&this->transform);

		//	If it is the mesh, set m_mesh.
		//	Else, if it is physics component, set m_physics.
		//	Else, if it is collider component, append to m_colliders.
		if (typeid(T) == typeid(MeshComponent)) {
			m_mesh = dynamic_cast<MeshComponent*>(newComponent);
		} else if (dynamic_cast<PhysicsComponent*>(newComponent)) {
			m_physics = dynamic_cast<PhysicsComponent*>(newComponent);
		} else if (dynamic_cast<ColliderComponent*>(newComponent)) {
			m_colliders.push_back(dynamic_cast<ColliderComponent*>(newComponent));
		}

		m_components.push_back(newComponent);
		return this;
	}
    
    template<typename T>
    inline T* GameObject::GetComponent()
    {
        for (unsigned int i = 0; i < m_components.size(); ++i)
        {
            T* found = dynamic_cast<T*>(m_components[i]);
            if (found)
            {
                return found;
            }
        }
        return nullptr;
    }

    
}
