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
			m_vertexMaterial(nullptr),
			m_mesh(nullptr),
			m_vertexBuffer(nullptr),
			physics(physics),
			m_transform(transform)
		{

		}
		virtual ~Collider() {}

		inline void VirtualCreate() { b_isVirtuallyDeleted = false; }
		inline void VirtualDelete() { b_isVirtuallyDeleted = true; }

		virtual void Draw() = 0;
		bool Intersects(Collider* collider) { return Intersects(this, collider); }
		virtual bool Intersects(glm::vec3 pos, glm::vec3 dir, float& t) = 0;	// ray intersection

		//	Custom transform for each collider.
		//	Collider uses the AABB of the original game object, then apply this custom transform.
		Transform m_transform;

	protected:
		//	Materials for rendering
		glm::vec4 m_vertexColor;
		Material* m_vertexMaterial;
		VertexArray* m_mesh;
		VertexBuffer* m_vertexBuffer;

		//	Stores the physics component that this object belongs to.
		PhysicsComponent* physics;

		//	Original transform of this collider.
		Transform* transform;

	private:
		//	Is this collider enaled? (toggled hrough ImGui interface)
		bool m_enabled = true;

		//	Is this collider created? (managed by undo system)
		bool b_isVirtuallyDeleted = false;

		//	Is m_transform changing?
		bool b_isChanging;

		//	Recorded for undo system, previous m_transform.
		Transform m_prevTransform;

		//	Used to handle closing.
		bool m_show = true;

		//	Initialized to be of BoxCollider type.
		int m_colliderType = 0;

		//	Record previous value of collider type.
		int m_prevColliderType = 0;

	public:
		virtual void SetOwner(GameObject* owner) = 0;
		virtual void OnUpdate(double deltaTime) = 0;
		virtual void OnImGuiRender();
		virtual void Serialize(cereal::JSONOutputArchive& oarchive) = 0;
		virtual void Deserialize(cereal::JSONInputArchive& iarchive) = 0;
		inline virtual void SetOwnerTransform(Transform* t) { transform = t; }
		inline PhysicsComponent* GetPhysics() const { return physics; }
		inline bool IsEnabled() { return m_enabled && !b_isVirtuallyDeleted; }

	protected:
		inline void SetColliderType(int colliderType) { m_colliderType = colliderType; }
		virtual std::vector<glm::vec3> GetVertices() const = 0;
	private:
		//	Update collider component after collider type update.
		Collider* UpdateColliderType(int colliderType);

		static bool Intersects(Collider* c1, Collider* c2);
		//	pass 2 points from each object to check if they collided on the vector projection of o1_max - o1_min.
		static bool IsEnclosed(glm::vec3 o1_min, glm::vec3 o1_max, std::vector<glm::vec3> o2);
	};
}