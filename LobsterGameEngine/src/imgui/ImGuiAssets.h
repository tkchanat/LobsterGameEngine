#pragma once
#include "ImGuiComponent.h"
#include "layer/EditorLayer.h"

#include "ImGuiFileBrowser.h"
#include "audio/AudioSystem.h"

namespace Lobster
{
	class ImGuiAssets : public ImGuiComponent
	{
	private:		
		int itemSelected = -1;
		int audioSelected = -1;
		AudioClip* audioPlaying = nullptr;
		std::string subdirSelected = "meshes"; // by default
		std::string pathSelected;
		ImGui::FileBrowser fileDialog;
		Scene* scene;
	public:
		ImGuiAssets(Scene* scene) : ImGuiComponent(), scene(scene) {}
		void ConfirmDeleteDialog() {
			ImGui::SetNextWindowSize(ImVec2(400, 120));
			if (ImGui::BeginPopupModal("Confirm Delete"))
			{
				ImGui::BeginChild("Text", ImVec2(0, 60));
				{
					ImGui::Text("Are you sure to delete \"%s\"?", pathSelected.c_str());
					ImGui::Text("This action cannot be undone.");
				}
				ImGui::EndChild();
				ImGui::SetCursorPosX(110);
				if (ImGui::Button("Delete")) {
					FileSystem::GetInstance()->removeResource(pathSelected);
					itemSelected = -1;
					pathSelected.clear();
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				ImGui::SetCursorPosX(240);
				if (ImGui::Button("Cancel"))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}
		}

		void NewNameDialog(Scene* scene, std::string path) {
			static char rename[128] = "";
			static bool nothing = false;			
			ImGui::SetNextWindowSize(ImVec2(400, 160));
			if (ImGui::BeginPopupModal("Name Game Object")) {
				ImGui::Text("Input a name:");
				ImGui::InputText("", rename, IM_ARRAYSIZE(rename), ImGuiInputTextFlags_CallbackAlways, \
					[](ImGuiInputTextCallbackData* data) -> int {
					GameObject* obj = (GameObject*)data->UserData;
					strcpy(rename, data->Buf);
					return 0;
				}, this);
				// Prompt error message (if any)
				if (nothing) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255, 0, 0, 255));
					ImGui::Text("The object name cannot be empty.");
					ImGui::PopStyleColor();
				}
				// Confirm and cancel
				ImGui::SetCursorPos(ImVec2(100, 120));
				if (ImGui::Button(" OK ")) {
					if (!strlen(rename)) {
						nothing = true;
					}
					else {
						GameObject* renamedGO = new GameObject(rename);
						renamedGO->AddComponent(new MeshComponent(path.c_str()));
						scene->AddGameObject(renamedGO);
						rename[0] = '\0';
						nothing = false;
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::SameLine();
				ImGui::SetCursorPosX(200);
				if (ImGui::Button("Cancel")) {
					nothing = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		void DisplayAudioPanel() {
			if (ImGui::Button("Load Selected")) {
				if (itemSelected != -1)
					AudioSystem::AddAudioClip(FileSystem::Path(pathSelected).c_str());
			}
			ImGui::Text("Loaded Audio Files:");
			ImGui::BeginChild("Loaded Audio", ImVec2(0, -25), true);
			{
				int i = 0;
				for (AudioClip* audioClip : AudioSystem::GetAudioList()) {
					std::string displayName = audioClip->GetName();
					if (audioClip == audioPlaying) displayName = "[PLAYING] " + displayName;
					if (ImGui::Selectable(displayName.c_str(), audioSelected == i)) {
						audioSelected = i;
					}
					i++;
				}
			}
			ImGui::EndChild();

			if (audioSelected == -1) {
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}
			if (audioPlaying) {
				if (ImGui::Button("Stop")) {
					audioPlaying->Stop();
					audioPlaying = nullptr;
				}
			}
			else {
				if (ImGui::Button("Play")) {
					AudioClip* ac = AudioSystem::GetAudioList()[audioSelected];
					audioPlaying = ac;
					ThreadPool::Enqueue([ac]() {
						ac->Play();
					});
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Unload")) {
				AudioClip* ac = AudioSystem::GetAudioList()[audioSelected];
				AudioSystem::RemoveAudioClip(ac);
			}
			if (audioSelected == -1) {
				ImGui::PopStyleVar();
				ImGui::PopItemFlag();				
			}
		}

		virtual void Show(bool* p_open) override
		{
			if (!ImGui::Begin("Assets", nullptr)) {
				ImGui::End();
				return;
			}

			if (ImGui::BeginMenuBar()) {
				if (ImGui::BeginMenu("Menu"))
				{
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			// ============ Folder Selection ============
			ImGui::PushItemWidth(120);
			if (ImGui::BeginCombo("Resource Type", subdirSelected.c_str()))
			{
				std::map structure = FileSystem::GetDirectoryStructure();
				for (const auto& iter : structure)
				{
					if (ImGui::Selectable(iter.first.c_str(), !subdirSelected.compare(iter.first))) {
						itemSelected = audioSelected = -1;
						subdirSelected = iter.first;
					}
				}
				ImGui::EndCombo();
			}

			// ============ control button group ============
			// File browser to choose files to import
			if (ImGui::Button("Import")) {
				fileDialog.Open();
			}
			fileDialog.Display();
			if (fileDialog.HasSelected())
			{
				LOG(fileDialog.GetSelected().string());
				if (subdirSelected == "audio")
					FileSystem::GetInstance()->addResource(fileDialog.GetSelected().string());
				fileDialog.ClearSelected();
			}

			// The "Add" and "Delete" function in mesh
			if (subdirSelected == "meshes" && itemSelected != -1) {				
				ImGui::SameLine();
				std::string path = FileSystem::GetInstance()->Path(pathSelected);				
				if (ImGui::Button("Add")) {
					ImGui::OpenPopup("Name Game Object");
				}
				NewNameDialog(scene, path);
				ImGui::SameLine();
				if (ImGui::Button("Delete")) {
					ImGui::OpenPopup("Confirm Delete");
				}
				ConfirmDeleteDialog();
			}

			// The region for displaying files that included as asset
			int height = (subdirSelected == "audio" ? 300 : 0);
			ImGui::BeginChild("Files", ImVec2(0, height), true);
			{
				// Note (AG): I am thinking whether to change it into the way of audio or not
				int i = 0;
				fs::path subdir = FileSystem::GetCurrentWorkingDirectory() / fs::path(subdirSelected);
				for (const auto& dirEntry : fs::recursive_directory_iterator(subdir)) {
					std::string displayName = /*(dirEntry.is_directory() ? "[D] " : "[F] ") + */dirEntry.path().filename().string();
					if (ImGui::Selectable(displayName.c_str(), itemSelected == i)) {
						itemSelected = i;
						pathSelected = FileSystem::Join(subdirSelected, dirEntry.path().filename().string());
					}
					i++;
				}
			}
			ImGui::EndChild();

			// a section to show audios under track
			if (subdirSelected == "audio") {
				DisplayAudioPanel();
			}

			ImGui::End();
		}
	};
}
