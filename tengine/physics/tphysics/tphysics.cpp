#include "tphysics.h"

CTPhysics::CTPhysics()
{
}

CTPhysics::~CTPhysics()
{
}

CPhysicsModel* CreatePhysicsModel()
{
	return new CTPhysics();
}

