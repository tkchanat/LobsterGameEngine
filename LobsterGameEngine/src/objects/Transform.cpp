#include "pch.h"
#include "Transform.h"

namespace Lobster
{
    
	Transform::Transform() :
		WorldPosition(glm::vec3(0, 0, 0)),
		WorldEulerAngles(glm::vec3(0, 0, 0)),
		WorldRotation(glm::quat(1, 0, 0, 0)),
		LocalPosition(glm::vec3(0, 0, 0)),
		LocalEulerAngles(glm::vec3(0, 0, 0)),
		LocalScale(glm::vec3(1, 1, 1)),
		LocalRotation(glm::quat(1, 0, 0, 0)),
		m_matrix(glm::mat4(1.0f)) {}

	void Transform::Translate(float dx, float dy, float dz)
	{
		WorldPosition += glm::vec3(dx, dy, dz);
	}

	void Transform::UpdateMatrix()
	{
		LocalRotation = glm::quat(glm::radians(LocalEulerAngles));
		WorldRotation = glm::quat(glm::radians(WorldEulerAngles));
		m_matrix = glm::mat4_cast(WorldRotation) * glm::translate(WorldPosition) * glm::mat4_cast(LocalRotation) * glm::scale(LocalScale); //  Update world matrix
	}

	void Transform::RotateEuler(float degree, glm::vec3 axis)
	{
		LocalRotation = glm::angleAxis(glm::radians(degree), glm::normalize(axis)) * LocalRotation;
		LocalEulerAngles = glm::degrees(glm::eulerAngles(LocalRotation));
	}

	// Rotates the transform about axis passing through point in world coordinates by angle degrees.
	void Transform::RotateAround(float degree, glm::vec3 axis, glm::vec3 point)
	{
		float radian = glm::radians(degree);
		glm::vec3 delta = WorldPosition - point;
		WorldPosition = WorldPosition * glm::angleAxis(radian, glm::normalize(axis));
		LookAt(point);
	}

	void Transform::LookAt(glm::vec3 at)
	{
		glm::vec3 direction = glm::normalize(at - WorldPosition);
		LocalRotation = glm::quatLookAt(direction, glm::vec3(0, 1, 0));
	}

}
