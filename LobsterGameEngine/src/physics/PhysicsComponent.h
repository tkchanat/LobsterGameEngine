#pragma once
#include <vector>
#include "components/Component.h"
#include "physics/ColliderCollection.h"

namespace Lobster {
	class PhysicsComponent : public Component {
	public:
		static const char* PhysicsBodyTypes[];
		const static char* PhysicsType[];

		PhysicsComponent() : Component(PHYSICS_COMPONENT) {}
		virtual ~PhysicsComponent() override {
			if(m_boundingBox) delete m_boundingBox;
			m_boundingBox = nullptr;
			for (Collider* collider : m_colliders) {
				if(collider) delete collider;
				collider = nullptr;
			}
		}

		void AddCollider(Collider* collider) {
			m_colliders.push_back(collider);
		}
		inline Collider* GetBoundingBox() const { return m_boundingBox; }
		inline std::vector<Collider*> GetColliders() const { return m_colliders; }

		bool Intersects(PhysicsComponent* other);
		virtual void Serialize(cereal::JSONOutputArchive& oarchive) override;
		virtual void Deserialize(cereal::JSONInputArchive& iarchive) override;
		virtual void OnPhysicsUpdate(double deltaTime) = 0;
		virtual void OnPhysicsLateUpdate(double deltaTime) = 0;

		//	Block / Overlap / Ignore
		inline int GetPhysicsType() const { return m_physicsType; }
		void RemoveCollider(Collider* collider);

	protected:
		//	Mass of component.
		float m_mass = 5.0f;

		//	Center of mass offset.
		glm::vec3 m_centerOfMass = glm::vec3(0, 0, 0);

		bool m_simulate = false;

		//	Bounding box is not visible to user, and is used by quick collision estimation only.
		Collider* m_boundingBox = nullptr;

		//	Array of colliders.
		std::vector<Collider*> m_colliders;

		//	Initialized to be of bound type.
		int m_physicsType = 0;
	private:
		friend class cereal::access;
		template <class Archive>
		void save(Archive & ar) const
		{
		}
		template <class Archive>
		void load(Archive & ar)
		{
		}
	};
}