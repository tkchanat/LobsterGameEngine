#pragma once
#include "Sprite.h"

namespace Lobster {

	class Sprite2D;
	class TextSprite2D;

	// save all information needed to generate Sprite
	struct SpriteInfo {
		Sprite2D::SpriteType type;
		float x, y;		
		int z;
		float alpha;
		bool isButton = false;
		float colorOnHover[4] = { 0.f, 0.f, 0.f, 0.08f };
		float colorOnClick[4] = { 0.f, 0.f, 0.f, 0.16f };
		std::string scriptNameOnHover, scriptNameOnClick;
		std::string funcOnHover, funcOnClick;
		// === image sprite only ===
		std::string imgPath; 
		float w, h;
		// === text sprite only ===
		std::string text; 
		// === text & dyanmic text sprite ===
		float fontSize;	
		float color[4] = { 1.f, 1.f, 1.f, 1.f };
		TextSprite2D::HorizontalAlignType alignType;
		std::string fontName;
		// === dynamic text only ===
		std::string sname;
		std::string vname;
		DynamicTextSprite2D::SupportedVarType vtype;

		SpriteInfo() {}
		SpriteInfo(Sprite2D::SpriteType type, float x, float y) : type(type), x(x), y(y) {}
		template <class Archive> void save(Archive & ar) const {
			ar(type);
			ar(x, y);
			ar(z);
			ar(alpha);
			ar(isButton);
			if (isButton) {
				ar(colorOnHover, colorOnClick);
				ar(scriptNameOnHover, scriptNameOnClick, funcOnHover, funcOnClick);
			}
			switch (type) {
			case Sprite2D::SpriteType::ImageSprite:
				ar(imgPath);
				ar(w, h);
				break;
			case Sprite2D::SpriteType::TextSprite:
				ar(text);
				// no break is correct
			case Sprite2D::SpriteType::DynamicTextSprite:
				ar(fontSize);
				ar(color);
				ar(fontName, sname, vname);
				ar(vtype);
				break;
			}			
		}
		template <class Archive> void load(Archive & ar) {
			ar(type);
			ar(x, y);
			ar(z);
			ar(alpha);
			ar(isButton);
			if (isButton) {
				ar(colorOnHover, colorOnClick);
				ar(scriptNameOnHover, scriptNameOnClick, funcOnHover, funcOnClick);
			}
			switch (type) {
			case Sprite2D::SpriteType::ImageSprite:
				ar(imgPath);
				ar(w, h);
				break;
			case Sprite2D::SpriteType::TextSprite:
				ar(text);
				// no break is correct
			case Sprite2D::SpriteType::DynamicTextSprite:
				ar(fontSize);
				ar(color);
				ar(fontName, sname, vname);
				ar(vtype);
				break;
			}
		}
	};

	class GameUI {
	private:
		std::vector<Sprite2D*> spriteList;	// a list of sprites sorted by z-values
	public:
		GameUI();
		~GameUI();
		void SortSprites();				// to sort spriteList in DESCENDING ORDER
		void AddSprite(Sprite2D* sprite);		
		void RemoveSprite(Sprite2D* sprite);
		void GoFront(Sprite2D* sprite); // move the item to the front by one layer
		void GoBack(Sprite2D* sprite);	// move the item to the back by one layer
		std::vector<Sprite2D*>& GetSpriteList();

