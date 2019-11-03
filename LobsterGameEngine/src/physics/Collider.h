#pragma once
#include "objects/Transform.h"

namespace Lobster {
	class Collider {
	public:
		Collider(bool draw = true) : m_draw(draw) {}
		virtual void OnUpdate(Transform* t) = 0;
		virtual void OnImGuiRender() {
			ImGui::Checkbox("Visualize Collider", &m_draw);
		}
		virtual bool Intersects(Collider* collider) = 0;

	protected:
		virtual void Draw() = 0;
		bool m_draw = true;
	};
}