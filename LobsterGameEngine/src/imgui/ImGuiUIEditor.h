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
			for (int i = 0; i <= winSize.y; i += 20) {
				int wi = winPos.y + i;
				float thickness = ((i % 100 == 0) ? 1.5f : 1.0f);
				ImColor gridColor = ((i % 100 == 0) ? ImColor(1.0f, 1.0f, 1.0f, 0.5f) : ImColor(1.0f, 1.0f, 1.0f, 0.2f));
				drawList->AddLine(ImVec2(winPos.x, wi), ImVec2(winPos.x + winSize.x, wi), gridColor, thickness);
			}
			// vertical lines
			for (int i = 0; i <= winSize.x; i += 20) {
				int wi = winPos.x + i;
				float thickness = ((i % 100 == 0) ? 1.5f : 1.0f);
				ImColor gridColor = ((i % 100 == 0) ? ImColor(1.0f, 1.0f, 1.0f, 0.5f) : ImColor(1.0f, 1.0f, 1.0f, 0.2f));
				drawList->AddLine(ImVec2(wi, winPos.y), ImVec2(wi, winPos.y + winSize.y), gridColor, thickness);
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
							ui->AddSprite(new ImageSprite2D(name.c_str(), windowSize.x, windowSize.y, x, y));
							ImGui::CloseCurrentPopup();
						}
					}
					ImGui::EndMenu();
				}
				if (ImGui::MenuItem("Add Text")) {
					ui->AddSprite(new TextSprite2D("Enter Your Text Here.", "TimesNewRoman.ttf",
						windowSize.x, windowSize.y, x, y));
					ImGui::CloseCurrentPopup();
				}
				ImGui::MenuItem("Dynamic Sprites");
				ImGui::EndPopup();
			}
		}

		virtual void Show(bool* p_open) override {
			static Config config;
			ImGui::SetNextWindowContentSize(ImVec2(config.width, config.height));
			ImGui::Begin("UI Editor", p_open, ImGuiWindowFlags_HorizontalScrollbar);
			// recalculate the position information for this window
			static Config winConfig;
			// only display the ui editor when GameUI exists
			if (ui) {				
				if (ImGui::BeginChild("Editor 2D"), true) {
					// add the background grids			
					DrawBackgroundGrid();
					// display all sprites
					int i = 0;
					for (Sprite2D* sprite : ui->GetSpriteList()) {
						// calculate the x, y, width, height
						float x = winConfig.width * sprite->x;
						float y = winConfig.height * sprite->y;
						ImGui::SetCursorPos(ImVec2(x, y));
						// display sprite
						sprite->OnImGuiRender();								
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
							sprite->ImGuiMenu(ui, ImVec2(winConfig.width, winConfig.height));
							ImGui::EndPopup();
						}
						i++;
					}
					// left mouse down on item
					if (ImGui::IsMouseDown(0) && selectedSprite) {
						// move sprite to a new position
						float startPosX = io.MousePos.x - pmx;
						float startPosY = io.MousePos.y - pmy;
						selectedSprite->x = startPosX / winConfig.width;
						selectedSprite->y = startPosY / winConfig.height;
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