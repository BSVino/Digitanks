#include "trigger_controller.h"

#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletCollision/BroadphaseCollision/btOverlappingPairCache.h"
#include "BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h"
#include "BulletCollision/CollisionDispatch/btCollisionWorld.h"
#include "LinearMath/btDefaultMotionState.h"

#include <game/entities/baseentity.h>
#include <tinker/application.h>

#include "bullet_physics.h"

CTriggerController::CTriggerController(IPhysicsEntity* pEntity, btPairCachingGhostObject* ghostObject)
{
	m_pEntity = pEntity;
	m_pGhostObject = ghostObject;
}

void CTriggerController::updateAction(btCollisionWorld* pCollisionWorld, btScalar flDeltaTime)
{
	m_pEntity->BeginTouchingList();

	pCollisionWorld->getDispatcher()->dispatchAllCollisionPairs(m_pGhostObject->getOverlappingPairCache(), pCollisionWorld->getDispatchInfo(), pCollisionWorld->getDispatcher());

	btScalar maxPen = btScalar(0.0);
	for (int i = 0; i < m_pGhostObject->getOverlappingPairCache()->getNumOverlappingPairs(); i++)
	{
		m_aManifolds.resize(0);

		btBroadphasePair* pCollisionPair = &m_pGhostObject->getOverlappingPairCache()->getOverlappingPairArray()[i];

		if (pCollisionPair->m_algorithm)
			pCollisionPair->m_algorithm->getAllContactManifolds(m_aManifolds);

		for (int j = 0; j < m_aManifolds.size(); j++)
		{
			btPersistentManifold* pManifold = m_aManifolds[j];

			const btCollisionObject* obA = static_cast<const btCollisionObject*>(pManifold->getBody0());
			const btCollisionObject* obB = static_cast<const btCollisionObject*>(pManifold->getBody1());

			btScalar directionSign;
			size_t iOther;
			if (obA == m_pGhostObject)
			{
				directionSign = btScalar(-1.0);
				iOther = (size_t)obB->getUserPointer();
			}
			else
			{
				directionSign = btScalar(1.0);
				iOther = (size_t)obA->getUserPointer();
			}

			bool bTouching = false;

			for (int p = 0; p < pManifold->getNumContacts(); p++)
			{
				const btManifoldPoint& pt = pManifold->getContactPoint(p);

				if (obA == m_pGhostObject)
				{
					if (!m_pEntity->ShouldCollideWith(iOther, Vector(pt.getPositionWorldOnB())))
						continue;
				}
				else
				{
					if (!m_pEntity->ShouldCollideWith(iOther, Vector(pt.getPositionWorldOnA())))
						continue;
				}

				if (pt.getDistance() < 0)
				{
					bTouching = true;
					break;
				}
			}

			if (bTouching)
				m_pEntity->Touching(iOther);

			//pManifold->clearManifold();
		}
	}

	m_pEntity->EndTouchingList();
}

btPairCachingGhostObject* CTriggerController::GetGhostObject()
{
	return m_pGhostObject;
}

IPhysicsEntity* CTriggerController::GetEntity() const
{
	return m_pEntity;
}
