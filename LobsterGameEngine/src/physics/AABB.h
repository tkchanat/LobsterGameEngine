#pragma once
#include "physics/Collider.h"

namespace Lobster
{

    class Material;
    class VertexArray;
    class VertexBuffer;

    class AABB : public Collider
    {
    public:
        glm::vec3 Center;
        glm::vec3 Min;
        glm::vec3 Max;

    private:
        float m_vertexData[24]; // raw vertices position
		float m_vertexInitialData[24]; // raw initial vertices position

    public:
        AABB(PhysicsComponent* physics, Transform transform = Transform());
		virtual ~AABB() override;
		void SetOwner(GameObject* owner) override;
		void OnUpdate(double deltaTime) override;
		virtual void Draw() override;
		bool Intersects(glm::vec3 pos, glm::vec3 dir, float& t) override;

	protected:
		virtual std::vector<glm::vec3> GetVertices() const override;

    private:
        void SetVertices(bool initialize);
        void UpdateRotation();
    };

}
