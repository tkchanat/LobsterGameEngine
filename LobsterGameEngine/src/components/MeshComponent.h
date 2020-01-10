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
		int BoneID;
		std::vector<BoneNode> Children;
		glm::mat4 Matrix;
	};
	struct MeshInfo {
		std::vector<VertexArray*> Meshes;
		std::vector<Material*> Materials;
		std::pair<glm::vec3, glm::vec3> Bound;
		std::vector<AnimationInfo> Animations;
		std::unordered_map<std::string, uint> BoneMap;
		std::vector<glm::mat4> BoneOffsets;
		std::vector<glm::mat4> BoneTransforms;
		BoneNode RootNode;
		inline bool HasAnimation() const { return !Animations.empty(); }
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
		bool b_animated;
		double m_animationTime;
		float m_timeMultiplier;
		int m_currentAnimation;
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
		inline std::pair<glm::vec3, glm::vec3> GetBound() const { return m_meshInfo.Bound; }
	private:
		void UpdateBoneTransforms(const BoneNode & node, const glm::mat4 & parentTransform, const glm::mat4& globalInverseTransform);
		virtual void Serialize(cereal::JSONOutputArchive& oarchive) override;
		virtual void Deserialize(cereal::JSONInputArchive& iarchive) override;
	private:
		friend class cereal::access;
		template <class Archive>
		void save(Archive & ar) const
		{
			std::vector<std::string> materialNames;
			for (auto material : m_meshInfo.Materials)
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
			for (auto name : materialNames) m_meshInfo.Materials.push_back(MaterialLibrary::Use(name.c_str()));
		}
    };

	// =============================================
	// Animation-related structures
	struct PositionKey {
		double Time;
		glm::vec3 Value;
	};
	struct RotationKey {
		double Time;
		glm::quat Value;
	};
	struct ScaleKey {
		double Time;
		glm::vec3 Value;
	};
	struct ChannelInfo {
		std::string Name;
		std::vector<PositionKey> Position;
		std::vector<RotationKey> Rotation;
		std::vector<ScaleKey> Scale;
	};
	struct AnimationInfo {
		std::string Name;
		double Duration;
		double TicksPerSecond;
		std::unordered_map<int, uint> ChannelMap; // BoneID to ChannelID
		std::vector<ChannelInfo> Channels;
	};
    
}
