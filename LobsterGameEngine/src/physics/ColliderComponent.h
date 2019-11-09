#pragma once
#include "components/Component.h"
#include "objects/Transform.h"

namespace Lobster {
	class ColliderComponent : public Component {
	public:
		const static char* ColliderType[];

		ColliderComponent(Transform transform = Transform(), bool draw = true) :
			m_transform(transform),
			m_draw(draw)
		{

		}

		void OnImGuiRender();

		virtual void DebugDraw() = 0;
		virtual bool Intersects(ColliderComponent* collider) = 0;

	protected:
		bool m_draw = true;

		//	Custom transform for each collider.
		//	Collider uses the AABB of the original game object, then apply this custom transform.
		Transform m_transform;

		//	Initialized to be of BoxCollider type.
		int m_colliderType = 0;
	};
}