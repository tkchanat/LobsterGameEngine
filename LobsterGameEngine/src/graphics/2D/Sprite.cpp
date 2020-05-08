#include "pch.h"
#include "Sprite.h"
#include "graphics/Texture.h"
#include "graphics/Renderer.h"
#include "system/Input.h"
#include "Scripts/Script.h"

#include <cereal/archives/binary.hpp>
#include <cereal/types/polymorphic.hpp>

#define Z_LEVEL_MAX 256.f;

namespace Lobster {

	// ============================================================
	// Sprite2D
	// ============================================================

	int Sprite2D::zLv = Z_LEVEL_MAX;

	Sprite2D::Sprite2D(float mouseX, float mouseY) : x(mouseX), y(mouseY), alpha(1.f) {
		z = zLv--; // z value count down		
	}

	void Sprite2D::SetZIndex(uint z) {
		this->z = z;
	}

	bool Sprite2D::Compare(Sprite2D* s1, Sprite2D* s2) {
		return (s1->GetZIndex() < s2->GetZIndex());
	}

	void Sprite2D::OnBegin() {
		// load and begin script
		if (!scriptNameOnHover.empty() && !funcOnHover.empty()) {
			scriptOnHover = new Script(scriptNameOnHover.c_str());
			scriptOnHover->OnBegin();
			std::string errmsg = scriptOnHover->GetErrmsg();
			if (!errmsg.empty()) {
				LOG(errmsg.c_str());
			}
		}
		if (!scriptNameOnClick.empty() && !funcOnClick.empty()) {
			scriptOnClick = new Script(scriptNameOnClick.c_str());
			scriptOnClick->OnBegin();
			std::string errmsg = scriptOnClick->GetErrmsg();
			if (!errmsg.empty()) {
				LOG(errmsg.c_str());
			}
		}
	}

	// do button related work
	void Sprite2D::OnUpdate(double dt) {
		if (!isButton) return;
		if (scriptOnHover && IsMouseOver() && !m_hovered) {
			m_hovered = true;
		}
		if (scriptOnClick && IsMouseOver() && Input::IsMouseDown(GLFW_MOUSE_BUTTON_LEFT) && !m_clicked) {
			m_clicked = true;
		}
		if (Input::IsMouseUp(GLFW_MOUSE_BUTTON_LEFT)) {
			m_clicked = false;
		}
		if (!IsMouseOver()) {
			m_hovered = false;
		}
	}

	void Sprite2D::OnLateUpdate(double dt)
	{
		if (scriptOnHover && m_hovered) {
			scriptOnHover->Execute(funcOnHover.c_str());
		}
		if (scriptOnClick && m_clicked) {
			scriptOnClick->Execute(funcOnClick.c_str());
		}
	}

