#pragma once
#include "physics/ColliderComponent.h"

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
    class AABB : public ColliderComponent
    {
    public:
        glm::vec3 Center;
        glm::vec3 Min;
        glm::vec3 Max;
    private:
        Material* m_debugMaterial;
        VertexArray* m_debugMesh;
        VertexBuffer* m_debugVertexBuffer;
        float m_debugData[24]; // raw vertices position
		float m_debugInitialData[24]; // raw initial vertices position
		float m_debugTranslatedData[24]; // raw translated (for once) vertices position
    public:
        AABB(Transform transform = Transform(), bool draw = true);

		void SetOwner(GameObject* owner);

		void OnUpdate(double deltaTime) override;
        bool Intersects(ColliderComponent* other) override;

	protected:
		void Draw() override;

    private:
        void SetVertices(int setExtra = 0);
        void UpdateRotation(glm::quat rotation, glm::vec3 scale, bool translated = false);
    };

}
