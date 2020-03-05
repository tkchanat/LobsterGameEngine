#pragma once
#include "components/Component.h"
#include "imgui/ImGuiNodeGraphEditor.h"

namespace Lobster {

	class PathFinder : public Component {
	private:
		bool m_enabled = true;
		Node* at = nullptr;
		Node* dest = nullptr;
		enum PathFinderType { None, Astar } type = None;

		std::vector<Node*> moveSequence;
	public:
		PathFinder();
		~PathFinder();		

		void AttachNode(Node* node); // move the object towards the node
		bool Find(Node* destination); // calculate the shortest path and store the sequence in moveSequence

		virtual void OnUpdate(double deltaTime) override;
		virtual void OnImGuiRender() override;

		virtual void Serialize(cereal::JSONOutputArchive& oarchive) override {};
		virtual void Deserialize(cereal::JSONInputArchive& oarchive) override {};
	};


}

