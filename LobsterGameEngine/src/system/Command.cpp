#include "pch.h"
#include "system/Command.h"
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
}