	void Sprite2D::BasicMenuItem(GameUI* ui, ImVec2 winSize) {
		// Label Property ======
		static char label_[32];
		if (ImGui::InputText("Label", label_, 32)) {
			m_label = label_;
		}
		ImGui::Separator();
		// Button Property ======
		if (ImGui::Checkbox("Button", &isButton));
		if (!isButton) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::TreeNode("Button Property")) {
			// load all available scripts
			std::vector<std::string> scripts;
			std::string path = FileSystem::Path("scripts");
			scripts.push_back("");
			for (const auto& dirEntry : fs::recursive_directory_iterator(path)) {
				std::string displayName = dirEntry.path().filename().string();
				scripts.push_back(displayName);
			}
			if (ImGui::ColorEdit4("Color OnHover", colorOnHover));
			if (ImGui::BeginCombo("Script OnHover", (scriptNameOnHover.empty() ? "None" : scriptNameOnHover.c_str()))) {
				for (std::string& file : scripts)
					if (ImGui::Selectable(file.c_str()))
						scriptNameOnHover = file;
				ImGui::EndCombo();
			}
			ImGui::InputText("Function OnHover", &funcOnHover[0], 32);
			if (ImGui::ColorEdit4("Color OnClick", colorOnClick));
			if (ImGui::BeginCombo("Script OnClick", (scriptNameOnClick.empty() ? "None" : scriptNameOnClick.c_str()))) {
				for (std::string& file : scripts)
					if (ImGui::Selectable(file.c_str()))
						scriptNameOnClick = file;
				ImGui::EndCombo();
			}
			ImGui::InputText("Function OnClick", &funcOnClick[0], 32);
			ImGui::TreePop();
		}
		if (!isButton) {
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
		// Z Index related ======
		ImGui::Separator();
		if (ImGui::MenuItem("Go to Front")) {
			ui->GoFront(this);
		}
		if (ImGui::MenuItem("Go to Back")) {
			ui->GoBack(this);
		}
		// Clone/Delete options ======
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
		Sprite2D(mouseX, mouseY), path(path)
	{
		spriteType = SpriteType::ImageSprite;
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

	void ImageSprite2D::OnUpdate(double dt) {
		// run script
		Sprite2D::OnUpdate(dt);
		// hover/click color
		RenderOverlayCommand ocommand;		
		if (Application::GetMode() == ApplicationMode::GAME && isButton) {
			bool mouseOver = IsMouseOver();
			if (mouseOver && Input::IsMouseDown(GLFW_MOUSE_BUTTON_LEFT)) {
				ocommand.blendR = colorOnClick[0];
				ocommand.blendG = colorOnClick[1];
				ocommand.blendB = colorOnClick[2];
				ocommand.blendA = colorOnClick[3];
			}
			else if (mouseOver) {
				ocommand.blendR = colorOnHover[0];
				ocommand.blendG = colorOnHover[1];
				ocommand.blendB = colorOnHover[2];
				ocommand.blendA = colorOnHover[3];
			}
		}
		// submit draw command
		Config& config = Application::GetInstance()->GetConfig();
		ocommand.UseTexture = tex;
		ocommand.type = RenderOverlayCommand::Image;
		ocommand.x = x * config.defaultWidth;
		ocommand.y = y * config.defaultHeight;
		ocommand.w = w;
		ocommand.h = h;
		ocommand.alpha = alpha;
		ocommand.z = (float)z / Z_LEVEL_MAX;
		Renderer::Submit(ocommand);		
	}

	bool ImageSprite2D::IsMouseOver() {
		double mx, my;
		Input::GetMousePos(mx, my);
		Config& config = Application::GetInstance()->GetConfig();
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

	TextSprite2D::TextSprite2D(const char* text, const char* typeface, float fontSize, float winW, float winH, float mouseX, float mouseY) : 
		Sprite2D(mouseX, mouseY), text(text), fontName(typeface) {	
		spriteType = SpriteType::TextSprite;
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
		SetFontSize(fontSize);
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

	void TextSprite2D::OnUpdate(double dt) {
		// run script
		Sprite2D::OnUpdate(dt);
		// hover/click color
		RenderOverlayCommand ocommand;
		if (Application::GetMode() == ApplicationMode::GAME && isButton) {
			bool mouseOver = IsMouseOver();
			if (mouseOver && Input::IsMouseDown(GLFW_MOUSE_BUTTON_LEFT)) {
				ocommand.blendR = colorOnClick[0];
				ocommand.blendG = colorOnClick[1];
				ocommand.blendB = colorOnClick[2];
				ocommand.blendA = colorOnClick[3];
			}
			else if (mouseOver) {
				ocommand.blendR = colorOnHover[0];
				ocommand.blendG = colorOnHover[1];
				ocommand.blendB = colorOnHover[2];
				ocommand.blendA = colorOnHover[3];
			}
		}
		// submit draw command
		Config& config = Application::GetInstance()->GetConfig();
		Texture2D* texture = getTexture();
		ocommand.UseTexture = texture;
		ocommand.type = RenderOverlayCommand::Text;
		ocommand.x = x * config.defaultWidth;
		ocommand.y = y * config.defaultHeight;
		ocommand.w = fontSize * text.size();
		ocommand.h = fontSize * 2;
		ocommand.alpha = alpha;
		ocommand.z = (float)z / Z_LEVEL_MAX;
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

	void TextSprite2D::ImGuiMenuHelper() {
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
		// ===================================
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
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
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
		ImGui::SameLine();
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted("This is disabled because of a bug which is not yet fixed.");
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
		// ===================================
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
	}

	void TextSprite2D::ImGuiMenu(GameUI* ui, ImVec2 winSize) {
		// preview mode
		ImGui::Checkbox("Preview", &m_preview);
		// text content		
		if (ImGui::InputText("Text", &text[0], MAX_TEXT_LENGTH)) {
			getTexture(true); // reload texture
		}
		// text formatting
		ImGuiMenuHelper();
		// basic functions
		BasicMenuItem(ui, winSize);
	}

	void TextSprite2D::Clip() {
		Config& config = Application::GetInstance()->GetConfig();
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

	// ============================================================
	// DynamicTextSprite2D
	// ============================================================
	DynamicTextSprite2D::DynamicTextSprite2D(const char* sname, const char* vname, SupportedVarType vtype, const char* typeface, float fontSize,
		float winW, float winH, float mouseX, float mouseY) :
		TextSprite2D(vname, typeface, fontSize, winW, winH, mouseX, mouseY), scriptName(sname), type(vtype), var(vname)
	{
		spriteType = SpriteType::DynamicTextSprite;
		text = "<" + var + ">";
	}

	void DynamicTextSprite2D::OnBegin() {
		// load script
		if (!scriptName.empty() && !var.empty()) {
			script = new Script(scriptName.c_str());
		}
		TextSprite2D::OnBegin();
		script->OnBegin();		
	}

	void DynamicTextSprite2D::OnUpdate(double dt) {
		static std::string prevText = text;
		if (script && Application::GetMode() == GAME) {
			script->OnUpdate(dt);
			luabridge::LuaRef ref = script->GetVar(var.c_str());
			static char prevErrmsg[512];
			if (!ref.isNil()) {
				try {
					switch (type) {
					case SupportedVarType::INT:
					{
						int vi = ref.cast<int>();
						text = std::to_string(vi);
					}
					break;
					case SupportedVarType::FLOAT:
					{
						float vf = ref.cast<float>();
						text = std::to_string(vf);
					}
					break;
					case SupportedVarType::STRING:
						text = ref.cast<std::string>();
						break;
					}
				}
				catch (std::exception& e) {
					char errmsg[512];
					sprintf(errmsg, "%s in %s: %s", var.c_str(), scriptName.c_str(), e.what());
					if (strcmp(prevErrmsg, errmsg) != 0) {
						WARN(errmsg);
						strcpy(prevErrmsg, errmsg);
					}
					text = "<" + var + ">";
				}
			}
		}
		else {
			text = "<" + var + ">";
		}
		// reload texture if content changed
		if (prevText != text) {
			getTexture(true);
			prevText = text;
		}		
		// render the text
		TextSprite2D::OnUpdate(dt);
	}

	void DynamicTextSprite2D::ImGuiMenu(GameUI* ui, ImVec2 winSize) {
		// script to track
		if (ImGui::BeginCombo("Script", (scriptName.empty() ? "None" : scriptName.c_str()))) {
			if (ImGui::Selectable("None")) {
				scriptName.clear();
			}
			for (const auto& dirEntry : fs::recursive_directory_iterator(FileSystem::Path(PATH_SCRIPTS))) {
				std::string displayName = dirEntry.path().filename().string();
				if (ImGui::Selectable(displayName.c_str())) {
					scriptName = displayName;
				}
			}
			ImGui::EndCombo();
		}
		
		// variable name to track
		if (ImGui::InputText("Variable", &var[0], MAX_TEXT_LENGTH)) {
			var = var.c_str(); // i know, this is stupid, but it works to fix a bug
			text = "<" + var + ">";
			getTexture(true); // reload texture
		}
		// variable type to track
		const static char* vtypeStr[] = { "int", "float", "string" };
		ImGui::Combo("Type", (int*)&type, vtypeStr, IM_ARRAYSIZE(vtypeStr));
		// text formatting
		ImGuiMenuHelper();
		BasicMenuItem(ui, winSize);
	}

} 
