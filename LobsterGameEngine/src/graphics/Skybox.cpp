#include "pch.h"
#include "Skybox.h"
#include "graphics/Texture.h"

namespace Lobster
{

	Skybox::Skybox(const char * right, const char * left, const char * up, const char * down, const char * back, const char * front) :
		m_cubemap(new TextureCube())
	{
		m_faces[0] = right;
		m_faces[1] = left;
		m_faces[2] = up;
		m_faces[3] = down;
		m_faces[4] = back;
		m_faces[5] = front;
		m_cubemap->Set(
			m_faces[0].c_str(),
			m_faces[1].c_str(),
			m_faces[2].c_str(),
			m_faces[3].c_str(),
			m_faces[4].c_str(),
			m_faces[5].c_str()
		);
	}

	Skybox::~Skybox()
	{
		if (m_cubemap) delete m_cubemap;
		m_cubemap = nullptr;
	}

	void Skybox::OnImGuiRender(bool * p_open)
	{
		const ImVec2 btnSize = ImVec2(66, 66);
		const ImVec2 padding = ImVec2(20, 20);
		const ImVec2 winSize = ImVec2(padding.x * 2 + btnSize.x * 4, padding.y * 3 + btnSize.y * 3);
		const ImVec2 btnPos[6] = {
			ImVec2(padding.x + btnSize.x, padding.y * 2),
			ImVec2(padding.x, padding.y * 2 + btnSize.y),
			ImVec2(padding.x + btnSize.x, padding.y * 2 + btnSize.y),
			ImVec2(padding.x + btnSize.x * 2, padding.y * 2 + btnSize.y),
			ImVec2(padding.x + btnSize.x * 3, padding.y * 2 + btnSize.y),
			ImVec2(padding.x + btnSize.x, padding.y * 2 + btnSize.y * 2),
		};
		const std::string dirPos[6] = { "Right", "Left", "Top", "Bottom", "Back", "Front" };
		ImGui::SetNextWindowSize(winSize);
		ImGui::Begin("Skybox Editor", p_open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse);
		for (unsigned int i = 0; i < 6; i++) {
			ImGui::SetCursorPos(btnPos[i]);
			Texture2D* image = TextureLibrary::Use(m_faces[i].c_str());
			if (ImGui::ImageButton(image ? image->Get() : nullptr, ImVec2(64, 64), ImVec2(0, 0), ImVec2(1, 1), 1)) {
				std::string fullpath = FileSystem::OpenFileDialog();
				if (!fullpath.empty()) {
					m_faces[i] = FileSystem::Path(fullpath);
					m_cubemap->Set(
						m_faces[0].c_str(),
						m_faces[1].c_str(),
						m_faces[2].c_str(),
						m_faces[3].c_str(),
						m_faces[4].c_str(),
						m_faces[5].c_str()
					);
				}
			}
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip(dirPos[i].c_str());
			}
		}
		ImGui::End();
	}

}