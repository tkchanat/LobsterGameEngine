#pragma once

namespace Lobster
{
    
    class GameObject;
    class Transform;
    class Script;

	// [WARNING] 
	// It's best to not change the order here. Otherwise, compatibility issue will occur.
	// Always append this list rather than inserting.
	enum ComponentType : uint {
		UNKNOWN,
		MESH_COMPONENT,
		CAMERA_COMPONENT,
		LIGHT_COMPONENT,
		PHYSICS_COMPONENT,
		SCRIPT_COMPONENT,
		AUDIO_SOURCE_COMPONENT,
		AUDIO_LISTENER_COMPONENT,
		PARTICLE_COMPONENT
	};

	//	Warning 2: Remember to register your component name (in proper English) in Component.cpp too.

	//	This class is an abstract class for inheriting components.
	//	If you don't know what a component does / have no idea what a ECS (Entity-Component System),
	//	I highly recommend the following video to start with: https://www.youtube.com/watch?v=2rW7ALyHaas
    class Component
    {
        friend Script;

    protected:
		ComponentType m_type;
		static std::string componentName[];
		//	This boolean value determines whether this component should update or not.
        bool m_enabled;
		//	These pointers are just short-hands for referencing when user needs to obtain them while scripting.
		//	Therefore, we declare the class Script as a friend for accessing these pointers.
        GameObject* gameObject;
        Transform* transform;

		//	Used to handle virtual create / delete. For components that doesn't need this feture, this variable will constantly stay at false.
		bool b_isVirtuallyDeleted = false;

		//	Used to handle closing.
		bool m_show = true;
		void RemoveComponent(Component* comp);

    public:
		virtual ~Component() {}

		virtual void VirtualCreate() {}
		virtual void VirtualDelete() {}

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnBegin() {}
        virtual void OnUpdate(double deltaTime) = 0;
		virtual void OnImGuiRender() = 0;
		virtual void OnSimulationBegin() {}
		virtual void OnSimulationEnd() {} 
		virtual void SetOwner(GameObject* owner) { gameObject = owner; }
		virtual void Serialize(cereal::BinaryOutputArchive& oarchive) = 0;
		virtual void Deserialize(cereal::BinaryInputArchive& iarchive) = 0;
        inline virtual void SetOwnerTransform(Transform* t) { transform = t; }
        inline void RemoveOwner() { gameObject = nullptr; }
		inline GameObject* GetOwner() { return gameObject; }	//	TODO: Discuss if this class is needed, used for print intersection result only for now.
		inline ComponentType GetType() const { return m_type; }
		inline std::string GetTypeName() const { return componentName[m_type]; }
        inline bool IsEnabled() { return m_enabled; }
		inline void SetEnabled(bool enabled) { m_enabled = enabled; }
    protected:
        explicit Component(ComponentType type) : m_type(type), m_enabled(true), gameObject(nullptr), transform(nullptr) {}
    };

	Component* CreateComponentFromType(const ComponentType& typeName);
}
