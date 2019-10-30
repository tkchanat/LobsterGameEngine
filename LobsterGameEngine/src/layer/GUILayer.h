#pragma once
#include "Layer.h"

#include "events/KeyboardEvent.h"

namespace Lobster
{

	class GUILayer : public Layer
	{
	public:
		GUILayer();
		~GUILayer();
		void Begin();
		void End();
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(double deltaTime) override;
	};

}
