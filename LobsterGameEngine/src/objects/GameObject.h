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
		//	Shortcut to access the vector of physics components.
		std::vector<PhysicsComponent*> m_physicsComp;
		//	Shortcut to access to the mesh component.
		MeshComponent* m_mesh = false;
		//	Boolean to indicate whether we are changing object in previous frame.
		bool b_isChanging = false;
		//	Transform object to store previous state of game object prior to change.
		Transform m_transPrev;

		//	Index of physics component.
		int m_physicsIndex = -1;
		//	Integer defining physics for this game object.
		int m_physicsType = 0;

		//	Helper function to get the bounding box for an object.
		inline std::vector<glm::vec3> GetBound() const {
			return (m_mesh ? m_mesh->GetBound() : std::vector<glm::vec3>({glm::vec3(-0.5, -0.5, -0.5), glm::vec3(0.5, 0.5, 0.5)}));
		}

    public:
        GameObject(const char* name);
        ~GameObject();
        void OnUpdate(double deltaTime);
		//	To update ImGui components that describes this game object's attributes
		virtual void OnImGuiRender(Scene* scene);
        template<typename T, typename ...Args> GameObject* AddComponent(Args&&... args);
        template<typename T> T* GetComponent();
		inline std::vector<PhysicsComponent*> GetPhysicsComponent() const { return m_physicsComp; }
		inline unsigned long long GetId() { return m_id; }
        inline std::string GetName() const { return m_name; }
		//	RemoveComponent removes the component in vector and deletes comp afterwards.
		void RemoveComponent(Component* comp);
    };
    
    //=========================================
    //  Template Implementations
    //=========================================
	template<typename T, typename ...Args>
	inline GameObject * GameObject::AddComponent(Args&&... args)
	{
		//  TODO:
		//  One game object can only have one mesh component
		//	Sunny: Function returns this directly when we add the second one now. Need discussion on deletion tho
		//	Also discuss about whether we need to keep m_mesh.

		//	Determines if we want to add a mesh component, return directly if we add more than one.
		if (typeid(T) == typeid(MeshComponent) && m_mesh) return this;

		T* newComponent = new T(std::forward<Args>(args)...);
		newComponent->SetOwner(this);
		newComponent->SetOwnerTransform(&this->transform);
		m_components.push_back(newComponent);

		//	If it is the mesh, set m_mesh.
		//	Else, if it is physics component, send it to physics component vector too.
		if (typeid(T) == typeid(MeshComponent)) {
			m_mesh = dynamic_cast<MeshComponent*>(newComponent);
		} else if (dynamic_cast<PhysicsComponent*>(newComponent)) {
			m_physicsComp.push_back(dynamic_cast<PhysicsComponent*>(newComponent));
		}

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
