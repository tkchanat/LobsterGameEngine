#pragma once
#include <string>
#include <glm/glm.hpp>
#include "components/Component.h"
//#include "objects/GameObject.h"
#include "objects/Transform.h"

namespace Lobster {

    class GameObject;

	class Command {
	public:
		//	When a command could no longer be undo again.
		virtual ~Command() {};

		//	We will execute a command with the given details.
		virtual void Exec() = 0;

		//	We will execute a command with exactly opposite details.
		virtual void Undo() = 0;

		//	Convert the command into a string.
		virtual std::string ToString() const = 0;
	};

	//	For all commands that are used to translate, rotate or scale a game object.
	class TransformCommand : public Command {
	public:
		TransformCommand(GameObject* object, Transform t_original, Transform t_new);
		void Exec() override;
		void Undo() override;
		std::string ToString() const override;

	private:
		GameObject* m_object;
		Transform m_original;
		Transform m_new;
	};

	//	Deletion of a game object at root level.
	class DestroyObjectCommand : public Command {
	public:
		virtual ~DestroyObjectCommand() override;

		DestroyObjectCommand(GameObject* object, Scene* scene);
		void Exec() override;
		void Undo() override;
		std::string ToString() const override;

	private:
		GameObject* m_object;
		Scene* m_scene;
		bool b_isDeleted;
	};

	//	Creation of a game object at root level.
	class CreateObjectCommand : public Command {
	public:
		virtual ~CreateObjectCommand() override;

		CreateObjectCommand(GameObject* object, Scene* scene);
		void Exec() override;
		void Undo() override;
		std::string ToString() const override;

	private:
		GameObject* m_object;
		Scene* m_scene;
		bool b_isDeleted;
	};

	//	Deletion of a game object with a parent.
	class DestroyChildCommand : public Command {
	public:
		virtual ~DestroyChildCommand() override;

		DestroyChildCommand(GameObject* object, GameObject* parent);
		void Exec() override;
		void Undo() override;
		std::string ToString() const override;

	private:
		GameObject* m_object;
		GameObject* m_parent;
		bool b_isDeleted;
	};

	//	Creation of a game object at root level.
	class CreateChildCommand : public Command {
	public:
		virtual ~CreateChildCommand() override;

		CreateChildCommand(GameObject* object, GameObject* parent);
		void Exec() override;
		void Undo() override;
		std::string ToString() const override;

	private:
		GameObject* m_object;
		GameObject* m_parent;
		bool b_isDeleted;
	};

	//	Deletion of a component.
	class DestroyComponentCommand : public Command {
	public:
		virtual ~DestroyComponentCommand() override;

		DestroyComponentCommand(Component* component, GameObject* object);
		void Exec() override;
		void Undo() override;
		std::string ToString() const override;

	private:
		Component* m_component;
		GameObject* m_object;
		bool b_isDeleted;
	};

	//	Creation of a component.
	class CreateComponentCommand : public Command {
	public:
		virtual ~CreateComponentCommand() override;

		CreateComponentCommand(Component* component, GameObject* object);
		void Exec() override;
		void Undo() override;
		std::string ToString() const override;

	private:
		Component* m_component;
		GameObject* m_object;
		bool b_isDeleted;
	};

	//	Setting property on a component.
	template <typename T, typename U = Component, typename V = Component>
	class PropertyAssignmentCommand : public Command {
	public:
		PropertyAssignmentCommand(U* component, T* prop, T originalValue, T newValue, std::string action, void(V::*f) () = nullptr) :
			m_component(component),
			m_prop(prop),
			m_original(originalValue),
			m_new(newValue),
			m_action(action),
			m_func(f)
		{

		}

		void Exec() override {
			*m_prop = m_new;
			if (m_func != nullptr && dynamic_cast<V*>(m_component)) ((dynamic_cast<V*>(m_component))->*m_func)();
		}

		void Undo() override {
			*m_prop = m_original;
			if (m_func != nullptr && dynamic_cast<V*>(m_component)) ((dynamic_cast<V*>(m_component))->*m_func)();
		}

		std::string ToString() const override {
			return m_action;
		}

	private:
		U* m_component;
		T* m_prop;
		T m_original;
		T m_new;
		std::string m_action;
		void(V::*m_func) (void);
	};
}
