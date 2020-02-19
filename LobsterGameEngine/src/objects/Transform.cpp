#include "pch.h"
#include "Transform.h"
#include "system/UndoSystem.h"

namespace Lobster
{
    
	Transform::Transform() :
		WorldPosition(glm::vec3(0, 0, 0)),
		LocalEulerAngles(glm::vec3(0, 0, 0)),
		LocalScale(glm::vec3(1, 1, 1)),
		OverallScale(1.0f),
		LocalRotation(glm::quat(1, 0, 0, 0)),
		m_matrix(glm::mat4(1.0f))
	{
	}

	void Transform::Translate(float dx, float dy, float dz)
	{
		WorldPosition += glm::vec3(dx, dy, dz);
	}

	void Transform::OnImGuiRender(GameObject* owner)
	{
		//	Check if transform is active, ie: we are trying to change the value of transform.
		static Transform transPrev;
		bool _activated = false;
		bool _changed = false;
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::DragFloat3("Position", glm::value_ptr(WorldPosition), 0.05f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			_changed |= ImGui::IsItemDeactivatedAfterChange();
			_activated |= ImGui::IsItemActivated();
			ImGui::DragFloat3("Rotation", glm::value_ptr(LocalEulerAngles), 1.0f, -360.0f, 360.0f);
			_changed |= ImGui::IsItemDeactivatedAfterChange();
			_activated |= ImGui::IsItemActivated();
			ImGui::DragFloat3("Scale", glm::value_ptr(LocalScale), 0.05f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			_changed |= ImGui::IsItemDeactivatedAfterChange();
			_activated |= ImGui::IsItemActivated();
			ImGui::DragFloat("Overall Scale", &OverallScale, 0.001f, 0.001f, 1000.f, "%.03f");
			_changed |= ImGui::IsItemDeactivatedAfterChange();
			_activated |= ImGui::IsItemActivated();
			if (_activated) {
				transPrev = Transform(*this);
			}
		}
		ImGui::Separator();
		//	Only send undo event if transform changed.
		if (_changed) {
			UndoSystem::GetInstance()->Push(new TransformCommand(owner, transPrev, *this));
		}
	}

	void Transform::UpdateMatrix()
	{
		LocalRotation = glm::quat(glm::radians(LocalEulerAngles));
		m_matrix = glm::translate(WorldPosition) * glm::mat4_cast(LocalRotation) * glm::scale(OverallScale * LocalScale); //  Update world matrix
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
