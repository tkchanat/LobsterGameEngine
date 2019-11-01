#include "pch.h"
#include "system/Command.h"

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

	//	TODO: Print something even more meaningful here.
	std::string TransformCommand::ToString() const {
		std::string desc, act;
		if (m_original.WorldPosition != m_new.WorldPosition) {
			desc = "Translation of ";
			act = glm::to_string(m_original.WorldPosition) + " to " + glm::to_string(m_new.WorldPosition);
		} else if (m_original.LocalEulerAngles != m_new.LocalEulerAngles) {
			desc = "Rotation of ";
			act = glm::to_string(m_original.LocalEulerAngles) + " to " + glm::to_string(m_new.LocalEulerAngles);
		} else {
			desc = "Scaling of ";
			act = glm::to_string(m_original.LocalScale) + " to " + glm::to_string(m_new.LocalScale);
		}
		
		return desc + m_object->GetName() + " from " + act + ".";
	}
}