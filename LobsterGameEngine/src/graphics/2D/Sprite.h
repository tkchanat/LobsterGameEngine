#pragma once
#include "pch.h"
#include "graphics/texture.h"

namespace Lobster {
	class GameUI;
	class Script;

	// Sprite2D is only applicable in GameUI
	// For 3D sprite and particles, use Sprite3D
	class Sprite2D {
		friend class GameUI;
	protected:
		int z;	// z-index (smaller goes to the front, allow negative)
		static int zLv;
		std::string m_label; // used for get sprite directly in script
		bool m_clicked = false;
		bool m_hovered = false;
		bool isButton = false;		
		float colorOnHover[4] = { 0.f, 0.f, 0.f, 0.08f };
		float colorOnClick[4] = { 0.f, 0.f, 0.f, 0.16f };
		// store the script bound with mouse interaction
		Script* scriptOnHover = nullptr, * scriptOnClick = nullptr;
		// store the relative path of the script to run
		std::string scriptNameOnHover, scriptNameOnClick;
		std::string funcOnHover = "OnHover", funcOnClick = "OnClick";

		void BasicMenuItem(GameUI* ui, ImVec2 winSize);
	public:
		float x;	// the x position of the sprite, in percentage [0, 1]
		float y;	// the y position of the sprite, in percentage [0, 1]
		float alpha;// alpha value of the sprite, in range [0, 1]
		enum class SpriteType { ImageSprite, TextSprite, DynamicTextSprite } spriteType;

		// constructor for derived classes
		Sprite2D(float mouseX, float mouseY);
		inline int GetZIndex() const { return z; }

		void SetZIndex(uint z);		
		virtual void Clip() {}
		virtual void ImGuiMenu(GameUI* ui, ImVec2 winSize) = 0;
		virtual void OnBegin();
		virtual void OnUpdate(double dt);
		virtual void OnImGuiRender() = 0;
		virtual bool IsMouseOver() { return false; }
		virtual std::string GetLabel() { return m_label; }
		// compare if s1 is in front of s2chec
		static bool Compare(Sprite2D* s1, Sprite2D* s2);
	};

	class ImageSprite2D : public Sprite2D {	
		friend class GameUI;
	private:
		Texture2D* tex;
		float m_width, m_height; // actual image width and height (in pixel)
		float _w, _h;	// temporarily store the width and height (in %)		
		bool relativeSize = false; // indicate the size in px or %
		std::string path; // track the path for scene saving/loading
	public:
		float w;	// the width of the sprite, always in px (for that in %, see _w)
		float h;	// the height of the sprite, always in px (for that in %, see _h)
		// standard constructor
		ImageSprite2D(const char* path, float winW, float winH, float mouseX = 0.5f, float mouseY = 0.5f);
		~ImageSprite2D();
		inline ImTextureID GetTexID() const { return tex->Get(); }
		inline std::string GetPath() const { return path; }

		virtual void Clip() override;	// adjust x, y into reasonable range
		virtual void ImGuiMenu(GameUI* ui, ImVec2 winSize) override;
		virtual void OnUpdate(double dt) override;
		virtual void OnImGuiRender() override;
		virtual bool IsMouseOver() override;
	};

	class TextSprite2D : public Sprite2D {
		friend class GameUI;
	public:
		enum HorizontalAlignType { Left, Center, Right, Count };
	protected:
		static const int MAX_TEXT_LENGTH = 128;
		static FT_Library s_library; // note that FT_Library is a pointer type
		HorizontalAlignType alignType = Left;
		FT_Face m_face = nullptr;
		float fontSize = 12.f;
		float color[4] = { 1.f, 1.f, 1.f, 1.f }; // font color, in range [0, 1]
		std::string text;
		std::string fontName;

		bool m_preview = false; // show preview in imgui
		const char* m_iconPath[3] = {
			"textures/ui/align-left.png", "textures/ui/align-center.png", "textures/ui/align-right.png"
		};
		static Texture2D* m_iconTex[3];
	protected:
		// find texture base on the id, if no texture found, create one with the id
		// AG: For FYP, this part works fine. If we gonna further develop, it is highly 
		// prioritized for refactor.
		Texture2D* getTexture(bool reload = false);
		// helper function to load typeface
		void loadFace(std::string fullpath);
	public:
		TextSprite2D(const char* text, const char* typeface, float fontSize, float winW, float winH, float mouseX, float mouseY);

		// setter
		inline void SetText(const char* text) { this->text = text; }
		void SetFontSize(float size);
		void SetColor(float r, float g, float b, float a);
		// getter
		inline std::string GetText() { return text; }
		inline std::string GetFontName() { return fontName; }
		inline float GetFontSize() { return fontSize; }
		inline glm::vec4 GetColor() { return glm::vec4(color[0], color[1], color[2], color[3]); }
		
		virtual void Clip() override;		
		virtual void ImGuiMenuHelper(); // let its descendent call helper directly on text formatting
		virtual void ImGuiMenu(GameUI* ui, ImVec2 winSize) override;		
		virtual void OnUpdate(double dt) override;
		virtual void OnImGuiRender() override;
		virtual bool IsMouseOver() override;
	};

	class DynamicTextSprite2D final : public TextSprite2D {
		friend class GameUI;
	public:
		enum SupportedVarType { INT, FLOAT, STRING };
	private:
		Script* script = nullptr;
		std::string scriptName;
		std::string var;
		SupportedVarType type;
	public:
		DynamicTextSprite2D(const char* sname, const char* vname, SupportedVarType vtype, const char* typeface, float fontSize,
			float winW, float winH, float mouseX, float mouseY);

		inline std::string GetScriptName() { return scriptName; }
		inline std::string GetVarName() { return var; }
		inline SupportedVarType GetVarType() { return type; }

		virtual void OnBegin() override;
		virtual void OnUpdate(double dt) override;
		virtual void ImGuiMenu(GameUI* ui, ImVec2 winSize) override;
	};

}
