#pragma once
#include <glm/vec3.hpp>
#include "physics/PhysicsComponent.h"

//	This class keep tracks of all active PhysicsComponent on the scene.
//	We only friended PhysicsComponent - this class would be meaningless to other classes.
//	Other classes could only access a GetInstance() static function alone.
namespace Lobster {
	class PhysicsSystem {
		friend class PhysicsComponent;
		friend class Rigidbody;

	private:
		static PhysicsSystem* m_instance;
		std::vector<PhysicsComponent*> compsList;
		inline void AddPhysicsComp(PhysicsComponent* comp) {
			compsList.push_back(comp);
		};
		inline void RemovePhysicsComp(PhysicsComponent* comp) {
			auto index = std::find(compsList.begin(), compsList.end(), comp);
			if (index != compsList.end()) {
				compsList.erase(index);
			}
		};

	public:
		PhysicsSystem();

		inline static PhysicsSystem* GetInstance() { return m_instance; }
	};
}