#include "pch.h"
#include "AIComponent.h"
#include "objects/GameObject.h"
#include "imgui/ImGuiNodeGraphEditor.h"

namespace Lobster {

	PathFinder::PathFinder() : Component(AI_COMPONENT) {
		
	}

	PathFinder::~PathFinder() {
		// detach all nodes
		if (at) at->attached = false;
		if (dest) dest->attached = false;
	}

	void PathFinder::AttachNode(Node* node) {
		this->at = node;
		transform->WorldPosition = glm::vec3(node->coord[0], node->coord[1], node->coord[2]);
	}

	bool PathFinder::Find(Node* destination = nullptr) {
		const int MAX_ITER = 1000;
		if (destination) dest = destination;
		if (at == destination) return true;
		auto compare = [](Node* left, Node* right) -> bool { return left->f < right->f; };
		auto compareLink = [](Link* a, Link* b) -> bool { return a->from->Distance(a->to) < b->from->Distance(b->to); };
		std::priority_queue<Node*, std::vector<Node*>, decltype(compare)> queue(compare);
		std::map<Node*, bool> bmap; // to keep track of the nodes in queue
		queue.push(at);
		bmap[at] = true;
		at->g = 0;
		at->f = at->Distance(dest);
		int iter = 0;
		while (!queue.empty()) {
			Node* current = queue.top();
			if (current == dest) {
				return true;
			}
			queue.pop();
			bmap[current] = false;
			std::sort(current->links.begin(), current->links.end(), compareLink);
			for (Link* link : current->links) {
				Node* neighbor = (link->from == current ? link->to : link->from);
				// accumulate the distance to neighbor
				float gTemp = current->g + current->Distance(neighbor);
				if (gTemp < neighbor->g) {
					pathsWalked[neighbor] = current;
					neighbor->g = gTemp;
					// f = g(x) + h(x), h here we use the linear distance between two nodes in 3D space
					neighbor->f = neighbor->g + neighbor->Distance(dest);
					if (!bmap[neighbor]) {
						queue.push(neighbor);
						bmap[neighbor] = true;
					}					
				}
			}
			iter++;
			if (iter > MAX_ITER) {
				throw std::exception("Exceed 1000 iterations, stopping.");
			}
		}
		return false;
	}

	void PathFinder::OnUpdate(double deltaTime) {
		if (Application::GetMode() != ApplicationMode::GAME) return;
	}

	void PathFinder::OnImGuiRender() {
		if (ImGui::CollapsingHeader("Path Finder", &m_show, ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Checkbox("Enable##ai-enable", &m_enabled);
			if (ImGui::BeginCombo("Algorithm", (type == PathFinderType::None ? "None" : "A*"))) {
				if (ImGui::Selectable("None")) {
					type = PathFinderType::None;
				}
				if (ImGui::Selectable("A*")) {
					type = PathFinderType::Astar;
				}
				ImGui::EndCombo();
			}
			// select the current node
			if (ImGui::BeginCombo("Current Node", (at ? at->id.c_str() : "None"))) {
				if (ImGui::Selectable("None")) {
					if (at) at->attached = false;
					at = nullptr;
				}
				for (Node* node : ImGuiNodeGraphEditor::GetInstance()->GetNodes()) {
					if (ImGui::Selectable(node->id.c_str())) {
						if (at) at->attached = false;
						at = node;
						at->attached = true;
						// also update another slot
						if (dest) dest->attached = true;
					}
				}
				ImGui::EndCombo();
			}
			// select the destination
			if (ImGui::BeginCombo("Destination", (dest ? dest->id.c_str() : "None"))) {
				if (ImGui::Selectable("None")) {
					if (dest) dest->attached = false;
					dest = nullptr;
				}
				for (Node* node : ImGuiNodeGraphEditor::GetInstance()->GetNodes()) {
					if (ImGui::Selectable(node->id.c_str())) {
						if (dest) dest->attached = false;
						dest = node;
						dest->attached = true;
						// also update another slot
						if (at) at->attached = true;
					}
				}
				ImGui::EndCombo();
			}
			// Testing area, TODO modify
			if (!(at && dest && type != PathFinderType::None)) {
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}
			if (ImGui::Button("Test")) {
				bool find = Find();
				std::stringstream ss;
				for (Node* current = dest; current; current = pathsWalked[current]) {
					ss << current->id << " <- ";
				}
				pathsWalked.clear();
				LOG(find ? "Connected" : "Not Connected");
				LOG(ss.str().c_str());
				// important: reset node status every time after pathfind
				ImGuiNodeGraphEditor::GetInstance()->ResetNodeState();
			}
			if (!(at && dest && type != PathFinderType::None)) {
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
			}
		}
	}

}