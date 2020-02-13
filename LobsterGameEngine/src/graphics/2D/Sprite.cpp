#include "pch.h"
#include "Sprite.h"
#include "graphics/Texture.h"
#include "graphics/Renderer.h"
#include "system/Input.h"

namespace Lobster {

	// ============================================================
	// Sprite2D
	// ============================================================

	int Sprite2D::zLv = 16;
	Config Sprite2D::config;

	Sprite2D::Sprite2D(float mouseX, float mouseY) : x(mouseX), y(mouseY), alpha(1.f) {
		z = zLv--; // z value count down
	}

	void Sprite2D::SetZIndex(uint z) {
		this->z = z;
	}

	bool Sprite2D::Compare(Sprite2D* s1, Sprite2D* s2) {
		return (s1->GetZIndex() > s2->GetZIndex());
	}

	// do nothing here
	void Sprite2D::SubmitDrawCommand() {}

	void Sprite2D::BasicMenuItem(GameUI* ui, ImVec2 winSize) {
		if (ImGui::MenuItem("Go to Front")) {
			ui->GoFront(this);
		}
		if (ImGui::MenuItem("Go to Back")) {
			ui->GoBack(this);
		}
		ImGui::Separator();
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
		if (ImGui::MenuItem("Delete")) {
			ui->RemoveSprite(this);
		}
		ImGui::PopStyleColor();
	}

	// ============================================================
	// ImageSprite2D
	// ============================================================

	ImageSprite2D::ImageSprite2D(const char* path, float winW, float winH, float mouseX, float mouseY) :
		Sprite2D(mouseX, mouseY)
	{
		std::string resPath = FileSystem::Join(PATH_SPRITES, path);
		tex = TextureLibrary::Use(FileSystem::Path(resPath).c_str());
		m_width = tex->GetWidth();
		m_height = tex->GetHeight();
		// default to be in px
		w = m_width; h = m_height;
		_w = m_width / winW;
		_h = m_height / winH;		
	}

	ImageSprite2D::~ImageSprite2D() {
		// we should not delete tex here, let TextureLibrary do the stuff
		// delete tex;
		tex = nullptr;
	}

	void ImageSprite2D::Clip() {
		ImVec2 winSize = ImGui::GetWindowSize();
		float pw = w / winSize.x, ph = h / winSize.y;
		if (x < 0.f) x = 0.f;
		else if (x + pw > 1.f) x = 1.f - pw;
		if (y < 0.f) y = 0.f;
		else if (y + ph > 1.f) y = 1.f - ph;
	}

