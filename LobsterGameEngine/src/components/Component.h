#pragma once

namespace Lobster
{
    
    class GameObject;
    class Transform;
    class Script;

	//	This class is an abstract class for inheriting components.
	//	If you don't know what a component does / have no idea what a ECS (Entity-Component System),
	//	I highly recommend the following video to start with: https://www.youtube.com/watch?v=2rW7ALyHaas
    class Component
    {
        friend Script;
    protected:
		//	This boolean value determines whether this component should update or not.
        bool m_enabled;
		//	These pointers are just short-hands for referencing when user needs to obtain them while scripting.
		//	Therefore, we declare the class Script as a friend for accessing these pointers.
        GameObject* gameObject;
        Transform* transform;
    public:
		virtual ~Component() {}
        virtual void OnUpdate(double deltaTime) = 0;
		virtual void OnImGuiRender() = 0;
        inline void SetOwner(GameObject* owner) { gameObject = owner; }
        inline void SetOwnerTransform(Transform* t) { transform = t; }
        inline void RemoveOwner() { gameObject = nullptr; }
        inline bool IsEnabled() { return m_enabled; }
    protected:
        Component() : m_enabled(true), gameObject(nullptr), transform(nullptr) {}
    };
}
