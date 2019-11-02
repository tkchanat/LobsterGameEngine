#include "pch.h"
#include "MeshComponent.h"
#include "system/FileSystem.h"
#include "graphics/Renderer.h"
#include "graphics/Material.h"
#include "graphics/meshes/MeshLoader.h"
#include "objects/Transform.h"

namespace Lobster
{
    
	MeshComponent::MeshComponent(const char* meshPath, const char* materialPath) :
		m_meshPath(meshPath)
    {
		//	Clone the resource by file system before loading
		FileSystem::GetInstance()->addResource(meshPath);
		m_materials.push_back(new Material(materialPath));

		auto minMax = MeshLoader::Load(meshPath, m_meshes, m_materials);
		bounds = { minMax.first, minMax.second };
    }

	MeshComponent::MeshComponent(VertexArray * mesh, const char * materialPath) :
		m_meshPath("")
	{
		m_meshes.push_back(mesh);
		m_materials.push_back(new Material(materialPath));
		//	TODO remove this hardcode part
		bounds = { glm::vec3(0, 0, 0), glm::vec3(0.01, 0.01, 0.01) };
	}

	MeshComponent::MeshComponent(VertexArray* mesh, glm::vec3 min, glm::vec3 max, const char * materialPath) :
		m_meshPath("")
	{
		m_meshes.push_back(mesh);
		m_materials.push_back(new Material(materialPath));
		bounds = { min, max };
	}
    
    MeshComponent::~MeshComponent()
    {
        //	Release memory
		for (VertexArray* va : m_meshes) if(va) delete va;
		for (Material* mat : m_materials) if(mat) delete mat;
		memset(m_meshes.data(), 0, sizeof(VertexArray*) * m_meshes.size());
		memset(m_materials.data(), 0, sizeof(Material*) * m_materials.size());
    }
    
	void MeshComponent::OnUpdate(double deltaTime)
	{
		//	Submit render command
		for (int i = 0; i < m_materials.size(); ++i)
		{
			RenderCommand command;
			command.UseMaterial = m_materials[i];
			command.UseVertexArray = m_meshes[i];
			command.UseWorldTransform = transform->GetMatrix();
			Renderer::Submit(command);
		}
	}

	void MeshComponent::OnImGuiRender()
	{
		if (ImGui::CollapsingHeader("MeshRenderer", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (int i = 0; i < m_materials.size(); ++i)
			{
				ImGui::Indent();
				m_materials[i]->OnImGuiRender();
				ImGui::Separator();
				ImGui::Unindent();
			}
		}
	}
}
