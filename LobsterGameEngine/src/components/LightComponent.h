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
		int m_isChanging = -1;		//	Used for undo system. 0 = color, 1 = intensity.
		LightType m_prevType;		//	Used for undo system.
		glm::vec3 m_prevColor;		//	Used for undo system.
		float m_prevIntensity;		//	Used for undo system.

		LightType m_type;
		glm::vec3 m_color;
		float m_intensity;
		bool b_dirty;
	public:
		LightComponent(LightType type = DIRECTIONAL_LIGHT);
		virtual ~LightComponent();

		virtual void VirtualCreate() override;
		virtual void VirtualDelete() override;

		virtual void OnAttach() override;
		virtual void OnUpdate(double deltaTime) override;
		virtual void OnImGuiRender() override;
        virtual void Serialize(cereal::BinaryOutputArchive& oarchive) override;
        virtual void Deserialize(cereal::BinaryInputArchive& iarchive) override;
		inline LightType GetType() const{ return m_type; }
    private:
		void ChangeLightType();

        friend class cereal::access;
        template <class Archive>
        void save(Archive & ar) const
        {
            ar(m_type);
            ar(m_color);
			ar(m_intensity);
        }
        template <class Archive>
        void load(Archive & ar)
        {
            ar(reinterpret_cast<LightType>(m_type));
            ar(m_color);
			ar(m_intensity);
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
