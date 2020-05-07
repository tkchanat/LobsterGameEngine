#pragma once
#include "physics/Collider.h"

namespace Lobster {
	class Material;
	class VertexArray;
	class VertexBuffer;

	class BoxCollider : public Collider {
	private:
		float m_vertexData[24]; // raw vertices position
		float m_vertexInitialData[24]; // raw initial vertices position

	public:
		BoxCollider(PhysicsComponent* physics, Transform transform = Transform());

		void SetOwner(GameObject* owner) override;
		void OnUpdate(double deltaTime) override;
		virtual void Draw() override;
		bool Intersects(glm::vec3 pos, glm::vec3 dir, float& t) override;

		virtual void Serialize(cereal::JSONOutputArchive& oarchive) override;
		virtual void Deserialize(cereal::JSONInputArchive& iarchive) override;

	protected:
		virtual std::vector<glm::vec3> GetVertices() const override;

	private:
		void UpdateRotation();
	private:
		friend class cereal::access;
		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(m_transform);
		}
	};
}