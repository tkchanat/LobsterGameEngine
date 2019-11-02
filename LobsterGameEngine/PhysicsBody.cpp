#include "pch.h"
#include "physics/PhysicsBody.h"

//	Thanks C++ static member initialization rules for forcing me to open a new .cpp file just for this line...
namespace Lobster {
	const char* PhysicsBody::PhysicsBodyTypes[] = { "Rigid", "Others" };
}