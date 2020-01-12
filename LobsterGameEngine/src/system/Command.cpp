#include "pch.h"
#include "system/Command.h"
#include "graphics/Scene.h"
#include "objects/GameObject.h"

namespace Lobster {
	TransformCommand::TransformCommand(GameObject* object, Transform t_original, Transform t_new) :
		m_object(object),
		m_original(t_original),
		m_new(t_new)
	{

	}

	//	Executing a transformation command is to change the matrix to the new one.
	void TransformCommand::Exec() {
		m_object->transform = m_new;
	}

	//	By performing undo on a transformation, we are essentially changing the transforming matrix back to the original one.
	void TransformCommand::Undo() {
		m_object->transform = m_original;
	}

	std::string TransformCommand::ToString() const {
		//	act stores the action, vect stores the vector details.
		std::string act, vect;
		if (m_original.WorldPosition != m_new.WorldPosition) {
			act = "Translated ";
			vect = StringOps::ToString(m_new.WorldPosition);
		} else if (m_original.LocalEulerAngles != m_new.LocalEulerAngles) {
			act = "Rotated ";
			vect = StringOps::ToString(m_new.LocalEulerAngles);
		} else {
			act = "Scaled ";
			vect = StringOps::ToString(m_new.LocalScale);
		}
		
		return act + m_object->GetName() + " to " + vect;
	}

	DestroyObjectCommand::DestroyObjectCommand(GameObject* object, Scene* scene) :
		m_object(object),
		m_scene(scene),
		b_isDeleted(true)
	{

	}

	DestroyObjectCommand::~DestroyObjectCommand() {
		if (b_isDeleted) delete m_object;
		m_object = nullptr;
		m_scene = nullptr;
	}

	//	Executing a destroy command is to soft delete an object.
	void DestroyObjectCommand::Exec() {
		m_scene->RemoveGameObject(m_object);
		b_isDeleted = true;
	}

	//	By performing an undo on destroy command, the object should come back to live.
	void DestroyObjectCommand::Undo() {
		m_scene->AddGameObject(m_object);
		b_isDeleted = false;
	}

	std::string DestroyObjectCommand::ToString() const {
		return "Destroyed " + m_object->GetName();
	}

	CreateObjectCommand::CreateObjectCommand(GameObject* object, Scene* scene) :
		m_object(object),
		m_scene(scene),
		b_isDeleted(false)
	{

	}

	CreateObjectCommand::~CreateObjectCommand() {
		if (b_isDeleted) delete m_object;
		m_object = nullptr;
		m_scene = nullptr;
	}

	//	Executing a create command is to bring an object to live.
	void CreateObjectCommand::Exec() {
		m_scene->AddGameObject(m_object);
		b_isDeleted = false;
	}

	//	By performing an undo on create command, the object should be deleted.
	void CreateObjectCommand::Undo() {
		m_scene->RemoveGameObject(m_object);
		b_isDeleted = true;
	}

	std::string CreateObjectCommand::ToString() const {
		return "Created " + m_object->GetName();
	}
}
