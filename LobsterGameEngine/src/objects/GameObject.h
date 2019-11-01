#pragma once

#include <typeinfo>
#include "Transform.h"
#include "components/ComponentCollection.h"
#include "system/filesystem.h"

#include "graphics/Scene.h"

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
		//	Boolean to indicate whether we are changing object in previous frame.
		bool b_isChanging = false;
		//	Transform object to store previous state of game object prior to change.
		Transform m_transPrev;
		//	Helper value to quickly check if we already have a MeshComponent added.
		bool b_hasMesh = false;
    public:
        GameObject(const char* name);
        ~GameObject();
        void OnUpdate(double deltaTime);
		// To update ImGui components that describes this game object's attributes

		virtual void OnImGuiRender(Scene* scene);
        template<typename T, typename ...Args> GameObject* AddComponent(Args&&... args);
        template<typename T> T* GetComponent();
		inline unsigned long long GetId() { return m_id; }
        inline std::string GetName() const { return m_name; }
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

		//	Determines if we want to add a mesh component.
		//	Attach a collider to it if we add our first MeshComponent,
		//	Return directly if we add more than one.
		if (b_hasMesh) return this;

		T* newComponent = new T(std::forward<Args>(args)...);
		newComponent->SetOwner(this);
		newComponent->SetOwnerTransform(&this->transform);
		m_components.push_back(newComponent);

		if (typeid(T) == typeid(MeshComponent)) {
			b_hasMesh = true;
			ColliderComponent* collider = new ColliderComponent(newComponent->ObjectBound()[0], newComponent->ObjectBound()[1]);
			collider->SetOwner(this);
			collider->SetOwnerTransform(&this->transform);
			m_components.push_back(collider);
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
