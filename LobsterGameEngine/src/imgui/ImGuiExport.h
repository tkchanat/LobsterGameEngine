#pragma once
#include "ImGuiComponent.h"

namespace Lobster
{

	class ImGuiExport : public ImGuiComponent
	{
	private:
		struct Platform {
		public:
			std::string Name;
			std::string IconPath;
		public:
			Platform(const char* name, const char* path) : Name(name), IconPath(path) {}
		};
		enum ExportStatusFlag : int {
			NONE = 0,
			OK = BIT(0),
			TEMPLATE_NOT_FOUND = BIT(1),
			PATH_NOT_COMPLETE = BIT(2),
			SCENE_NOT_SET = BIT(3),
			UNKNOWN_ERROR = BIT(4)
		};
	private:
		std::vector<Platform> m_platforms;
		int m_exportStatusFlag = 0;
	public:
		ImGuiExport() {
			m_platforms.emplace_back("Windows", "textures/ui/windows.png");
			m_platforms.emplace_back("MacOSX", "textures/ui/macos.png");
		}
		virtual void Show(bool* p_open) override
		{
			const char* title = "Export Project";
			static char path_buf[MAX_FILE_BUFFER_SIZE] = { '\0' };
			static char template_buf[MAX_FILE_BUFFER_SIZE] = { '\0' };
			static char scene_buf[MAX_FILE_BUFFER_SIZE] = { '\0' };
			static char name_buf[128] = "Chopsticks";
			if (!p_open) return;
			ImGui::OpenPopup(title);

			ImGui::SetNextWindowSize(ImVec2(400, 0));
			if (ImGui::BeginPopupModal(title, NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				// ========================================================
				// Select platform
				ImGui::Columns(2, NULL, false);
				ImGui::Text("Select platform to export:");
				static int e = 0;
				ImVec2 icon_size(24, 24);
				for (int i = 0; i < m_platforms.size(); ++i) {
					ImGui::RadioButton(m_platforms[i].Name.c_str(), &e, i);
					ImGui::SameLine();
					Texture2D* image = TextureLibrary::Use(m_platforms[i].IconPath.c_str());
					ImGui::Image(image ? image->Get() : nullptr, icon_size);
				}
				ImGui::NextColumn();
				ImGui::Dummy(ImVec2(100, 64));
				ImGui::SameLine();
				Texture2D* icon = TextureLibrary::Placeholder();
				ImGui::ImageButton(icon->Get(), ImVec2(64, 64));
				ImGui::Columns(1);
				ImGui::Separator();

				// ========================================================
				// Export options
				ImGui::Text("Export options:");
				ImGui::SetNextItemWidth(250);
				// Export path
				ImGui::InputText("Export path", path_buf, IM_ARRAYSIZE(path_buf));
				ImGui::SameLine();
				if (ImGui::Button("...###path")) {
					std::string path = FileSystem::OpenDirectoryDialog();
					if (!path.empty()) {
						memcpy(path_buf, path.data(), MAX_PATH * sizeof(char));
					}
				}
				// Template path
				if (m_exportStatusFlag & TEMPLATE_NOT_FOUND) {
					ImGui::SetNextItemWidth(250);
					ImGui::InputText("Template path", template_buf, IM_ARRAYSIZE(template_buf));
					ImGui::SameLine();
					if (ImGui::Button("...###template")) {
						std::string path = FileSystem::OpenFileDialog();
						if (!path.empty()) {
							memcpy(template_buf, path.data(), MAX_PATH * sizeof(char));
						}
					}
				}
				// Executable name
				ImGui::SetNextItemWidth(250);
				ImGui::InputText("Executable name", name_buf, IM_ARRAYSIZE(name_buf));
				bool path_invalid = path_buf[0] == '\0' || name_buf == '\0';
				m_exportStatusFlag = path_invalid ? m_exportStatusFlag | PATH_NOT_COMPLETE : m_exportStatusFlag & ~PATH_NOT_COMPLETE;
				// Startup scene
				ImGui::SetNextItemWidth(250);
				ImGui::InputText("Startup scene", scene_buf, IM_ARRAYSIZE(scene_buf));
				ImGui::SameLine();
				if (ImGui::Button("...###template")) {
					std::string path = FileSystem::OpenFileDialog();
					if (!path.empty()) {
						memcpy(scene_buf, path.data(), MAX_PATH * sizeof(char));
					}
				}
				m_exportStatusFlag = scene_buf[0] == '\0' ? m_exportStatusFlag | SCENE_NOT_SET : m_exportStatusFlag & ~SCENE_NOT_SET;
				ImGui::Separator();

				// ========================================================
				// Messages
				ImGui::Text("Messages:");
				if (m_exportStatusFlag & OK) {
					ImGui::TextColored(ImVec4(0, 1, 0, 1), "Successfully published!");
				}
				if (m_exportStatusFlag & TEMPLATE_NOT_FOUND) {
					ImGui::TextColored(ImVec4(1, 0, 0, 1), "Template not found!\nPlease manually specify template path.");
				}
				if (m_exportStatusFlag & PATH_NOT_COMPLETE) {
					ImGui::Text("Specify the executable path and name.");
				}
				if (m_exportStatusFlag & SCENE_NOT_SET) {
					ImGui::Text("Specify the startup scene.");
				}
				ImGui::Separator();

				// ========================================================
				// Actions
				if (path_invalid) {
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				}
				if (ImGui::Button("Export", ImVec2(190, 0))) {
					if (FileSystem::ExistDirectory(path_buf)) {
						// copy and move relative files
						if (template_buf[0] == '\0')
							m_exportStatusFlag = Export(path_buf, name_buf, "./templates/windows_template.exe", scene_buf);
						else
							m_exportStatusFlag = Export(path_buf, name_buf, template_buf, scene_buf);
					}
				}
				if (path_invalid) {
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
				}
				ImGui::SetItemDefaultFocus();
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(190, 0))) { 
					ImGui::CloseCurrentPopup(); 
					*p_open = false;
				}
				ImGui::EndPopup();
			}
		}

		int Export(const std::string& outDir, const std::string& appName, const char* templatePath, const char* scenePath) {
#ifdef LOBSTER_PLATFORM_WIN
			// Try to find template file
			if (!FileSystem::Exist(templatePath)) 
				return TEMPLATE_NOT_FOUND;
			std::string source_template = FileSystem::RelativeToAbsolute(templatePath);
			// Make directory
			std::string game_folder = outDir + "\\" + appName;
			std::string mkdir_command = "mkdir \"" + game_folder + "\"";
			system(mkdir_command.c_str());
			// Copy template file
			std::string copy_command = "copy \"" + source_template + "\" \"" + game_folder + "\\" + appName + ".exe\"";
			system(copy_command.c_str());
			// Copy resource files
			std::string resources_dir = FileSystem::GetCurrentWorkingDirectory();
			resources_dir = FileSystem::Absolute(resources_dir);
			copy_command = "xcopy /s \"" + resources_dir + "\" \"" + game_folder + "\\resources\\\" /y";
			system(copy_command.c_str());
			std::string exe_dir = FileSystem::GetCurrentExecutableDirectory();
			copy_command = "xcopy \"" + exe_dir + "\\*.dll\" \"" + game_folder + "\\\" /y";
			system(copy_command.c_str());
#elif LOBSTER_PLATFORM_MAC
#else
#endif

			std::string iniPath = game_folder + "\\resources\\output.txt";
			std::ofstream out(iniPath);
			if (out.is_open()) {
				out << FileSystem::Path(scenePath);
			}
			out.close();

			return OK;
		}

	};

}
