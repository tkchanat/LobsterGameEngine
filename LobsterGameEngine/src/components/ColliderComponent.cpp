#include "pch.h"
#include "ColliderComponent.h"
#include "graphics/meshes/MeshLoader.h"
#include "objects/Transform.h"

namespace Lobster
{
	ColliderComponent::ColliderComponent(glm::vec3 min, glm::vec3 max, bool trigger) : b_trigger(trigger)
	{
		m_AABB = AABB(min, max);
	}

	ColliderComponent::~ColliderComponent()
	{

	}

	void ColliderComponent::OnUpdate(double deltaTime)
	{
		// update AABB
		m_AABB.Center = transform->WorldPosition;
		m_AABB.UpdateRotation(transform->LocalRotation, transform->LocalScale);
		if (b_enabled)
		{
			m_AABB.DebugDraw();
		}
	}

	void ColliderComponent::OnImGuiRender()
	{
		if (ImGui::CollapsingHeader("Collider", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Checkbox("Enable", &b_enabled);
		}
	}

	std::vector<glm::vec3> ColliderComponent::ObjectBound()
	{
		return std::vector<glm::vec3>();
	}

	bool ColliderComponent::Intersects(ColliderComponent * other)
	{
		//	Naive implementation for now
		return m_AABB.Intersects(other->m_AABB);
	}
}
