#pragma once
#include <vector>
#include "components/Component.h"
#include "physics/ColliderCollection.h"
#include "physics/PhysicsSystem.h"

namespace Lobster {
	class PhysicsComponent : public Component {
	public:
		static const char* PhysicsBodyTypes[];
		static const char* PhysicsType[];

		PhysicsComponent();
		virtual ~PhysicsComponent() override;

		virtual void VirtualCreate() override;
		virtual void VirtualDelete() override;

		void AddCollider(Collider* collider) {
			m_colliders.push_back(collider);
		}
		inline Collider* GetBoundingBox() const { return m_boundingBox; }
		inline std::vector<Collider*> GetColliders() const { return m_colliders; }

		bool Intersects(PhysicsComponent* other);
		virtual void Serialize(cereal::BinaryOutputArchive& oarchive) override;
		virtual void Deserialize(cereal::BinaryInputArchive& iarchive) override;
		virtual void OnPhysicsUpdate(double deltaTime) = 0;

		//	Block / Overlap / Ignore
		inline int GetPhysicsType() const { return m_physicsType; }
		void RemoveCollider(Collider* collider);

		//	ApplyForce: Force in Newton (N).
		inline void AddVelocity(glm::vec3 velocity) { m_velocity += glm::inverse(transform->WorldRotation) * velocity; }
		inline void ApplyForce(glm::vec3 force) { m_acceleration += glm::inverse(transform->WorldRotation) * force / m_mass; }

		inline void AddAngularVelocity(glm::vec3 velocity) { m_angularVelocity += velocity; }
		inline void ApplyAngularForce(glm::vec3 force) { m_angularVelocity += force / m_mass; }

		inline glm::vec3 GetVelocity() { return transform->WorldRotation * m_velocity; }
		inline glm::vec3 GetAcceleration() { return transform->WorldRotation * m_acceleration; }
		
		inline glm::vec3 GetAngularVelocity() { return m_angularVelocity; }
		inline glm::vec3 GetAngularAcceleration() { return m_angularAcceleration; }

	protected:
		//	Mass of component.
		float m_mass = 5.0f;

		//	Center of mass offset.
		glm::vec3 m_centerOfMass = glm::vec3(0, 0, 0);

		//	Velocity and acceleration for physics calculation.
		glm::vec3 m_velocity;
		glm::vec3 m_acceleration;

		//	Angular velocity and acceleration for physics calculation.
		glm::vec3 m_angularVelocity;
		glm::vec3 m_angularAcceleration;

		bool m_simulate = false;

		//	Bounding box is not visible to user, and is used by quick collision estimation only.
		Collider* m_boundingBox = nullptr;

		//	Array of colliders.
		std::vector<Collider*> m_colliders;

		//	Initialized to be of bound type.
		int m_physicsType = 0;

		//	Keeping track of colliding objects in the previous frame.
		std::vector<PhysicsComponent*> m_prevCollidingList;

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