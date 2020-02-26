#pragma once
#include "components/Component.h"
#include "imgui/ImGuiNodeGraphEditor.h"

namespace Lobster {

	class AIComponent : public Component {
	private:
		bool m_showNodeEditor = false;
		ImGuiNodeGraphEditor* nodeEditor;
	public:
		AIComponent();
		~AIComponent();

		void SetGameObjectName(std::string name);
		virtual void OnUpdate(double deltaTime) override {}
		virtual void OnImGuiRender() override;

		virtual void Serialize(cereal::JSONOutputArchive& oarchive) override {};
		virtual void Deserialize(cereal::JSONInputArchive& oarchive) override {};
	};


}

