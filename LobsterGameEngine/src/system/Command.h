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

	//	Creation / Deletion of a game object.
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

	//	Creation of a game object.
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
}