		void Serialize(cereal::BinaryOutputArchive& oarchive);
		void Deserialize(cereal::BinaryInputArchive& iarchive);
	private:
		friend class cereal::access;
		template <class Archive> void save(Archive & ar) const {
			// serialize UI
			std::vector<SpriteInfo> spritesInfo;
			// store information for construction
			for (Sprite2D* sprite : spriteList) {
				SpriteInfo info(sprite->spriteType, sprite->x, sprite->y);
				info.alpha = sprite->alpha;
				switch (sprite->spriteType) {
				case Sprite2D::SpriteType::ImageSprite:
				{
					ImageSprite2D* tsprite = dynamic_cast<ImageSprite2D*>(sprite);
					info.imgPath = tsprite->GetPath();
					info.z = tsprite->GetZIndex();
					info.w = tsprite->w; info.h = tsprite->h;
					info.isButton = tsprite->isButton;
					if (info.isButton) {
						std::copy(tsprite->colorOnHover, tsprite->colorOnHover + 4, info.colorOnHover);
						std::copy(tsprite->colorOnClick, tsprite->colorOnClick + 4, info.colorOnClick);
						info.scriptNameOnHover = tsprite->scriptNameOnHover;
						info.scriptNameOnClick = tsprite->scriptNameOnClick;
					}
					break;
				}
				case Sprite2D::SpriteType::TextSprite:
				{
					TextSprite2D* tsprite = dynamic_cast<TextSprite2D*>(sprite);
					info.text = tsprite->GetText();
					info.fontSize = tsprite->GetFontSize();
					info.fontName = tsprite->GetFontName();
					info.alignType = tsprite->alignType;
					std::copy(tsprite->color, tsprite->color + 4, info.color);
					break;
				}
				case Sprite2D::SpriteType::DynamicTextSprite:
				{
					DynamicTextSprite2D* tsprite = dynamic_cast<DynamicTextSprite2D*>(sprite);
					info.fontSize = tsprite->GetFontSize();
					info.fontName = tsprite->GetFontName();
					info.alignType = tsprite->alignType;
					std::copy(tsprite->color, tsprite->color + 4, info.color);
					info.sname = tsprite->GetScriptName();
					info.vname = tsprite->GetVarName();
					info.vtype = tsprite->GetVarType();
					break;
				}
				}
				spritesInfo.push_back(info);
			}				
			ar(spritesInfo);
		}

		template <class Archive> void load(Archive & ar) {
			// deserialize UI
			std::vector<SpriteInfo> spritesInfo;
			try {
				ar(spritesInfo);
			}
			catch (std::exception e) {
				LOG("Deserializing GameUI failed, reason: {}", e.what());
				return;
			}
			Config& config = Application::GetInstance()->GetConfig();
			for (SpriteInfo& info : spritesInfo) {
				switch (info.type) {
				case Sprite2D::SpriteType::ImageSprite:
				{
					ImageSprite2D* sprite = new ImageSprite2D(info.imgPath.c_str(), config.width, config.height, info.x, info.y);
					sprite->alpha = info.alpha;
					sprite->z = info.z;
					sprite->w = info.w; sprite->h = info.h;
					sprite->isButton = info.isButton;
					if (info.isButton) {
						std::copy(info.colorOnHover, info.colorOnHover + 4, sprite->colorOnHover);
						std::copy(info.colorOnClick, info.colorOnClick + 4, sprite->colorOnClick);
						sprite->scriptNameOnHover = info.scriptNameOnHover;
						sprite->scriptNameOnClick  = info.scriptNameOnClick;
					}					
					AddSprite(sprite);
					break;
				}
				case Sprite2D::SpriteType::TextSprite:
				{
					TextSprite2D* sprite = new TextSprite2D(info.text.c_str(), info.fontName.c_str(), info.fontSize, config.width, config.height, info.x, info.y);
					sprite->alpha = info.alpha;
					sprite->alignType = info.alignType;
					std::copy(info.color, info.color + 4, sprite->color);
					AddSprite(sprite);
					sprite->getTexture(true);
					break;
				}
				case Sprite2D::SpriteType::DynamicTextSprite:
				{
					DynamicTextSprite2D* sprite = new DynamicTextSprite2D(info.sname.c_str(), info.vname.c_str(), info.vtype, info.fontName.c_str(), info.fontSize,
						config.width, config.height, info.x, info.y);
					sprite->alpha = info.alpha;
					sprite->alignType = info.alignType;
					std::copy(info.color, info.color + 4, sprite->color);
					AddSprite(sprite);
					sprite->getTexture(true);
					break;
				}
				}				
			}
		}
	};

}
