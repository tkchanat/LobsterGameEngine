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

		//	Stores the collided objects right now. Clears at each frame.
		std::vector<GameObject*> m_colliding;
		//	Stores the collided objects at the previous frame. Updates over time.
		std::vector<GameObject*> m_collided;
		//	Stores the most recent colliding objects.
		std::vector<GameObject*> m_lastCollided;

		template<typename T, typename ...Args> Component* CreateComponent(Args&&... args);

    public:
        GameObject(const char* name);
        ~GameObject(); // TODO: private the destructor, forcing users to call Destroy() instead
		void Destroy();
		void OnBegin(); // call when the object is initialized in game mode
        void OnUpdate(double deltaTime);
		void Serialize(cereal::JSONOutputArchive& oarchive);
		void Deserialize(cereal::JSONInputArchive& iarchive);
		//	To update ImGui components that describes this game object's attributes
		virtual void OnImGuiRender();
		virtual void OnSimulationBegin();
		virtual void OnSimulationEnd();
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

		bool Intersects(GameObject* other);
		//	Functions to register collision or intersection of components.
		inline void HasCollided(GameObject* other) { m_colliding.push_back(other); }
		inline std::vector<GameObject*> GetCollided() const { return m_collided; }
		inline std::vector<GameObject*> GetColliding() const { return m_colliding; }
		inline std::vector<GameObject*> GetLastCollided() const { return m_lastCollided; }
		inline void frameElapse() {
			if (m_colliding.size() > 0) m_lastCollided = m_colliding;
			m_collided = m_colliding;
			m_colliding.clear();
		}


		//	Functions for scripting. Called by physics component.
		void OnCollide(GameObject* other);
		void OnEnter(GameObject* other);
		void OnOverlap(GameObject* other);
		void OnLeave(GameObject* other);
		bool IsOverlap(GameObject* other);

	private:
		friend class cereal::access;
		template <class Archive>
		void save(Archive & ar) const
		{
			// =============================================
			// record all children name
			std::vector<std::string> childrenNames;
			for (auto child : m_children) childrenNames.push_back(child->GetName());
			ar(childrenNames);
			// recursively serialize all children
			for (auto child : m_children) {
				child->Serialize(ar);
			}

			// =============================================
			// then serialize this GameObject's properties

			// transform
			ar(transform);
			// components
			std::vector<ComponentType> componentTypes;
			for (auto component : m_components) componentTypes.push_back(component->GetType());
			ar(componentTypes);
			// subsequence components
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

			// =============================================
			// then deserialize this GameObject's properties
			// transform 
			ar(transform);
			// components
			std::vector<ComponentType> componentTypes;
			std::vector<Component*> uninitializedComponents;
			ar(componentTypes);
			for (ComponentType type : componentTypes) uninitializedComponents.push_back(CreateComponentFromType(type));
			// initialize subsequence components and append to game object
			for (Component* component : uninitializedComponents) {
				if (!component) continue;
				component->Deserialize(ar);
				AddComponent(component);
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
