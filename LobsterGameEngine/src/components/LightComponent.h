#pragma once
#include "Component.h"
#define MAX_DIRECTIONAL_LIGHTS 10
#define MAX_POINT_LIGHTS 100
#define MAX_SPOT_LIGHTS 100

namespace Lobster
{

	enum LightType : uint
	{
		DIRECTIONAL_LIGHT,
		POINT_LIGHT,
		SPOT_LIGHT
	};

	struct ubo_DirectionalLight {
		glm::vec3 direction;
		float intensity;
		glm::vec3 color;
		float padding;
	};

	struct ubo_Lights {
		ubo_DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
		int directionalLightCount;
		int pointLightCount;
		int spotLightCount;
		int padding;
	};

	class LightComponent : public Component
	{
		friend class LightLibrary;
	private:
		LightType m_type;
		glm::vec3 m_color;
		float m_intensity;
		bool b_dirty;
	public:
		LightComponent(LightType type = DIRECTIONAL_LIGHT);
		~LightComponent();
		virtual void OnAttach() override;
		virtual void OnUpdate(double deltaTime) override;
		virtual void OnImGuiRender() override;
		inline LightType GetType() const{ return m_type; }
	};

	class LightLibrary
	{
	private:
		uint m_ubo;
		ubo_DirectionalLight m_directionalLights[MAX_DIRECTIONAL_LIGHTS];
		size_t m_directionalLightCount = 0;
		static LightLibrary* s_instance;
	public:
		static void Initialize();
		static void AddLight(LightComponent* light, LightType type);
		static void RemoveLight(LightComponent* light, LightType type);
		static void SetUniforms();
	};

}