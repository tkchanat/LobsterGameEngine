#pragma once
#include "imgui/ImGuiComponent.h"

namespace Lobster {

	// This class supposes to be a UI editor for node graph
	// however, it is too cumbersome and common libraries found on Internet does not help a lot
	// therefore, I temporarily set it as a simple ImGui-based nodes setter
	// and we can go back to this if this engine is going to be further developed
	// -AG	
	struct Link;
	class ImGuiNodeGraphEditor;

	struct Node {		
		inline static int cnt = 0;
		// internal members
		Node* targetNode = nullptr;
		// public members		
		ImGuiNodeGraphEditor* parent = nullptr;
		std::string id;		
		bool show = true;
		bool attached = false;
		float coord[3];
		std::vector<Link*> links;
		// states
		float f = FLT_MAX;
		float g = FLT_MAX;

		Node(ImGuiNodeGraphEditor* parent);
		Node(ImGuiNodeGraphEditor* parent, float x, float y, float z);
		~Node();
		float Distance(const Node* another);
		Link* IsConnectedTo(Node* node);
		void RemoveLink(Link* target);
		void Show();
	};

	struct Link {
		Node* from = nullptr;
		Node* to = nullptr;
		float distance;

		Link(Node* from, Node* to);
		~Link();
	};

	class ImGuiNodeGraphEditor : public ImGuiComponent {
	private:
		inline static ImGuiNodeGraphEditor* s_editor = nullptr;

		std::string gameObjName;
		std::vector<Node*> nodes;
		std::vector<Link*> links;

		const std::string chainImgPath = "textures/ui/chain.png";
	public:		
		ImGuiNodeGraphEditor();
		~ImGuiNodeGraphEditor();
		inline static ImGuiNodeGraphEditor* GetInstance() { return s_editor; }
		void SetGameObjectName(std::string name);
		void ResetNodeState(); // to reset the dirty state of all nodes
		inline std::vector<Node*>& GetNodes() { return nodes; }
		inline std::vector<Link*>& GetLinks() { return links; }
		void RemoveLink(Link* link);

		virtual void Show(bool* p_open);

	};


}

