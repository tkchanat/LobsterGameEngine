#pragma once

#include "Component.h"
#include "objects/Transform.h"

namespace Lobster
{

	class FrameBuffer;
    
	//	This enum class is for configuring the projection type when constructing a CameraComponent class.
	//	It is only visible if you include CameraComponent.h or ComponentCollection.h
    enum class ProjectionType : uint
    {
        PERSPECTIVE,
        ORTHOGONAL
    };
    
	//	This class is for attaching a static viewing camera to a game object.
	//	User can customize the field of view (FOV), near and far plane, as well as the projection type of the camera (TODO: setter and getter functions).
	//	Note:	At the moment, we only allow one scene to have at most one main camera.
	//			Though multiple camera in the scene would still allow, yet the first camera added to the scene will be considered as main camera.
	//			While other newly added camera would be considered as useless, because their view will not be rendered.
    class CameraComponent : public Component
    {
    private:
        float m_fieldOfView;
        float m_nearPlane;
        float m_farPlane;
        ProjectionType m_type;
        glm::mat4 m_viewMatrix;
        glm::mat4 m_projectionMatrix;
		FrameBuffer* m_frameBuffer;
		static CameraComponent* s_activeCamera;
    public:
        CameraComponent(ProjectionType type);
        ~CameraComponent();
		void ResizeProjection(float aspectRatio);
		virtual void OnAttach() override;
        virtual void OnUpdate(double deltaTime) override;
		virtual void OnImGuiRender() override;
		virtual void Serialize(cereal::JSONOutputArchive& oarchive) override;
		virtual void Deserialize(cereal::JSONInputArchive& iarchive) override;
		glm::mat4 GetViewMatrix() const;
        inline glm::mat4 GetProjectionMatrix() const { return m_projectionMatrix; }
		inline glm::vec3 GetPosition() const { return Component::transform->WorldPosition; }
		inline FrameBuffer* GetFrameBuffer() { return m_frameBuffer; }
		static inline CameraComponent* GetActiveCamera() { return s_activeCamera; }
	private:
		friend class cereal::access;
		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(m_fieldOfView);
			ar(m_nearPlane);
			ar(m_farPlane);
			//ar(m_type);
			//ar(m_viewMatrix);
			//ar(m_projectionMatrix);
		}
    };
    
}
