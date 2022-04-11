#pragma once
#include "GameObject.h"
#include "../GameTech/CourseworkGame.h"
namespace NCL {
	namespace CSC8503 {
		class Bonus : public GameObject {
		public:
			Bonus(CourseworkGame* g) : GameObject() {
				game = g;
			}
			void OnCollisionBegin(GameObject* otherObject) override {
				if (otherObject->GetWorldID() == 1 || otherObject->GetWorldID() == 7) {
					game->OnBonusCollect();
					isActive = false; //should maybe delete bonus after collection?
				}
			}
		protected:
			CourseworkGame* game;
		};
	}
}