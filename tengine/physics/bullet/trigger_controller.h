#pragma once

#include "LinearMath/btVector3.h"

#include "BulletDynamics/Dynamics/btActionInterface.h"
#include "BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h"

#include <game/entityhandle.h>
#include <physics/physics.h>

class btPairCachingGhostObject;

class CTriggerController : public btActionInterface
{
public:
	CTriggerController(IPhysicsEntity* pEntity, btPairCachingGhostObject* ghostObject);

public:
	// btActionInterface
	virtual void	updateAction(btCollisionWorld* collisionWorld, btScalar deltaTime);
	void			debugDraw(btIDebugDraw* debugDrawer) {};

	btPairCachingGhostObject* GetGhostObject();

	IPhysicsEntity* GetEntity() const;

protected:
	btManifoldArray				m_aManifolds;

	IPhysicsEntity*             m_pEntity;

	btPairCachingGhostObject*	m_pGhostObject;
};
