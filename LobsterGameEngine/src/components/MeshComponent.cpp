#include "pch.h"
#include "objects/GameObject.h"
#include "physics/Rigidbody.h"
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
		LoadFromFile(meshPath, materialPath);
    }

	MeshComponent::MeshComponent(VertexArray * mesh, const char * materialPath) :
		m_meshPath("")
	{
		m_meshes.push_back(mesh);
		m_materials.push_back(materialPath ? MaterialLibrary::Use(materialPath) : MaterialLibrary::UseDefault());
		//	TODO remove this hard code part
		bounds = { glm::vec3(0, 0, 0), glm::vec3(0.01, 0.01, 0.01) };
	}

	MeshComponent::MeshComponent(VertexArray* mesh, glm::vec3 min, glm::vec3 max, const char * materialPath) :
		m_meshPath("")
	{
		m_meshes.push_back(mesh);
		m_materials.push_back(materialPath ? MaterialLibrary::Use(materialPath) : MaterialLibrary::UseDefault());
		bounds = { min, max };
	}

	void MeshComponent::LoadFromFile(const char * meshPath, const char * materialPath)
	{
		// import mesh and load defined materials from model file
		auto minMax = MeshLoader::Load(meshPath, m_meshes, m_materials);
		bounds = { minMax.first, minMax.second };
		// if no materials are defined, use material of our own
		if (m_materials.empty()) m_materials.push_back((materialPath ? MaterialLibrary::Use(materialPath) : MaterialLibrary::UseDefault()));
	}
    
    MeshComponent::~MeshComponent()
    {
        //	Release memory
		for (VertexArray* va : m_meshes) if(va) delete va;
    }
    
	void MeshComponent::OnAttach()
	{
		Rigidbody* rigidbody = new Rigidbody();
		rigidbody->SetEnabled(false);
		gameObject->AddComponent(rigidbody);
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
				m_materials[i]->OnImGuiRender(i);
				ImGui::Unindent();
			}
		}
	}

	void MeshComponent::Serialize(cereal::JSONOutputArchive & oarchive)
	{
		LOG("Serializing MeshComponent {}", m_meshPath);
		oarchive(*this);
	}

	void MeshComponent::Deserialize(cereal::JSONInputArchive & iarchive)
	{
		LOG("Deserializing MeshComponent {}", m_meshPath);
		try {
			iarchive(*this);
			LoadFromFile(m_meshPath.c_str(), nullptr);
		}
		catch (std::exception e) {
			LOG("Deserializing MeshComponent {} failed. Reason: {}", m_meshPath, e.what());
		}
	}
}
