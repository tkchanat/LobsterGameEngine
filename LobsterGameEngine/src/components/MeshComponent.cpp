#include "pch.h"
#include "objects/GameObject.h"
#include "physics/AABB.h"
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
		Component(MESH_COMPONENT),
		m_meshPath(meshPath),
		m_meshInfo(MeshInfo()),
		b_animated(false),
		m_animationTime(0.0),
		m_timeMultiplier(1.0f),
		m_currentAnimation(0)
    {
		//	Clone the resource by file system before loading
		FileSystem::GetInstance()->addResource(meshPath);
		LoadFromFile(meshPath, materialPath);
    }

	MeshComponent::MeshComponent(VertexArray * mesh, const char * materialPath) :
		Component(MESH_COMPONENT),
		m_meshPath("")
	{
		m_meshInfo.Meshes.push_back(mesh);
		m_meshInfo.Materials.push_back(materialPath ? MaterialLibrary::Use(materialPath) : MaterialLibrary::UseDefault());
		m_meshInfo.Bound = { glm::vec3(0, 0, 0), glm::vec3(0.01, 0.01, 0.01) };
	}

	MeshComponent::MeshComponent(VertexArray* mesh, glm::vec3 min, glm::vec3 max, const char * materialPath) :
		Component(MESH_COMPONENT),
		m_meshPath("")
	{
		m_meshInfo.Meshes.push_back(mesh);
		m_meshInfo.Materials.push_back(materialPath ? MaterialLibrary::Use(materialPath) : MaterialLibrary::UseDefault());
		m_meshInfo.Bound = { min, max };
	}

	void MeshComponent::LoadFromFile(const char * meshPath, const char * materialPath)
	{
		// import mesh and load defined materials from model file
		m_meshInfo = MeshLoader::Load(meshPath);
		// if no materials are defined, use material of our own
		if (m_meshInfo.Materials.empty()) m_meshInfo.Materials.push_back((materialPath ? MaterialLibrary::Use(materialPath) : MaterialLibrary::UseDefault()));
	}
    
    MeshComponent::~MeshComponent()
    {
        //	Release memory
		for (int i = 0; i < m_meshInfo.Meshes.size(); ++i) {
			VertexArray* va = m_meshInfo.Meshes[i];
			if (va)	delete va;
			va = nullptr;
		}
    }
    
	void MeshComponent::OnAttach()
	{
		PhysicsComponent* physics = new Rigidbody();
		physics->SetEnabled(false);
		gameObject->AddComponent(physics);
	}

	void MeshComponent::OnUpdate(double deltaTime)
	{
		// Animation update
		if (b_animated)
		{
			float ticksPerSecond = m_meshInfo.Animations[m_currentAnimation].TicksPerSecond;
			m_animationTime += (deltaTime / 1000.0) * ticksPerSecond * m_timeMultiplier;
			m_animationTime = std::fmod(m_animationTime, m_meshInfo.Animations[m_currentAnimation].Duration);
			const glm::mat4& transform = m_meshInfo.RootNode.Matrix;
			UpdateBoneTransforms(m_meshInfo.RootNode, glm::mat4(1.0), glm::inverse(transform));
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
				ImGui::Text("Current Animation [%d]: %s", m_currentAnimation, m_meshInfo.Animations[m_currentAnimation].Name);
				ImGui::Text("Animation Time: %2.f", m_animationTime);
				ImGui::SliderFloat("Time Multiplier", &m_timeMultiplier, 0.5f, 2.0f);
				std::string label = fmt::format("{} Bones", m_meshInfo.BoneMap.size());
				if (ImGui::TreeNode(label.c_str()))
				{
					for (const auto& pair : m_meshInfo.BoneMap) {
						ImGui::BulletText("[%d] %s", pair.second, pair.first.c_str());
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

	void MeshComponent::UpdateBoneTransforms(const BoneNode& node, const glm::mat4& parentTransform, const glm::mat4& globalInverseTransform)
	{
		int boneID = node.BoneID;
		glm::mat4 nodeTransform = node.Matrix;
		// interpolate between keys
		const AnimationInfo& anim = m_meshInfo.Animations[m_currentAnimation];
		auto it = anim.ChannelMap.find(boneID);
		if (it != anim.ChannelMap.end()) {
			const ChannelInfo& channel = anim.Channels[it->second];
			// position
			glm::vec3 finalPosition = channel.Position[0].Value;
			if (channel.Position.size() > 1) {
				uint index([&](){
					assert(channel.Position.size() > 0);
					uint i = 0;
					for (; i < channel.Position.size() - 1; ++i)
						if (m_animationTime < channel.Position[i].Time)
							return i;
					return i - 1;
				}());
				uint nextIndex = index + 1;
				assert(nextIndex < channel.Position.size());
				float deltaTime = channel.Position[nextIndex].Time - channel.Position[index].Time;
				float factor = (m_animationTime - channel.Position[index].Time) / deltaTime;
				if (factor < 0.0f) factor = 0.0f;
				assert(factor <= 1.0f && "Factor must be smaller than 1.0f");
				const glm::vec3& start = channel.Position[index].Value;
				const glm::vec3& end = channel.Position[nextIndex].Value;
				finalPosition = glm::mix(start, end, factor);
			}
			// rotation
			glm::quat finalRotation = channel.Rotation[0].Value;
			if (channel.Rotation.size() > 1) {
				uint index([&]() {
					assert(channel.Rotation.size() > 0);
					uint i = 0;
					for (; i < channel.Rotation.size() - 1; ++i)
						if (m_animationTime < channel.Rotation[i].Time)
							return i;
					return i - 1;
				}());
				uint nextIndex = index + 1;
				assert(nextIndex < channel.Scale.size());
				float deltaTime = channel.Scale[nextIndex].Time - channel.Scale[index].Time;
				float factor = (m_animationTime - channel.Scale[index].Time) / deltaTime;
				if (factor < 0.0f) factor = 0.0f;
				assert(factor <= 1.0f && "Factor must be smaller than 1.0f");
				const glm::quat& start = channel.Rotation[index].Value;
				const glm::quat& end = channel.Rotation[nextIndex].Value;
				finalRotation = glm::slerp(start, end, factor);
			}
			// scale
			glm::vec3 finalScale = channel.Scale[0].Value;
			if (channel.Scale.size() > 1) {
				uint index([&]() {
					assert(channel.Scale.size() > 0);
					uint i = 0;
					for (; i < channel.Scale.size() - 1; ++i)
						if (m_animationTime < channel.Scale[i].Time)
							return i;
					return i - 1;
				}());
				uint nextIndex = index + 1;
				assert(nextIndex < channel.Scale.size());
				float deltaTime = channel.Scale[nextIndex].Time - channel.Scale[index].Time;
				float factor = (m_animationTime - channel.Scale[index].Time) / deltaTime;
				if (factor < 0.0f) factor = 0.0f;
				assert(factor <= 1.0f && "Factor must be smaller than 1.0f");
				const glm::vec3& start = channel.Scale[index].Value;
				const glm::vec3& end = channel.Scale[nextIndex].Value;
				finalScale = glm::mix(start, end, factor);
			}
			// combine transformations
			glm::mat4 scale = glm::scale(finalScale);
			glm::mat4 rotation = glm::toMat4(finalRotation);
			glm::mat4 translation = glm::translate(finalPosition);
			nodeTransform = translation * rotation * scale;
		}

		// recursively update bone transforms
		glm::mat4 globalTransform = parentTransform * nodeTransform;
		if (boneID < m_meshInfo.BoneTransforms.size()) {
			m_meshInfo.BoneTransforms[boneID] = globalInverseTransform * globalTransform * m_meshInfo.BoneOffsets[boneID];
		}
		for (int i = 0; i < node.Children.size(); ++i) {
			UpdateBoneTransforms(node.Children[i], globalTransform, globalInverseTransform);
		}
	}

	void MeshComponent::Serialize(cereal::JSONOutputArchive & oarchive)
	{
		//LOG("Serializing MeshComponent {}", m_meshPath);
		oarchive(*this);
	}

	void MeshComponent::Deserialize(cereal::JSONInputArchive & iarchive)
	{
		//LOG("Deserializing MeshComponent {}", m_meshPath);
		try {
			iarchive(*this);
			LoadFromFile(m_meshPath.c_str(), nullptr);
		}
		catch (std::exception e) {
			LOG("Deserializing MeshComponent {} failed. Reason: {}", m_meshPath, e.what());
		}
	}
}
