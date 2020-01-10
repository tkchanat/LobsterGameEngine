#pragma once
#include "Component.h"

namespace Lobster
{

	enum LightType : uint
	{
		DIRECTIONAL_LIGHT,
		POINT_LIGHT
	};

	struct ubo_DirectionalLight {
		glm::vec3 direction;
		float intensity;
		glm::vec3 color;
		float padding;
	};

	struct ubo_PointLight {
		glm::vec3 position;
		float attenuation;
		glm::vec3 color;
		float padding;
	};

	struct ubo_Lights {
		ubo_DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
		ubo_PointLight pointLights[MAX_POINT_LIGHTS];
		int directionalLightCount;
		int pointLightCount;
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
        virtual void Serialize(cereal::JSONOutputArchive& oarchive) override;
        virtual void Deserialize(cereal::JSONInputArchive& iarchive) override;
		inline LightType GetType() const{ return m_type; }
    private:
        friend class cereal::access;
        template <class Archive>
        void save(Archive & ar) const
        {
            ar(m_type);
            ar(m_color);
        }
        template <class Archive>
        void load(Archive & ar)
        {
            ar(m_type);
            ar(m_color);
        }
	};

	class LightLibrary
	{
	private:
		uint m_ubo;
		std::list<LightComponent*> m_directionalLights;
		std::list<LightComponent*> m_pointLights;
		static LightLibrary* s_instance;
	public:
		static void Initialize();
		static void AddLight(LightComponent* light, LightType type);
		static void RemoveLight(LightComponent* light, LightType type);
		static void SetUniforms();
	};

}
