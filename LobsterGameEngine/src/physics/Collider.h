#pragma once
#include "components/Component.h"
#include "AABB.h"

namespace Lobster {
	//	BOUND / OVERLAP / IGNORED
	//	Note: IGNORE is a reserved word, therefore we used IGNORED instead. I hate you C++ :)
	enum ColliderType : uint {
		BOUND,
		OVERLAP,
		IGNORED
	};

	class Collider : public Component {
	public:
		Collider(std::vector<glm::vec3> minMax);
		void OnUpdate(double deltaTime) override;
		void OnImGuiRender() override;
		inline void SetOwnerTransform(Transform* t) {
			if (m_boundingBox) delete m_boundingBox;
			m_boundingBox = new AABB(m_minMax[0], m_minMax[1]);
			transform = t;
		}

		inline ColliderType GetColliderType() const { return m_colliderType; }
		inline void SetColliderType(ColliderType type) { m_colliderType = type; }

	private:
		ColliderType m_colliderType = ColliderType::BOUND;
		//	TODO: Extend AABB to inherit BoundingBox class.
		std::vector<glm::vec3> m_minMax;
		BoundingBox* m_boundingBox = nullptr;
	};
}