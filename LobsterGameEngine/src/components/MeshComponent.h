#pragma once
#include "Component.h"
#include "graphics/VertexArray.h"
#include "graphics/Material.h"
#include "physics/AABB.h"
#include "system/FileSystem.h"

namespace Lobster
{
    class CameraComponent;
    class Material;

	struct PositionKey;
	struct RotationKey;
	struct ScaleKey;
	struct ChannelInfo;
	struct AnimationInfo;
	struct BoneNode {
		std::string Name;
		std::vector<BoneNode> Children;
		glm::mat4 Matrix;
	};
	struct MeshInfo {
		std::vector<VertexArray*> Meshes;
		std::vector<Material*> Materials;
		std::pair<glm::vec3, glm::vec3> Bound;
		std::unordered_map<std::string, int> BoneMap;
		std::vector<glm::mat4> BoneOffsets;
		std::vector<glm::mat4> BoneTransforms;
		BoneNode RootNode;
	};

	//	This class encapsulates all the geometric data for a model / mesh.
	//	When this component is enabled, the renderer will automatically render this mesh.
	//	Otherwise, this mesh will be ignored.
    class MeshComponent : public Component
    {
    private:
		// Mesh data
		std::string m_meshPath;
		MeshInfo m_meshInfo;
	private:
		// Skeletal animation data
		bool b_dirty = false;
		bool b_posing = false;
		bool b_animated = false;
		double m_animationTime = 0.0;
		double m_fadeAnimationTime = 0.0;
		double m_fadeDuration = 0.0;
		float m_timeMultiplier = 1.0f;
		int m_targetAnimation = 0;
		int m_currentAnimation = 0;
		std::vector<AnimationInfo> m_animations;
    public:
		MeshComponent() : Component(MESH_COMPONENT) {}
        MeshComponent(const char* meshPath, const char* materialPath = nullptr);
        MeshComponent(VertexArray* mesh, const char* materialPath = nullptr);
		MeshComponent(VertexArray* mesh, glm::vec3 min, glm::vec3 max, const char* materialPath = nullptr);
        virtual ~MeshComponent() override;
		virtual void OnAttach() override;
		virtual void OnUpdate(double deltaTime) override;
		virtual void OnImGuiRender() override;
		inline std::pair<glm::vec3, glm::vec3> GetBound() const { return m_meshInfo.Bound; }
		inline void PlayAnimation() { b_animated = b_posing = true; }
		inline void PauseAnimation() { b_animated = false; b_posing = true; }
		inline void StopAnimation() { m_animationTime = 0.0; b_animated = b_posing = false; }
		void CrossfadeAnimation(int animation, double fadeDuration);
	private:
		AnimationInfo LoadAnimation(const char* path);
		void SaveAnimation(int animation);
		void LoadFromFile(const char* meshPath, const char* materialPath);
		void UpdateBoneTransforms(const BoneNode & node, const glm::mat4 & parentTransform, const glm::mat4& globalInverseTransform);
		glm::vec3 InterpolatePosition(double animationTime, const ChannelInfo& channel) const;
		glm::quat InterpolateRotation(double animationTime, const ChannelInfo& channel) const;
		glm::vec3 InterpolateScale(double animationTime, const ChannelInfo& channel) const;
		virtual void Serialize(cereal::JSONOutputArchive& oarchive) override;
		virtual void Deserialize(cereal::JSONInputArchive& iarchive) override;
	private:
		friend class cereal::access;
		template <class Archive>
		void save(Archive & ar) const
		{
			// Mesh & Materials
			std::vector<std::string> materialNames;
			for (auto material : m_meshInfo.Materials) {
				materialNames.push_back(material->GetName());
			}
			ar(m_meshPath);
			ar(materialNames);

			// Animations
			std::vector<std::string> animationNames;
			for (auto& anim : m_animations) {
				animationNames.push_back(anim.Name);
			}
			ar(animationNames);
		}
		template <class Archive>
		void load(Archive & ar)
		{
			// Mesh & Materials
			ar(m_meshPath);
			std::vector<std::string> materialNames;
			ar(materialNames);
			for (auto name : materialNames) m_meshInfo.Materials.push_back(MaterialLibrary::Use(name.c_str()));
			
			// Animations
			std::vector<std::string> animationNames;
			ar(animationNames);
			for (auto name : animationNames) {
				m_animations.push_back(LoadAnimation(name.c_str()));
			}
		}
    };

	// =============================================
	// Animation-related structures
	struct PositionKey {
		double Time;
		glm::vec3 Value;
	private:
		friend class cereal::access;
		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(Time, Value);
		}
	};
	struct RotationKey {
		double Time;
		glm::quat Value;
	private:
		friend class cereal::access;
		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(Time, Value);
		}
	};
	struct ScaleKey {
		double Time;
		glm::vec3 Value;
	private:
		friend class cereal::access;
		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(Time, Value);
		}
	};
	struct ChannelInfo {
		std::string Name;
		std::vector<PositionKey> Position;
		std::vector<RotationKey> Rotation;
		std::vector<ScaleKey> Scale;
	private:
		friend class cereal::access;
		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(Name);
			ar(Position);
			ar(Rotation);
			ar(Scale);
		}
	};
	struct AnimationInfo {
		std::string Name;
		double Duration;
		double TicksPerSecond;
		std::unordered_map<std::string, int> ChannelMap; // BoneID to ChannelID
		std::vector<ChannelInfo> Channels;
	private:
		friend class cereal::access;
		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(Name);
			ar(Duration);
			ar(TicksPerSecond);
			ar(ChannelMap);
			ar(Channels);
		}
	};
    
}
