#pragma once
#include "objects/Transform.h"

namespace Lobster {
	class BoundingBox {
	public:
		virtual void OnUpdate(Transform* t) = 0;

	protected:
		virtual void Draw() = 0;
	};
}