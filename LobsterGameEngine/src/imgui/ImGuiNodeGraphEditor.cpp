// https://gist.github.com/spacechase0/e2ff2c4820726d62074ec0d3708d61c3

#include "pch.h"
#include <any>
#include <cmath>
#include "ImGuiNodeGraphEditor.h"

namespace Lobster {

	Node::Node(ImGuiNodeGraphEditor* parent) : parent(parent) {
		coord[0] = 0; coord[1] = 0; coord[2] = 0;
		id = std::string("Node ") + std::to_string(cnt++);
	}
	Node::Node(ImGuiNodeGraphEditor* parent, float x, float y, float z) : parent(parent) {
		coord[0] = x; coord[1] = y; coord[2] = z;
		id = std::string("Node ") + std::to_string(cnt++);
	}

	Node::~Node() {
		// clear all links connected to this node
		for (Link* link : links) {
			delete link;
		}
		links.clear();
	}

	float Node::Distance(const Node* another) {
		return glm::distance(glm::vec3(coord[0], coord[1], coord[2]), glm::vec3(another->coord[0], another->coord[1], another->coord[2]));
	}

	Link* Node::IsConnectedTo(Node* node) {
		for (Link* link : links) {
			if (link->from == node || link->to == node) return link;
		}
		return nullptr;
	}

	void Node::RemoveLink(Link* target) {
		links.erase(std::find(links.begin(), links.end(), target), links.end());
	}

	void Node::Show() {
		ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen;
		// call different CollapsingHeader base on the boolean value in attached
		if (attached ? ImGui::CollapsingHeader(id.c_str(), flag) : ImGui::CollapsingHeader(id.c_str(), &show, flag)) {
			if (attached) {
				ImGui::TextColored(ImVec4(1.f, 0, 0, 1.f), "Attached");
			}
			// coordinates in the world
			ImGui::SetNextItemWidth(200);
			char coordid[64]; sprintf(coordid, "Coordinate##coord-%s", id.c_str());
			ImGui::DragFloat3(coordid, coord, 0.1f);
			// active links
			ImGui::Columns(4, "active-link-col", false);
			for (auto it = links.begin(); it != links.end(); it++) {
				Link* link = *it;
				// show another node attached to the link
				std::string text = (link->from == this ? link->to->id.c_str() : link->from->id.c_str());
				ImGui::Text(text.c_str());
				ImGui::SameLine();
				// cross button
				std::string crossName = std::string("x##x-") + link->from->id + "-" + link->to->id + "-" + id;
				if (ImGui::Button(crossName.c_str())) {					
					delete link;
					parent->RemoveLink(link);
					it--;
				}
				ImGui::NextColumn();
			}
			ImGui::Columns(1);
			ImGui::SetNextItemWidth(300);
			std::string comboName = std::string("Target Node##combo") + id;
			if (ImGui::BeginCombo(comboName.c_str(), (targetNode ? targetNode->id.c_str() : "None"))) {
				std::string nodeTargetName = std::string("None##node-") + id;
				if (ImGui::Selectable(nodeTargetName.c_str())) {
					targetNode = nullptr;
				}
				for (Node* node : parent->GetNodes()) {
					if (node == this) continue; // not to display self
					if (this->IsConnectedTo(node)) continue; // not to display nodes already connected
					std::string noneName = node->id + "##" + id + "-select-" + node->id;
					if (ImGui::Selectable(noneName.c_str())) {
						targetNode = node;
					}
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			// unable to click button before a valid node is selected
			bool addlink_disabled = false;
			if (!targetNode) {
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				addlink_disabled = true;
			}
			std::string addLinkName = std::string("Add Link##addlink-") + id;
			if (ImGui::Button(addLinkName.c_str())) {
				Link* link = new Link(this, targetNode);
				// attach link to both nodes
				links.push_back(link);
				targetNode->links.push_back(link);
				// append into graph editor for tracking
				parent->GetLinks().push_back(link);
				// set target mpde back to none
				targetNode = nullptr;
			}
			if (addlink_disabled) {
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
		ImGui::Separator();
	}

	Link::Link(Node* from, Node* to) : from(from), to(to) {
		distance = to - from;
	}

	Link::~Link() {
		from->RemoveLink(this);
		to->RemoveLink(this);
	}

	ImGuiNodeGraphEditor::ImGuiNodeGraphEditor() {
		if (s_editor) throw std::exception("A NodeGraphEditor is already created.");
		s_editor = this;				
	}

	ImGuiNodeGraphEditor::~ImGuiNodeGraphEditor() {
		for (Node* node : nodes) delete node;
		for (Link* path : links) delete path;
		nodes.clear();
		links.clear();
		s_editor = nullptr;
	}

	void ImGuiNodeGraphEditor::SetGameObjectName(std::string name) {
		gameObjName = name;
	}

	void ImGuiNodeGraphEditor::ResetNodeState() {
		for (Node* node : nodes) {
			node->f = FLT_MAX;
			node->g = FLT_MAX;
		}
	}

	void ImGuiNodeGraphEditor::RemoveLink(Link* link) {
		links.erase(std::remove(links.begin(), links.end(), link));
	}

	void ImGuiNodeGraphEditor::Show(bool* p_open) {
		ImGui::Begin("Editor", p_open);
		// draw add nodes
		for (auto it = nodes.begin(); it != nodes.end(); it++) {
			Node* node = *it;
			node->Show();
			// delete the node if the user cross the node
			if (!node->show) {				
				delete node;
				nodes.erase(it--);
			}
		}
		// allow add node
		ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.25);
		if (ImGui::Button("Add Node", ImVec2(ImGui::GetWindowSize().x * 0.5, 24))) {
			Node* newNode = new Node(this);
			nodes.push_back(newNode);
		}
		ImGui::End();
	}


}