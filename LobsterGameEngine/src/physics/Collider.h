#pragma once
#include "components/Component.h"
#include "objects/Transform.h"

namespace Lobster {
	class PhysicsComponent;

	class Collider {
	public:
		const static char* ColliderType[];

		Collider(PhysicsComponent* physics, Transform transform = Transform(), bool draw = true) :
			physics(physics),
			m_transform(transform),
			m_draw(draw)
		{

		}
		virtual ~Collider() {}

		virtual void DebugDraw() = 0;
		virtual bool Intersects(Collider* collider) = 0;	
		virtual bool Intersects(glm::vec3 pos, glm::vec3 dir, float& t) = 0;	// ray intersection

	protected:
		bool m_enabled;
		bool m_draw = true;

		//	Stores the physics component that this object belongs to.
		PhysicsComponent* physics;

		//	Original transform of this collider.
		Transform* transform;

		//	Custom transform for each collider.
		//	Collider uses the AABB of the original game object, then apply this custom transform.
		Transform m_transform;

		//	Used to handle closing.
		bool m_show = true;

		//	Initialized to be of BoxCollider type.
		int m_colliderType = 0;

	public:
		virtual void OnUpdate(double deltaTime) = 0;
		virtual void OnImGuiRender();
		inline virtual void SetOwnerTransform(Transform* t) { transform = t; }
		inline PhysicsComponent* GetPhysics() const { return physics; }
		inline bool IsEnabled() { return m_enabled; }
	};
}