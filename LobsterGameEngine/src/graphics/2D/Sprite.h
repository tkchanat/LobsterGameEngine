#pragma once
#include "pch.h"
#include "graphics/texture.h"
#include "graphics/2D/GameUI.h"


namespace Lobster {
	class GameUI;

	// Sprite2D is only applicable in GameUI
	// For 3D sprite and particles, use Sprite3D
	class Sprite2D {			
	protected:
		int z;	// z-index (smaller goes to the front, allow negative)
		static int zLv;
		static Config config;

		void BasicMenuItem(GameUI* ui, ImVec2 winSize);
	public:
		float x;	// the x position of the sprite, in percentage [0, 1]
		float y;	// the y position of the sprite, in percentage [0, 1]

		// constructor for derived classes
		Sprite2D(float mouseX, float mouseY);
		inline int GetZIndex() const { return z; }

		void SetZIndex(uint z);
		virtual void Clip() {}
		virtual void ImGuiMenu(GameUI* ui, ImVec2 winSize) = 0;
		virtual void OnImGuiRender() = 0;
		virtual void SubmitDrawCommand();
		// compare if s2 is in front of s1
		static bool Compare(Sprite2D* s1, Sprite2D* s2);
	};

	class ImageSprite2D : public Sprite2D {
	private:
		Texture2D* tex;
		float m_width, m_height; // actual image width and height (in pixel)
		float _w, _h;	// temporarily store the width and height (in %)		
		bool relativeSize = false; // indicate the size in px or %

	public:
		float w;	// the width of the sprite, always in px (for that in %, see _w)
		float h;	// the height of the sprite, always in px (for that in %, see _h)
		// standard constructor
		ImageSprite2D(const char* path, float winW, float winH, float mouseX = 0.5f, float mouseY = 0.5f);
		~ImageSprite2D();
		inline ImTextureID GetTexID() const { return tex->Get(); }

		virtual void Clip() override;	// adjust x, y into reasonable range
		virtual void ImGuiMenu(GameUI* ui, ImVec2 winSize) override;
		virtual void OnImGuiRender() override;
		virtual void SubmitDrawCommand() override;
	};

	class TextSprite2D : public Sprite2D {
		enum HorizontalAlignType { Left, Center, Right, Count };
	private:
		static const int MAX_TEXT_LENGTH = 128;
		static FT_Library s_library; // note that FT_Library is a pointer type
		HorizontalAlignType alignType = Left;
		FT_Face m_face = nullptr;	// typeface stored for FreeType	
		float fontSize = 12.f;
		float color[4] = { 1.f, 1.f, 1.f, 1.f }; // font color, in range [0, 1]
		std::string text;
		char _text[128];

		bool m_preview = false; // show preview in imgui
		const char* m_iconPath[3] = {
			"textures/ui/align-left.png", "textures/ui/align-center.png", "textures/ui/align-right.png"
		};
		static Texture2D* m_iconTex[3];
	private:
		// find texture base on the id, if no texture found, create one with the id
		// AG: For FYP, this part works fine. If we gonna further develop, it is highly 
		// prioritized for refactor.
		Texture2D* getTexture(bool reload = false);
		// helper function to load typeface
		void loadFace(std::string fullpath);
	public:
		TextSprite2D(const char* text, const char* typeface, float winW, float winH, float mouseX, float mouseY);

		void SetFontSize(int size);
		virtual void Clip() override;
		virtual void ImGuiMenu(GameUI* ui, ImVec2 winSize) override;
		virtual void OnImGuiRender() override;		
		virtual void SubmitDrawCommand() override;
	};

}
