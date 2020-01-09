#pragma once
#include "components/Component.h"
#include "objects/Transform.h"

namespace Lobster {
	class PhysicsComponent;

	class Material;
	class VertexArray;
	class VertexBuffer;

	class Collider {
	public:
		const static char* ColliderType[];

		Collider(PhysicsComponent* physics, Transform transform = Transform()) :
			m_debugMaterial(nullptr),
			m_debugMesh(nullptr),
			m_debugVertexBuffer(nullptr),
			physics(physics),
			m_transform(transform)
		{

		}
		virtual ~Collider() {}

		virtual void DebugDraw() = 0;
		//virtual bool Intersects(Collider* collider) = 0;
		bool Intersects(Collider* collider) { return Intersects(this, collider); }
		virtual bool Intersects(glm::vec3 pos, glm::vec3 dir, float& t) = 0;	// ray intersection

	protected:
		//	Materials for rendering
		glm::vec4 m_debugColor;
		Material* m_debugMaterial;
		VertexArray* m_debugMesh;
		VertexBuffer* m_debugVertexBuffer;

		bool m_enabled = true;

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

	private:
		//	Record previous value of collider type.
		int m_prevColliderType = 0;

	public:
		virtual void SetOwner(GameObject* owner) = 0;
		virtual void OnUpdate(double deltaTime) = 0;
		virtual void OnImGuiRender();
		inline virtual void SetOwnerTransform(Transform* t) { transform = t; }
		inline PhysicsComponent* GetPhysics() const { return physics; }
		inline bool IsEnabled() { return m_enabled; }

	protected:
		inline void SetColliderType(int colliderType) { m_colliderType = colliderType; }
		virtual std::vector<glm::vec3> GetVertices() const = 0;

	private:
		static bool Intersects(Collider* c1, Collider* c2);
		//	pass 2 points from each object to check if they collided on the vector projection of o1_max - o1_min.
		static bool IsEnclosed(glm::vec3 o1_min, glm::vec3 o1_max, std::vector<glm::vec3> o2);
	};
}