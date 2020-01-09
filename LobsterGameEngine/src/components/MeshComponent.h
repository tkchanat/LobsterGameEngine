#pragma once
#include "Component.h"
#include "graphics/VertexArray.h"
#include "graphics/Material.h"
#include "physics/AABB.h"
#include "system/FileSystem.h"

namespace Lobster
{
    class CameraComponent;
    
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
		MeshComponent() : Component(MESH_COMPONENT) {}
        MeshComponent(const char* meshPath, const char* materialPath = nullptr);
        MeshComponent(VertexArray* mesh, const char* materialPath = nullptr);
		MeshComponent(VertexArray* mesh, glm::vec3 min, glm::vec3 max, const char* materialPath = nullptr);
		void LoadFromFile(const char* meshPath, const char* materialPath);
        virtual ~MeshComponent() override;
		virtual void OnAttach() override;
		virtual void OnUpdate(double deltaTime) override;
		virtual void OnImGuiRender() override;
		virtual void Serialize(cereal::JSONOutputArchive& oarchive) override;
		virtual void Deserialize(cereal::JSONInputArchive& iarchive) override;
		inline std::pair<glm::vec3, glm::vec3> GetBound() const { return bounds; }
	private:
		friend class cereal::access;
		template <class Archive>
		void save(Archive & ar) const
		{
			std::vector<std::string> materialNames;
			for (auto material : m_materials)
				materialNames.push_back(material->GetName());

			ar(m_meshPath);
			ar(materialNames);
		}
		template <class Archive>
		void load(Archive & ar)
		{
			ar(m_meshPath);
			std::vector<std::string> materialNames;
			ar(materialNames);
			for (auto name : materialNames) m_materials.push_back(MaterialLibrary::Use(name.c_str()));
		}
    };
    
}
