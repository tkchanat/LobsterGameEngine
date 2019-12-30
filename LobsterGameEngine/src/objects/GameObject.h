#pragma once

#include <typeinfo>
#include "Transform.h"
#include "components/ComponentCollection.h"
#include "system/filesystem.h"

//#include "graphics/Scene.h"
#include "physics/PhysicsComponentCollection.h"

namespace Lobster
{
	//	This class is the building block of a scene.
	//	This acts as a node with a list of components attached, and have a spatial relationship with the world origin.
    class GameObject
    {
		friend class Scene;
    public:
		//	The world / model matrix of the game object
        Transform transform;

    private:
		static std::hash<uintptr_t> hashFunc;
		unsigned long long m_id;
        std::string m_name;
		GameObject* m_parent;
		std::vector<GameObject*> m_children;
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
        ~GameObject(); // TODO: private the destructor, forcing users to call Destroy() instead
		void Destroy();
        void OnUpdate(double deltaTime);
		void OnImGuiRender();
		void Serialize(cereal::JSONOutputArchive& oarchive);
		void Deserialize(cereal::JSONInputArchive& iarchive);
		GameObject* AddComponent(Component* component);
		GameObject* AddChild(GameObject* child);
		template<typename T> T* GetComponent();
		std::pair<glm::vec3, glm::vec3> GetBound();
		inline unsigned long long GetId() { return m_id; }
        inline std::string GetName() const { return m_name; }
		inline GameObject* GetParent() const { return m_parent; }
		inline std::vector<GameObject*> GetChildren() const { return m_children; }
		inline size_t GetChildrenCount() const { return m_children.size(); }
		//	RemoveComponent removes the component in vector and deletes comp afterwards.
		void RemoveComponent(Component* comp);
	private:
		friend class cereal::access;
		template <class Archive>
		void save(Archive & ar) const
		{
			// mark down all children name
			std::vector<std::string> childrenNames;
			for (auto child : m_children) childrenNames.push_back(child->GetName());
			ar(childrenNames);
			// recursively serialize all children
			for (auto child : m_children) {
				child->Serialize(ar);
			}

			// then deserialize this GameObject's properties
			std::vector<std::string> componentNames;
			for (auto component : m_components) componentNames.push_back(typeid(*component).name());
			ar(componentNames);

			for (auto component : m_components) {
				component->Serialize(ar);
			}
		}
		template <class Archive>
		void load(Archive & ar)
		{
			// recreate all children
			std::vector<std::string> childrenNames;
			ar(childrenNames);
			for (auto name : childrenNames) AddChild(new GameObject(name.c_str()));

			// recursively deserialize all children
			for (auto child : m_children) {
				child->Deserialize(ar);
			}

			std::vector<std::string> componentNames;
			ar(componentNames);
			for (auto name : componentNames) AddComponent(CreateComponentFromTypeName(name, ar));

			for (auto component : m_components) {
				component->Deserialize(ar);
			}
		}
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
