#pragma once
#include "Component.h"
#include "graphics/VertexArray.h"
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
    public:
        MeshComponent(const char* meshPath, const char* materialPath = "materials/default.mat");
        MeshComponent(VertexArray* mesh, const char* materialPath = "materials/default.mat");
		MeshComponent(VertexArray* mesh, glm::vec3 min, glm::vec3 max, const char* materialPath = "materials/default.mat");
        virtual ~MeshComponent() override;
		virtual void OnAttach() override;
		virtual void OnUpdate(double deltaTime) override;
		virtual void OnImGuiRender() override;
		inline std::pair<glm::vec3, glm::vec3> GetBound() const { return m_meshInfo.Bound; }
	private:
		void UpdateBoneTransforms();
		void ReadNodeHierarchy(const BoneNode& node, const glm::mat4& parentTransform);
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
		double Duration;
		double TicksPerSecond;
		std::vector<ChannelInfo> Channels;
	};
    
}
