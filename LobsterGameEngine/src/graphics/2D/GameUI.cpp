#include "pch.h"
#include "GameUI.h"

namespace Lobster {

	GameUI::GameUI() {
		
	}

	GameUI::~GameUI() {

	}

	void GameUI::SortSprites() {
		if (spriteList.size() <= 1) return;
		std::sort(spriteList.begin(), spriteList.end(), Sprite2D::Compare);
	}

	void GameUI::AddSprite(Sprite2D* sprite) {
		spriteList.push_back(sprite);
		SortSprites();
	}

	void GameUI::RemoveSprite(Sprite2D* sprite) {
		spriteList.erase(std::find(spriteList.begin(), spriteList.end(), sprite));
		delete sprite;
	}

	void GameUI::GoFront(Sprite2D* sprite) {
		auto it = std::find(spriteList.begin(), spriteList.end(), sprite);
		if ((it + 1) != spriteList.end()) {
			int temp = (*it)->GetZIndex();
			(*it)->SetZIndex((*(it + 1))->GetZIndex());
			(*(it + 1))->SetZIndex(temp);
			std::iter_swap(it, it + 1);
		}
		else LOG("Sprite is already at the top");
	}

	void GameUI::GoBack(Sprite2D* sprite) {
		auto it = std::find(spriteList.begin(), spriteList.end(), sprite);
		if (it != spriteList.begin()) {
			int temp = (*it)->GetZIndex();
			(*it)->SetZIndex((*(it - 1))->GetZIndex());
			(*(it - 1))->SetZIndex(temp);
			std::iter_swap(it, it - 1);
		}
		else LOG("Sprite is already at the bottom");
	}

	std::vector<Sprite2D*>& GameUI::GetSpriteList() {
		return spriteList;
	}

	void GameUI::Serialize(cereal::JSONOutputArchive& oarchive) {
		oarchive(*this);
	}

	void GameUI::Deserialize(cereal::JSONInputArchive& iarchive) {
		try {
			iarchive(*this);
		}
		catch (std::exception e) {
			LOG("Deserializing GameUI failed, reason: {}", e.what());
		}
	}
}