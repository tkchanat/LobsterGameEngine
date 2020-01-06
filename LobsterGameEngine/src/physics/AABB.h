#pragma once
#include "physics/Collider.h"

namespace Lobster
{

    class Material;
    class VertexArray;
    class VertexBuffer;

    //	TODO:
    //	extend this class in the future with reference to Unity's API: https://docs.unity3d.com/ScriptReference/Bounds.html
    //	AABB is one kind of bounding volume. It can be used NOT ONLY for collision detection,
    //	but also identifying mesh boundary and quick look-up in scene graph etc.

	//	TODO 2:
	//	After completion of the function, create a proper draw function.
	//	We should not create a private draw function and rely on friend class to render the bounding box.
    class AABB : public Collider
    {
    public:
        glm::vec3 Center;
        glm::vec3 Min;
        glm::vec3 Max;
    private:
        float m_debugData[24]; // raw vertices position
		float m_debugInitialData[24]; // raw initial vertices position
    public:
        AABB(PhysicsComponent* physics, Transform transform = Transform());

		void SetOwner(GameObject* owner) override;
		void OnUpdate(double deltaTime) override;
		virtual void DebugDraw() override;
		bool Intersects(glm::vec3 pos, glm::vec3 dir, float& t) override;

	protected:
		//virtual void DebugDraw() override;
		virtual std::vector<glm::vec3> GetVertices() const override;

    private:
        void SetVertices(bool initialize);
        void UpdateRotation(glm::quat rotation, glm::vec3 scale);
    };

}
