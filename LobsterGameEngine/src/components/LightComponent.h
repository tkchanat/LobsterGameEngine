#pragma once
#include "Component.h"

namespace Lobster
{

	enum LightType : uint
	{
		DIRECTIONAL_LIGHT,
		POINT_LIGHT,
		SPOT_LIGHT
	};

	class LightComponent : public Component
	{
	public:
		LightComponent(LightType type = DIRECTIONAL_LIGHT);
		~LightComponent();
		virtual void OnUpdate(double deltaTime) override;
		virtual void OnImGuiRender() override;
	};

}