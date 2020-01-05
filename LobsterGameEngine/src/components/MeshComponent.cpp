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
		m_meshPath(meshPath),
		m_meshInfo(MeshInfo()),
		b_animated(false),
		m_animationTime(0.0),
		m_timeMultiplier(1.0f)
    {
		//	Clone the resource by file system before loading
		FileSystem::GetInstance()->addResource(meshPath);
		// import mesh and load defined materials from model file
		m_meshInfo = MeshLoader::Load(meshPath);
		// if no materials are defined, use material of our own
		if(m_meshInfo.Materials.empty()) m_meshInfo.Materials.push_back(MaterialLibrary::Use(materialPath));
    }

	MeshComponent::MeshComponent(VertexArray * mesh, const char * materialPath) :
		m_meshPath("")
	{
		m_meshInfo.Meshes.push_back(mesh);
		m_meshInfo.Materials.push_back(MaterialLibrary::Use(materialPath));
		//	TODO remove this hard code part
		m_meshInfo.Bound = { glm::vec3(0, 0, 0), glm::vec3(0.01, 0.01, 0.01) };
	}

	MeshComponent::MeshComponent(VertexArray* mesh, glm::vec3 min, glm::vec3 max, const char * materialPath) :
		m_meshPath("")
	{
		m_meshInfo.Meshes.push_back(mesh);
		m_meshInfo.Materials.push_back(MaterialLibrary::Use(materialPath));
		m_meshInfo.Bound = { min, max };
	}
    
    MeshComponent::~MeshComponent()
    {
        //	Release memory
		for (VertexArray* va : m_meshInfo.Meshes) if(va) delete va;
		memset(m_meshInfo.Meshes.data(), 0, sizeof(VertexArray*) * m_meshInfo.Meshes.size());
    }
    
	void MeshComponent::OnAttach()
	{
		Rigidbody* rigidbody = new Rigidbody();
		rigidbody->SetEnabled(false);
		gameObject->AddComponent(rigidbody);
	}

	void MeshComponent::OnUpdate(double deltaTime)
	{
		// Animation update
		if (b_animated)
		{
			float ticksPerSecond = m_meshInfo.Animations[0].TicksPerSecond;
			m_animationTime += (deltaTime / 1000.0) * ticksPerSecond * m_timeMultiplier;
			m_animationTime = std::fmod(m_animationTime, m_meshInfo.Animations[0].Duration);
		}

		// Submit render command
		for (int i = 0; i < m_meshInfo.Materials.size(); ++i)
		{
			RenderCommand command;
			command.UseMaterial = m_meshInfo.Materials[i];
			command.UseVertexArray = m_meshInfo.Meshes[i];
			command.UseWorldTransform = transform->GetMatrix();
			if (b_animated) {
				command.UseBoneTransforms = m_meshInfo.BoneTransforms.data();
			}
			Renderer::Submit(command);
		}
	}

	void MeshComponent::OnImGuiRender()
	{
		if (ImGui::CollapsingHeader("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent();
			if (m_meshInfo.HasAnimation() && ImGui::CollapsingHeader("Skeletal Animation", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Checkbox("Animated?", &b_animated);
				ImGui::Text("Animation Time: %2.f", m_animationTime);
				ImGui::SliderFloat("Time Multiplier", &m_timeMultiplier, 0.5f, 2.0f);
				std::string label = fmt::format("{} Bones", m_meshInfo.BoneMap.size());
				if (ImGui::TreeNode(label.c_str()))
				{
					for (const auto& pair : m_meshInfo.BoneMap) {
						ImGui::BulletText(pair.first.c_str());
					}
					ImGui::TreePop();
				}
			}
			if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_None))
			{
				ImGui::Indent();
				for (int i = 0; i < m_meshInfo.Materials.size(); ++i)
				{
					m_meshInfo.Materials[i]->OnImGuiRender(i);
					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::Spacing();
				}
				ImGui::Unindent();
			}
			ImGui::Unindent();
		}
	}
	void MeshComponent::UpdateBoneTransforms()
	{
	}
	void MeshComponent::ReadNodeHierarchy(const BoneNode & node, const glm::mat4 & parentTransform)
	{
	}
}
