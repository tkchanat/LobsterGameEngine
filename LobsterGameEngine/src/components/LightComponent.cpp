#include "pch.h"
#include "LightComponent.h"
#include "imgui/ImGuiScene.h"
#include "objects/Transform.h"
#include "physics/Rigidbody.h"

namespace Lobster
{

	LightComponent::LightComponent(LightType type) :
		Component(LIGHT_COMPONENT),
		m_type(DIRECTIONAL_LIGHT),
		m_color(glm::vec3(1)),
		m_intensity(1),
		b_dirty(false)
	{
	}

	LightComponent::~LightComponent()
	{
		LightLibrary::RemoveLight(this, m_type);
	}

	void LightComponent::OnAttach()
	{
		this->transform->WorldPosition = glm::vec3(0, 2, 3);
		LightLibrary::AddLight(this, GetType());

		PhysicsComponent* physics = new Rigidbody();
		physics->SetEnabled(false);
		gameObject->AddComponent(physics);
	}

	void LightComponent::OnUpdate(double deltaTime)
	{
#ifdef LOBSTER_BUILD_DEBUG
		GizmosCommand command;
		command.texture = "textures/ui/light.png";
		command.position = transform->WorldPosition;
		command.source = gameObject;
		ImGuiScene::SubmitGizmos(command);
#endif
	}

	void LightComponent::OnImGuiRender()
	{
		std::string headerLabel = fmt::format("Light {}", b_dirty ? "*" : "");
		if (ImGui::CollapsingHeader(headerLabel.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			// Light type
			const char* types[] = { "Directional", "Point", "Spot" };
			LightType prev_type = m_type;
			ImGui::Combo("Type", (int*)&m_type, types, IM_ARRAYSIZE(types));
			if (prev_type != m_type) {
				LightLibrary::RemoveLight(this, prev_type);
				LightLibrary::AddLight(this, m_type);
				b_dirty = true;
			}

			// Light color
			ImGui::ColorEdit3("Color", glm::value_ptr(m_color), 0);

			// Light intensity
			ImGui::SliderFloat("Intensity", &m_intensity, 0.f, 1.f);
		}
	}

    void LightComponent::Serialize(cereal::JSONOutputArchive& oarchive)
    {
        //LOG("Serializing LightComponent");
        oarchive(*this);
    }

    void LightComponent::Deserialize(cereal::JSONInputArchive& iarchive)
    {
        //LOG("Deserializing LightComponent");
        try {
            iarchive(*this);
        }
        catch (std::exception e) {
            LOG("Deserializing LightComponent failed. Reason: {}", e.what());
        }
    }

	// =======================================================
	// LightLibrary
	// =======================================================
	LightLibrary* LightLibrary::s_instance = nullptr;

	void LightLibrary::Initialize()
	{
		if (s_instance != nullptr)
		{
			throw std::runtime_error("LightLibrary already existed!");
		}
		s_instance = new LightLibrary();

		// directional lights
		glGenBuffers(1, &s_instance->m_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, s_instance->m_ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(ubo_Lights), NULL, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, s_instance->m_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void LightLibrary::AddLight(LightComponent * light, LightType type)
	{
		switch (type)
		{
		case Lobster::DIRECTIONAL_LIGHT:
			if (s_instance->m_directionalLightCount + 1 >= MAX_DIRECTIONAL_LIGHTS) {
				WARN("MAX_DIRECTIONAL_LIGHTS exceeded, this new light will be ignored!");
				return;
			}
			ubo_DirectionalLight ubo;
			ubo.direction = light->transform->WorldPosition;
			ubo.intensity = light->m_intensity;
			ubo.color = light->m_color;
			s_instance->m_directionalLights[s_instance->m_directionalLightCount++] = ubo;
			break;
		case Lobster::POINT_LIGHT:
			break;
		case Lobster::SPOT_LIGHT:
			break;
		default:
			break;
		}
	}

	void LightLibrary::RemoveLight(LightComponent * light, LightType type)
	{
		switch (type)
		{
		case Lobster::DIRECTIONAL_LIGHT:
			if (s_instance->m_directionalLightCount - 1 < 0) return;
			s_instance->m_directionalLightCount--;
			break;
		case Lobster::POINT_LIGHT:
			break;
		case Lobster::SPOT_LIGHT:
			break;
		default:
			break;
		}
	}

	void LightLibrary::SetUniforms()
	{
		glBindBuffer(GL_UNIFORM_BUFFER, s_instance->m_ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ubo_DirectionalLight) * MAX_DIRECTIONAL_LIGHTS, s_instance->m_directionalLights);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(ubo_DirectionalLight) * MAX_DIRECTIONAL_LIGHTS, sizeof(int), &s_instance->m_directionalLightCount);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

}
