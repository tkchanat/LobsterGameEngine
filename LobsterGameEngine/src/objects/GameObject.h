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
		friend class Scene;
		friend class ImGuiHierarchy;
    public:
		//	The world / model matrix of the game object
        Transform transform;

    private:
		static std::hash<uintptr_t> hashFunc;
		unsigned long long m_id;
        std::string m_name;
		GameObject* m_parent;
		std::vector<std::shared_ptr<GameObject>> m_children;
        std::vector<Component*> m_components;
		//	Shortcut to access the vector of colliders.
		std::vector<Collider*> m_colliders;
		//	Boolean to indicate whether we are changing object in previous frame.
		bool b_isChanging = false;
		//	Transform object to store previous state of game object prior to change.
		Transform m_transPrev;

		template<typename T, typename ...Args> Component* CreateComponent(Args&&... args);

    public:
        GameObject(const char* name);
        ~GameObject();
		void Destroy();
        void OnUpdate(double deltaTime);
		//	To update ImGui components that describes this game object's attributes
		virtual void OnImGuiRender();
		GameObject* AddComponent(Component* component);
		GameObject* AddChild(GameObject* child);
		template<typename T> T* GetComponent();
		std::pair<glm::vec3, glm::vec3> GetBound();
		inline unsigned long long GetId() { return m_id; }
        inline std::string GetName() const { return m_name; }
		inline GameObject* GetParent() const { return m_parent; }
		inline size_t GetChildrenCount() const { return m_children.size(); }
		//	RemoveComponent removes the component in vector and deletes comp afterwards.
		void RemoveComponent(Component* comp);
	private:
		inline std::vector<std::shared_ptr<GameObject>> GetChildren() const { return m_children; }
    };
    
    //=========================================
    //  Template Implementations
    //=========================================
	template<typename T, typename ...Args>
	inline Component * GameObject::CreateComponent(Args&&... args)
	{
		return new T(std::forward<Args>(args)...);
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
