#pragma once
#include "Component.h"
#include "graphics/VertexArray.h"
#include "physics/AABB.h"
#include "system/FileSystem.h"

namespace Lobster
{
    class CameraComponent;
    class Material;
    
	//	This class encapsulates all the geometric data for a model / mesh.
	//	When this component is enabled, the renderer will automatically render this mesh.
	//	Otherwise, this mesh will be ignored.
    class MeshComponent : public Component
    {
    private:
		std::vector<VertexArray*> m_meshes;
        std::vector<Material*> m_materials;
		std::string m_meshPath;
		std::pair<glm::vec3, glm::vec3> bounds;
    public:
        MeshComponent(const char* meshPath, const char* materialPath = "materials/default.mat");
        MeshComponent(VertexArray* mesh, const char* materialPath = "materials/default.mat");
		MeshComponent(VertexArray* mesh, glm::vec3 min, glm::vec3 max, const char* materialPath = "materials/default.mat");
        virtual ~MeshComponent() override;
		virtual void OnUpdate(double deltaTime) override;
		virtual void OnImGuiRender() override;
		inline std::pair<glm::vec3, glm::vec3> GetBound() const { return bounds; }
    };
    
}
