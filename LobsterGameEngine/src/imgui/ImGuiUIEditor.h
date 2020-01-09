#pragma once
#include "imgui/ImGuiComponent.h"
#include "imgui/ImGuiAssets.h"
#include "graphics/2D/GameUI.h"

namespace Lobster {

	// There is only one ImGuiUIEditor, when different GameUI pointers loaded to edit
	class ImGuiUIEditor : public ImGuiComponent {
	private:
		GameUI* ui = nullptr;	// pointer to an attached GameUI, never delete it here	
		ImGuiIO& io = ImGui::GetIO();
		Sprite2D* selectedSprite = nullptr;
		float pmx, pmy;			// to save the intial position of mouse on the sprite
		float xOnRightClick, yOnRightClick; // to save the position of mouse on right click
		ImColor gridColor = ImColor(1.0f, 1.0f, 1.0f, 0.2f);
	public:
		ImGuiUIEditor() {
		}

		void SetUI(GameUI* ui) {
			// TODO save/load??
			this->ui = ui;
		}

		void DrawBackgroundGrid() {
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 winPos = ImGui::GetWindowPos();
			ImVec2 winSize = ImGui::GetWindowSize();
			// horizontal lines
			for (int i = winPos.y; i < winSize.y + winPos.y; i += 32) {
				drawList->AddLine(ImVec2(winPos.x, i), ImVec2(winPos.x + winSize.x, i), gridColor, 0.5);
			}
			// vertical lines
			for (int i = winPos.x; i < winSize.x + winPos.x; i += 32) {
				drawList->AddLine(ImVec2(i, winPos.y), ImVec2(i, winPos.y + winSize.y), gridColor, 0.5);
			}
		}

		void ShowPopupContext() {
			ImVec2 windowPos = ImGui::GetWindowPos();
			ImVec2 windowSize = ImGui::GetWindowSize();
			// On right click
			if (ImGui::IsMouseClicked(1)) {
				// save the mouse position for sprite generation
				xOnRightClick = io.MousePos.x;
				yOnRightClick = io.MousePos.y;
			}
			// right clicking the background			
			if (ImGui::BeginPopupContextWindow((const char*)0, 1, false)) {	
				float x = (xOnRightClick - windowPos.x) / windowSize.x;
				float y = (yOnRightClick - windowPos.y) / windowSize.y;
				if (ImGui::BeginMenu("Add Image")) {					
					for (std::string& name : ImGuiAssets::ListResources(PATH_SPRITES)) {
						if (ImGui::MenuItem(name.c_str())) {							
							ui->AddSprite(new Sprite2D(name.c_str(), windowSize.x, windowSize.y, x, y));
							ImGui::CloseCurrentPopup();
						}
					}
					ImGui::EndMenu();
				}
				if (ImGui::MenuItem("Add Text")) {

				}
				ImGui::MenuItem("Dynamic Sprites");
				ImGui::EndPopup();
			}
		}

		virtual void Show(bool* p_open) override {
			ImGui::Begin("UI Editor", p_open);			
			// recalculate the position information for this window
			ImVec2 windowPos = ImGui::GetWindowPos();
			ImVec2 windowSize = ImGui::GetWindowSize();			
			// only display the ui editor when GameUI exists
			if (ui) {				
				if (ImGui::BeginChild("Editor 2D"), true) {
					// add the background grids			
					DrawBackgroundGrid();
					// display all sprites
					int i = 0;
					for (Sprite2D* sprite : ui->GetSpriteList()) {
						// calculate the x, y, width, height
						float x = windowPos.x + windowSize.x * sprite->x;
						float y = windowPos.y + windowSize.y * sprite->y;
						float width = sprite->w;
						float height = sprite->h;
						ImGui::SetCursorScreenPos(ImVec2(x, y));
						// display sprite						
						ImGui::ImageButton(sprite->GetTexID(), ImVec2(width, height));			
						// item on first mouse down => save sprite, pmx and pmy
						if (ImGui::IsItemActivated() && ImGui::IsItemHovered()) {
							selectedSprite = sprite;
							pmx = io.MousePos.x - x;
							pmy = io.MousePos.y - y;
						}
						// set right click on item
						char c[64]; 
						sprintf(c, "sprite-%2d", i); // name each popup with distinct name
						if (ImGui::BeginPopupContextItem(c)) {
							sprite->ImGuiMenu(ui, windowSize);							
							ImGui::EndPopup();
						}
						i++;
					}
					// left mouse down on item
					if (ImGui::IsMouseDown(0) && selectedSprite) {
						// move sprite to a new position
						float startPosX = io.MousePos.x - pmx;
						float startPosY = io.MousePos.y - pmy;
						selectedSprite->x = (startPosX - windowPos.x) / windowSize.x;
						selectedSprite->y = (startPosY - windowPos.y) / windowSize.y;
						selectedSprite->Clip();
					}
					else if (!ImGui::IsMouseDown(0) && !ImGui::IsMouseDown(1)) {
						selectedSprite = nullptr;
					}
					// right click on background
					ShowPopupContext();
					ImGui::EndChild();
				}				
			}
			ImGui::End();
		}
	};

}