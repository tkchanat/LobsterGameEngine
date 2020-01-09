#pragma once
#include "pch.h"
#include "graphics/texture.h"
#include "graphics/2D/GameUI.h"

namespace Lobster {
	class GameUI;

	// Sprite2D is only applicable in GameUI
	// For 3D sprite and particles, use Sprite3D
	class Sprite2D {
	private:
		Texture2D* tex;
		float m_width, m_height; // actual image width and height (in pixel)
		int z;	// z-index (smaller goes to the front, allow negative)
		bool relativeSize = false; // indicate the size in px or %
		float _w, _h;	// temporarily store the width and height (in %)
		static int zCnt;
	public:
		float x;	// the x position of the sprite, in percentage [0, 1]
		float y;	// the y position of the sprite, in percentage [0, 1]
		float w;	// the width of the sprite, always in px (for that in %, see _w)
		float h;	// the height of the sprite, always in px (for that in %, see _h)

		Sprite2D(const char* path, float winW, float winH, float mouseX = 0.5f, float mouseY = 0.5f);
		~Sprite2D();
		inline ImTextureID GetTexID() const { return tex->Get(); }
		inline int GetZIndex() const { return z; }

		void SetZIndex(uint z);
		void Clip();	// adjust x, y into reasonable range
		void ImGuiMenu(GameUI* ui, ImVec2 winSize);
		void Draw();

		static bool Compare(Sprite2D* s1, Sprite2D* s2);
	};

	class TextSprite2D : public Sprite2D {
		static FT_Library library;
		static FT_Face face;
	};

}
