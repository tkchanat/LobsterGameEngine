#include "pch.h"
#include "Sprite.h"
#include "graphics/Texture.h"

namespace Lobster {

	int Sprite2D::zCnt = 1;

	Sprite2D::Sprite2D(const char* path, float winW, float winH, float mouseX, float mouseY) :
		x(mouseX), y(mouseY)
	{
		// TODO compatible with both directories with sprite and without sprite
		std::string resPath = FileSystem::Join(PATH_SPRITES, path);
		tex = TextureLibrary::Use(FileSystem::Path(resPath).c_str());
		m_width = tex->GetWidth();
		m_height = tex->GetHeight();
		// default to be in px
		w = m_width; h = m_height;
		_w = m_width / winW;
		_h = m_height / winH;
		z = zCnt++;
	}

	Sprite2D::~Sprite2D() {
		// Note that we should not delete tex here, let TextureLibrary do the stuff
		// delete tex;
		tex = nullptr;
	}

	bool Sprite2D::Compare(Sprite2D* s1, Sprite2D* s2) {
		return (s1->GetZIndex() < s2->GetZIndex());
	}

	void Sprite2D::SetZIndex(uint z) {
		this->z = z;
	}

	void Sprite2D::Clip() {
		ImVec2 winSize = ImGui::GetWindowSize();
		float pw = w / winSize.x, ph = h / winSize.y;
		if (x < 0.f) x = 0.f;
		else if (x + pw > 1.f) x = 1.f - pw;
		if (y < 0.f) y = 0.f;
		else if (y + ph > 1.f) y = 1.f - ph;
	}

	void Sprite2D::ImGuiMenu(GameUI* ui, ImVec2 winSize) {
		if (ImGui::MenuItem("Go to Front")) {
			ui->GoFront(this);
		}
		if (ImGui::MenuItem("Go to Back")) {
			ui->GoBack(this);
		}
		ImGui::Separator();
		bool autoWidth = false, autoHeight = false;
		if (ImGui::Checkbox("Relative Size", &relativeSize));
		// sprite width
		if (ImGui::Button("Auto##auto_width")) {
			w = h * (relativeSize ? (winSize.y / winSize.x) : (1.f)) * m_width / m_height;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100.0f);
		if (relativeSize) {
			if (ImGui::SliderFloat("Width (%)", &_w, 0.f, 1.f)) {
				w = winSize.x * _w;
			}
		}
		else if (ImGui::SliderFloat("Width (px)", &w, 1.f, winSize.x, "%1.f")) {
			_w = w / winSize.x;
		}
		// sprite height
		if (ImGui::Button("Auto##auto_height")) {
			h = w * (relativeSize ? (winSize.x / winSize.y) : (1.f)) * m_height / m_width;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100.0f);
		if (relativeSize) {
			if (ImGui::SliderFloat("Height (%)", &_h, 0.f, 1.f)) {
				h = winSize.y * _h;
			}
		}
		else if (ImGui::SliderFloat("Height (px)", &h, 1.f, winSize.y, "%1.f")) {
			_h = h / winSize.y;
		}
		ImGui::Separator();
		// delete
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
		if (ImGui::MenuItem("Delete")) {
			ui->RemoveSprite(this);
		}
		ImGui::PopStyleColor();
	}

	void Sprite2D::Draw() {				
		GLuint positionBufferObject;
		GLfloat vertexPositions[] =
		{
			-1.0f, -1.0f, 0.0f, 1.0f,
			-1.0f,  1.0f, 0.0f, 1.0f,
			 1.0f,  1.0f, 0.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 1.0f,
		};

		glClear(GL_COLOR_BUFFER_BIT);

		glGenBuffers(1, &positionBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, positionBufferObject);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 5 * 4, vertexPositions, GL_STATIC_DRAW);

		/*
		GLuint tBuf = NULL;
		glGenBuffers(1, &tBuf);
		glBindBuffer(GL_ARRAY_BUFFER, tBuf);
		glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), tCoords, GL_STATIC_DRAW);
		*/

		// buffer
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 5);
		glDisableVertexAttribArray(0);
	}
}