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
		m_meshInfo(MeshInfo())
    {
		//	Clone the resource by file system before loading
		FileSystem::GetInstance()->addResource(meshPath);
		LoadFromFile(meshPath, materialPath);
		if (!m_animations.empty()) {
			b_dirty = true;
		}
    }

	MeshComponent::MeshComponent(VertexArray * mesh, const char * materialPath) :
		Component(MESH_COMPONENT),
		m_meshPath(""),
		m_meshInfo(MeshInfo())
	{
		m_meshInfo.Meshes.push_back(mesh);
		m_meshInfo.Materials.push_back(materialPath ? MaterialLibrary::Use(materialPath) : MaterialLibrary::UseDefault());
		m_meshInfo.Bound = { glm::vec3(0, 0, 0), glm::vec3(0.01, 0.01, 0.01) };
	}

	MeshComponent::MeshComponent(VertexArray* mesh, glm::vec3 min, glm::vec3 max, const char * materialPath) :
		Component(MESH_COMPONENT),
		m_meshPath(""),
		m_meshInfo(MeshInfo())
	{
		m_meshInfo.Meshes.push_back(mesh);
		m_meshInfo.Materials.push_back(materialPath ? MaterialLibrary::Use(materialPath) : MaterialLibrary::UseDefault());
		m_meshInfo.Bound = { min, max };
	}

	void MeshComponent::CrossfadeAnimation(int animation, double fadeDuration)
	{
		if (fadeDuration < 0.0 || fadeDuration > 1.0) {
			WARN("CrossfadeAnimation(): Please input a 0.0 - 1.0 value for fadeDuration instead.");
			return;
		}
		m_targetAnimation = animation;
		m_fadeDuration = fadeDuration * m_animations[animation].Duration;
	}

	AnimationInfo MeshComponent::LoadAnimation(const char* path)
	{
		// load animation from file
		AnimationInfo anim;
		bool exists = FileSystem::Exist(FileSystem::Path(path));
		if (!exists) return anim;
		std::stringstream ss = FileSystem::ReadStringStream(FileSystem::Path(path).c_str(), true);
		try {
			cereal::BinaryInputArchive iarchive(ss);
			iarchive(anim);
		}
		catch (std::exception e) {
			LOG("Loading animation {} failed! Reason: {}", path, e.what());
		}
		return anim;
	}

	void MeshComponent::SaveAnimation()
	{
		for (auto& anim : m_animations) {
			std::stringstream ss;
			{
				cereal::BinaryOutputArchive oarchive(ss);
				oarchive(anim);
			}
			FileSystem::WriteStringStream(FileSystem::Path(anim.Name).c_str(), ss, true);
		}
		b_dirty = false;
	}

	void MeshComponent::LoadFromFile(const char * meshPath, const char * materialPath)
	{
		// import mesh and load defined materials from model file
		m_meshInfo = MeshLoader::Load(meshPath);
		// load animation from file only if .anim file not found
		if (m_animations.empty()) {
			m_animations = MeshLoader::LoadAnimation(meshPath);
		}
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
		if (!m_animations.empty()) {
			if (b_animated)
			{
				float ticksPerSecond = m_animations[m_currentAnimation].TicksPerSecond;
				double elapsedTicks = (deltaTime / 1000.0) * ticksPerSecond * m_timeMultiplier;
				m_animationTime += elapsedTicks;
				m_animationTime = std::fmod(m_animationTime, m_animations[m_currentAnimation].Duration);
				assert(m_animationTime < m_animations[m_currentAnimation].Duration);
				if (m_currentAnimation != m_targetAnimation) {
					m_fadeAnimationTime += elapsedTicks;
					// fading finished
					if (m_fadeAnimationTime > m_fadeDuration) {
						m_currentAnimation = m_targetAnimation;
						m_animationTime = m_fadeAnimationTime;
						m_fadeAnimationTime = 0.0;
					}
				}
			}
			const glm::mat4& globalTransform = m_meshInfo.RootNode.Matrix;
			UpdateBoneTransforms(m_meshInfo.RootNode, glm::mat4(1.0), glm::inverse(globalTransform));
		}

		// Submit render command
		for (int i = 0; i < m_meshInfo.Materials.size(); ++i)
		{
			RenderCommand command;
			command.UseMaterial = m_meshInfo.Materials[i];
			command.UseVertexArray = m_meshInfo.Meshes[i];
			command.UseWorldTransform = transform->GetMatrix();
			if (b_posing) {
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
			std::string animationHeader = fmt::format("Skeletal Animation{}", b_dirty ? "*" : "");
			if (ImGui::CollapsingHeader(animationHeader.c_str(), ImGuiTreeNodeFlags_DefaultOpen) && !m_animations.empty())
			{
				// Save material pop up
				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Save", "", false))
						SaveAnimation();
					if (ImGui::MenuItem("Cancel", "", false))
						ImGui::CloseCurrentPopup();
					ImGui::EndPopup();
				}

				if (ImGui::Button("Play")) {
					PlayAnimation();
				}
				ImGui::SameLine();
				if (ImGui::Button("Pause")) {
					PauseAnimation();
				}
				ImGui::SameLine();
				if (ImGui::Button("Stop")) {
					StopAnimation();
				}
				ImGui::SetNextTreeNodeOpen(true);
				if (ImGui::TreeNode("Animations"))
				{
					int lastAnimation = m_currentAnimation;
					static int animIndex = m_currentAnimation;
					ImGui::SliderInt("Index", &animIndex, 0, m_animations.size() - 1);
					if (lastAnimation != animIndex) {
						CrossfadeAnimation(animIndex, 0.5);
					}

					for (int i = 0; i < m_animations.size(); ++i) {
						static char name[64] = "";
						AnimationInfo& anim = m_animations[i];
						std::string str = fmt::format("{}[{}]{}", i == m_currentAnimation ? "> " : "  ", i, anim.Name);
						ImGui::Text(str.c_str());
						// Change animation name
						if (ImGui::BeginPopupContextItem(str.c_str()))
						{
							ImGui::InputText("##edit", name, IM_ARRAYSIZE(name));
							if (ImGui::Button("Rename")) {
								std::string validName = "animations/" + StringOps::GetValidFilename(name) + ".anim";
								anim.Name = validName;
							}
							ImGui::EndPopup();
						}
					}
					if (ImGui::Button("Add Animation")) {
						std::string path = FileSystem::OpenFileDialog();
						if (!path.empty()) {
							m_animations.push_back(LoadAnimation(path.c_str()));
							b_dirty = true;
						}
					}
					ImGui::TreePop();
				}
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
		std::string boneName = node.Name;
		glm::mat4 nodeTransform = node.Matrix;
		// interpolate between keys
		glm::vec3 finalPosition = glm::vec3();
		glm::quat finalRotation = glm::quat();
		glm::vec3 finalScale = glm::vec3(1.0);
		const AnimationInfo& anim = m_animations[m_currentAnimation];
		auto it = anim.ChannelMap.find(boneName);
		if (it != anim.ChannelMap.end() && it->second < anim.Channels.size()) {
			const ChannelInfo& channel = anim.Channels[it->second];
			finalPosition = InterpolatePosition(m_animationTime, channel);
			finalRotation = InterpolateRotation(m_animationTime, channel);
			finalScale = InterpolateScale(m_animationTime, channel);
		}
		
		// cross fading...
		if (m_fadeAnimationTime > 0.0) {
			const AnimationInfo& anim = m_animations[m_targetAnimation];
			auto it = anim.ChannelMap.find(boneName);
			if (it != anim.ChannelMap.end() && it->second < anim.Channels.size()) {
				const ChannelInfo& channel = anim.Channels[it->second];
				float normalizeFade = m_fadeAnimationTime / m_fadeDuration;
				finalPosition = glm::mix(finalPosition, InterpolatePosition(m_fadeAnimationTime, channel), normalizeFade);
				finalRotation = glm::slerp(finalRotation, InterpolateRotation(m_fadeAnimationTime, channel), normalizeFade);
				finalScale = glm::mix(finalScale, InterpolateScale(m_fadeAnimationTime, channel), normalizeFade);
			}
		}

		// combine transformations
		glm::mat4 scale = glm::scale(finalScale);
		glm::mat4 rotation = glm::toMat4(finalRotation);
		glm::mat4 translation = glm::translate(finalPosition);
		nodeTransform = translation * rotation * scale;

		// recursively update bone transforms
		glm::mat4 globalTransform = parentTransform * nodeTransform;
		if (anim.ChannelMap.find(boneName) != anim.ChannelMap.end()) {
			int boneID = m_meshInfo.BoneMap[boneName];
			m_meshInfo.BoneTransforms[boneID] = globalInverseTransform * globalTransform * m_meshInfo.BoneOffsets[boneID];
		}
		for (int i = 0; i < node.Children.size(); ++i) {
			UpdateBoneTransforms(node.Children[i], globalTransform, globalInverseTransform);
		}
	}

	glm::vec3 MeshComponent::InterpolatePosition(double animationTime, const ChannelInfo & channel) const
	{
		glm::vec3 finalPosition = channel.Position[0].Value;
		if (channel.Position.size() > 1) {
			uint index([&]() {
				assert(channel.Position.size() > 0);
				uint i = 0;
				for (; i < channel.Position.size() - 1; ++i)
					if (animationTime < channel.Position[i].Time)
						return i;
				return i - 1;
			}());
			uint nextIndex = index + 1;
			assert(nextIndex < channel.Position.size());
			float deltaTime = channel.Position[nextIndex].Time - channel.Position[index].Time;
			float factor = (animationTime - channel.Position[index].Time) / deltaTime;
			factor = glm::clamp(factor, 0.0f, 1.0f);
			const glm::vec3& start = channel.Position[index].Value;
			const glm::vec3& end = channel.Position[nextIndex].Value;
			finalPosition = glm::mix(start, end, factor);
		}
		return finalPosition;
	}

	glm::quat MeshComponent::InterpolateRotation(double animationTime, const ChannelInfo & channel) const
	{
		glm::quat finalRotation = channel.Rotation[0].Value;
		if (channel.Rotation.size() > 1) {
			uint index([&]() {
				assert(channel.Rotation.size() > 0);
				uint i = 0;
				for (; i < channel.Rotation.size() - 1; ++i)
					if (animationTime < channel.Rotation[i].Time)
						return i;
				return i - 1;
			}());
			uint nextIndex = index + 1;
			assert(nextIndex < channel.Scale.size());
			float deltaTime = channel.Scale[nextIndex].Time - channel.Scale[index].Time;
			float factor = (animationTime - channel.Scale[index].Time) / deltaTime;
			factor = glm::clamp(factor, 0.0f, 1.0f);
			const glm::quat& start = channel.Rotation[index].Value;
			const glm::quat& end = channel.Rotation[nextIndex].Value;
			finalRotation = glm::slerp(start, end, factor);
		}
		return finalRotation;
	}

	glm::vec3 MeshComponent::InterpolateScale(double animationTime, const ChannelInfo & channel) const
	{
		glm::vec3 finalScale = channel.Scale[0].Value;
		if (channel.Scale.size() > 1) {
			uint index([&]() {
				assert(channel.Scale.size() > 0);
				uint i = 0;
				for (; i < channel.Scale.size() - 1; ++i)
					if (animationTime < channel.Scale[i].Time)
						return i;
				return i - 1;
			}());
			uint nextIndex = index + 1;
			assert(nextIndex < channel.Scale.size());
			float deltaTime = channel.Scale[nextIndex].Time - channel.Scale[index].Time;
			float factor = (animationTime - channel.Scale[index].Time) / deltaTime;
			factor = glm::clamp(factor, 0.0f, 1.0f);
			const glm::vec3& start = channel.Scale[index].Value;
			const glm::vec3& end = channel.Scale[nextIndex].Value;
			finalScale = glm::mix(start, end, factor);
		}
		return finalScale;
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
