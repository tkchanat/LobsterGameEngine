#pragma once
#include "imgui/ImGuiComponent.h"

namespace Lobster {

	static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
	static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }

	struct Node;
	struct Pin {
		int ID;
		Node* Parent;
		std::string Name;
		enum class PinType {} Type;

		Pin(int id, const char* name) : ID(id), Parent(nullptr), Name(name) {}
		void Draw() const {

		}
	};

	struct Node {
		int ID;
		std::string Name;
		std::vector<Pin> Inputs;
		std::vector<Pin> Outputs;
		ImColor Color;
		enum NodeType { Default } Type;
		ImVec2 Size;

		std::string State;
		std::string SavedState;

		Node(int id, const char* name, ImColor color = ImColor(255, 255, 255)) :
			ID(id), Name(name), Color(color), Type(NodeType::Default), Size(0, 0) {}		
		void Draw() const {
			ImGui::Text(Name.c_str());
			for (const Pin& pin : Inputs) {
				pin.Draw();
			}
			for (const Pin& pin : Outputs) {
				pin.Draw();
			}
		}
	};

	class ImGuiNodeGraphEditor : public ImGuiComponent {
	private:
		std::string gameObjName;

		inline static int s_nodeId = 1;
		std::vector<Node> nodes;
	public:
		ImGuiNodeGraphEditor() {
			
		}

		~ImGuiNodeGraphEditor() {
			
		}

		void SetGameObjectName(std::string name) {
			gameObjName = name;
		}

		virtual void Show(bool* p_open) {
			ImGui::SetNextWindowSize(ImVec2(700, 600));
			ImGui::Begin("Node Graph Editor", p_open);
			ImGui::Text("// TODO");
			ImGui::End();
		}
	};

}

