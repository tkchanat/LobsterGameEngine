#pragma once

#include "Component.h"
#include "objects/Transform.h"
#include "graphics/2D/GameUI.h"

namespace Lobster
{

	class FrameBuffer;
    
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
        glm::mat4 m_viewMatrix;
        glm::mat4 m_projectionMatrix;
		glm::mat4 m_orthoMatrix;
		FrameBuffer* m_frameBuffer;
		bool b_uiEditor = false;
		GameUI* gameUI = nullptr;
		
		static CameraComponent* s_activeCamera;

    public:
        CameraComponent();
        virtual ~CameraComponent();
		void ResizeProjection(float width, float height);
		void DrawUI();
		virtual void OnAttach() override;
        virtual void OnUpdate(double deltaTime) override;
		virtual void OnImGuiRender() override;
		virtual void OnBegin() override;
		virtual void Serialize(cereal::BinaryOutputArchive& oarchive) override;
		virtual void Deserialize(cereal::BinaryInputArchive& iarchive) override;
		glm::mat4 GetViewMatrix() const;
        inline glm::mat4 GetProjectionMatrix() const { return m_projectionMatrix; }
		inline glm::mat4 GetOrthoMatrix() const { return m_orthoMatrix; }
		inline glm::vec3 GetPosition() const { return Component::transform->WorldPosition; }
		inline FrameBuffer* GetFrameBuffer() { return m_frameBuffer; }
		static inline CameraComponent* GetActiveCamera() { return s_activeCamera; }
	private:
		friend class cereal::access;
		template <class Archive>
		void save(Archive & ar) const
		{
			ar(m_fieldOfView);
			ar(m_nearPlane, m_farPlane);
			//gameUI->Serialize();
		}
		template <class Archive>
		void load(Archive & ar)
		{
			ar(m_fieldOfView);
			ar(m_nearPlane, m_farPlane);
			//gameUI->Deserialize();
		}
    };
    
}
