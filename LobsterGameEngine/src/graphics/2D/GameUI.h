#pragma once
#include "Sprite.h"

namespace Lobster {

	class Sprite2D;
	class TextSprite2D;

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
	};
}
