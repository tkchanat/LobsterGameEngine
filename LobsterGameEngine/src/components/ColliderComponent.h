#pragma once
#include "Component.h"
#include "graphics/VertexArray.h"
#include "physics/AABB.h"

namespace Lobster
{

	//	A collider component is an invisible component used for physics calculation
	class ColliderComponent : public Component
	{
	private:
		//	Implement with naive AABB for now.
		AABB m_AABB;
		//	Whether this component is of trigger type.
		bool b_trigger = false;
		//	A trigger should be enabled by default.
		bool b_enabled = true;
	public:
		//	Should support all constructor types for meshes too - so we can build a bounding box from any given MeshComponent.
		ColliderComponent(glm::vec3 min, glm::vec3 max, bool trigger = false);

		virtual ~ColliderComponent() override;
		virtual void OnUpdate(double deltaTime) override;
		virtual void OnImGuiRender() override;
		virtual std::vector<glm::vec3> ObjectBound() override;

		bool Intersects(ColliderComponent* other);
	};

}