	void ImageSprite2D::ImGuiMenu(GameUI* ui, ImVec2 winSize) {
		bool autoWidth = false, autoHeight = false;
		if (ImGui::Checkbox("Relative Size", &relativeSize));
		// sprite width
		if (ImGui::Button("Auto##auto_width")) {
			w = h * m_width / m_height;
			_w = w / winSize.x;
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
			h = w * m_height / m_width;
			_h = h / winSize.y;
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
		ImGui::SliderFloat("Alpha", &alpha, 0.f, 1.f, "%.2f");
		ImGui::Separator();
		// basic functions
		BasicMenuItem(ui, winSize);
	}

	void ImageSprite2D::OnImGuiRender() {
		ImGui::ImageButton(GetTexID(), ImVec2(w, h));
	}

	void ImageSprite2D::SubmitDrawCommand() {
		RenderOverlayCommand ocommand;
		ocommand.UseTexture = tex;
		ocommand.type = RenderOverlayCommand::Image;
		ocommand.x = x * config.width;
		ocommand.y = y * config.height;
		ocommand.w = w;
		ocommand.h = h;
		ocommand.alpha = alpha;
		ocommand.z = (float) z / 1000.f;
		Renderer::Submit(ocommand);		
	}

	bool ImageSprite2D::IsMouseOver() {
		double mx, my;
		Input::GetMousePos(mx, my);
		Config config;
		float winX = mx / config.width;
		float winY = my / config.height;
		if (winX >= x && winX <= x + _w && winY >= y && winY <= y + _h) return true;
		return false;
	}

	// ============================================================
	// TextSprite2D
	// ============================================================

	FT_Library TextSprite2D::s_library = nullptr;
	Texture2D* TextSprite2D::m_iconTex[3] = {};

	TextSprite2D::TextSprite2D(const char* text, const char* typeface, float winW, float winH, float mouseX, float mouseY) : 
		Sprite2D(mouseX, mouseY), text(text) {	
		strcpy(_text, text);
		// initialize static library, only called once
		if (!s_library) {
			FT_Error error = FT_Init_FreeType(&s_library);
			if (error) { ERROR("Unable to initialize FreeType Library."); throw; }
			// load the icons for button
			for (int i = 0; i < 3; i++) {
				m_iconTex[i] = TextureLibrary::Use(FileSystem::Path(m_iconPath[i]).c_str());
			}
		}
		// initialize face
		std::string resPath = FileSystem::Path(FileSystem::Join(PATH_FONT, typeface));
		loadFace(resPath);
		// set font size
		SetFontSize(24);
		// load the texture into it first
		getTexture();
	}

	void TextSprite2D::SetFontSize(float size) {
		fontSize = size;
		FT_Set_Pixel_Sizes(m_face, 0, size);
	}

	void TextSprite2D::SetColor(float r, float g, float b, float a) {
		color[0] = r; color[1] = g; color[2] = b; color[3] = a;
	}

	Texture2D* TextSprite2D::getTexture(bool reload) {
		// z-value should be unique in all sprites
		char id[8];
		sprintf(id, "%d", z);
		// search if the texture already exists
		Texture2D* texture = TextureLibrary::Use(id, nullptr, 0, 0);
		if (reload || !texture) {
			int off = 0;
			int pixelWidth = 0; // bounding box width in pixel
			// first iteration: calculate bounding box and kerning information
			for (int n = 0; n < text.size(); n++) {
				FT_Load_Char(m_face, text[n], FT_LOAD_RENDER);
				FT_Bitmap bmp = m_face->glyph->bitmap;
				pixelWidth += (bmp.width ? bmp.width : (m_face->glyph->advance.x >> 6));
			}
			// define the buffer to pass to OpenGL
			int ww = fontSize * text.size();
			int hh = fontSize * 2;
			std::vector<byte> buffer(ww * hh * 4, 0); // RGBA 4 channels
			// second iteration: render text into texture
			for (int n = 0; n < text.size(); n++) {
				// load glyph image into the slot (erase previous one) and renderer immediately
				FT_Load_Char(m_face, text[n], FT_LOAD_RENDER);
				FT_Bitmap bmp = m_face->glyph->bitmap;
				int bW = bmp.width;
				int bH = bmp.rows;
				// create the texture buffer of the entire text
				int top = hh * 0.66 - m_face->glyph->bitmap_top; // approx. vert center aligned
				int left = 0;
				if (alignType == Center) left = (ww - pixelWidth) / 2;
				else if (alignType == Right) left = ww - pixelWidth;
				for (int h = 0; h < bH; ++h) {
					for (int w = 0; w < bW; ++w) {		
						int pos = 4 * ((top + h) * ww + off + w + left);
						buffer[pos] = color[0] * 255;
						buffer[pos + 1] = color[1] * 255;
						buffer[pos + 2] = color[2] * 255;
						buffer[pos + 3] = bmp.buffer[h * bW + w];						
					}
				}
				// shift position
				off += (m_face->glyph->advance.x >> 6);
			}
			// load the buffer into texture
			texture = TextureLibrary::Use(id, buffer.data(), ww, hh);
		}
		return texture;
	}

	void TextSprite2D::loadFace(std::string fullpath) {
		// release memory
		if (m_face) {
			FT_Done_Face(m_face);
			m_face = nullptr;
		}
		// reinitialize face		
		FT_Error error = FT_New_Face(s_library, fullpath.c_str(), 0, &m_face);
		if (error == FT_Err_Unknown_File_Format) {
			ERROR("{}: Unsupported format", fullpath.c_str());
			throw;
		}
		else if (error) {
			ERROR("{}: File cannot be opened", fullpath.c_str());
			throw;
		}
	}

	void TextSprite2D::SubmitDrawCommand() {
		Texture2D* texture = getTexture();
		RenderOverlayCommand ocommand;
		ocommand.UseTexture = texture;
		ocommand.type = RenderOverlayCommand::Text;
		ocommand.x = x * config.width;
		ocommand.y = y * config.height;
		ocommand.w = fontSize * text.size();
		ocommand.h = fontSize * 2;
		ocommand.alpha = alpha;
		ocommand.z = (float)z / 1000.f;
		Renderer::Submit(ocommand);
	}

	void TextSprite2D::OnImGuiRender() {
		int length = fontSize * text.size();
		ImGui::SetWindowFontScale(fontSize / ImGui::GetFontSize());
		ImGui::PushStyleColor(0, ImVec4(color[0], color[1], color[2], 1.f));
		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.f, 0.5f));
		if (m_preview) {
			ImGui::ImageButton(getTexture()->Get(), ImVec2(fontSize * text.size(), fontSize * 2));
		}
		else {
			ImGui::Button(text.c_str(), ImVec2(0, fontSize * 2));
		}		
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
		ImGui::SetWindowFontScale(1.f);
	}

	void TextSprite2D::ImGuiMenu(GameUI* ui, ImVec2 winSize) {
		// preview mode
		ImGui::Checkbox("Preview", &m_preview);
		// text content		
		if (ImGui::InputText("Text", _text, MAX_TEXT_LENGTH)) {
			text = _text;
			getTexture(true); // reload texture
		}
		// alignment buttons
		for (int type = HorizontalAlignType::Left; type < HorizontalAlignType::Count; type++) {
			if (ImGui::ImageButton(m_iconTex[type]->Get(), ImVec2(16, 16))) {
				alignType = (HorizontalAlignType)type;
				getTexture(true);
			}
			ImGui::SameLine();
		}
		ImGui::Dummy(ImVec2(1, 1));
		// font type
		fs::path subdir = FileSystem::Path(PATH_FONT);
		if (ImGui::BeginCombo("Font Type", m_face->family_name)) {
			for (const auto& dirEntry : fs::recursive_directory_iterator(subdir)) {
				std::string displayName = dirEntry.path().stem().string();
				if (ImGui::Selectable(displayName.c_str(), false)) {
					// with unknown reason, switching font after constructor does not work
					std::string path = FileSystem::Join(PATH_FONT, dirEntry.path().filename().string());
					loadFace(FileSystem::Path(path));
					getTexture(true);
				}
			}
			ImGui::EndCombo();
		}
		// font size
		ImGui::InputFloat("Font Size", &fontSize, 1.f, 1.f, "%.1f");
		// color picker
		if (ImGui::ColorEdit3("Font Color", color)) {
			getTexture(true);
		}
		if (ImGui::SliderFloat("Alpha", &alpha, 0.f, 1.f, "%.2f")) {
			getTexture(true);
		}
		ImGui::Separator();
		// basic functions
		BasicMenuItem(ui, winSize);
	}

	void TextSprite2D::Clip() {		
		float length = (float) fontSize * text.size() / config.width;
		float height = (float) fontSize * 2 / config.height;
		if (x < 0.f) x = 0.f;		
		else if (x + length / 2 > 1.f) x = 1.f - length / 2;
		if (y < 0.f) y = 0.f;
		else if (y + height > 1.f) y = 1.f - height;
	}

	bool TextSprite2D::IsMouseOver() {
		double mx, my;
		Input::GetMousePos(mx, my);
		float w = fontSize * text.size();
		float h = fontSize * 2;
		if (mx >= x && mx <= x + w && my >= y && my <= y + h) return true;
		return false;
	}

} 