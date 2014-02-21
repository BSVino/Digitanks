#ifndef TINKER_BULLET_PHYSICS_H
#define TINKER_BULLET_PHYSICS_H

#include "physics.h"

#include <btBulletDynamicsCommon.h>

#include <game/entityhandle.h>

#include "character_controller.h"
#include "trigger_controller.h"

inline btVector3 ToBTVector(const Vector& v)
{
	return btVector3(v.x, v.y, v.z);
}

inline Vector ToTVector(const btVector3& v)
{
	return Vector(v.x(), v.y(), v.z());
}

class CClosestRayResultCallback : public btCollisionWorld::ClosestRayResultCallback
{
public:
	CClosestRayResultCallback(const btVector3& rayFromWorld, const btVector3& rayToWorld, btCollisionObject* pIgnore=nullptr)
		: btCollisionWorld::ClosestRayResultCallback(rayFromWorld, rayToWorld)
	{
		m_pIgnore = pIgnore;
	}

	virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
	{
		if (rayResult.m_collisionObject == m_pIgnore)
			return 1.0;

		return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
	}

protected:
	btCollisionObject*     m_pIgnore;
};

class CClosestConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback
{
public:
	CClosestConvexResultCallback(const btVector3& rayFromWorld, const btVector3& rayToWorld, btCollisionObject* pIgnore=nullptr)
		: btCollisionWorld::ClosestConvexResultCallback(rayFromWorld, rayToWorld)
	{
		m_pIgnore = pIgnore;
	}

	virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)
	{
		if (convexResult.m_hitCollisionObject == m_pIgnore)
			return 1.0;

		return btCollisionWorld::ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
	}

protected:
	btCollisionObject*     m_pIgnore;
};

class CAllContactResultsCallback : public btCollisionWorld::ContactResultCallback
{
public:
	CAllContactResultsCallback(CTraceResult& tr, btCollisionObject* pIgnore=nullptr)
		: m_tr(tr), btCollisionWorld::ContactResultCallback()
	{
		m_pIgnore = pIgnore;
	}

	virtual btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0,int partId0,int index0,const btCollisionObjectWrapper* colObj1,int partId1,int index1)
	{
		if (colObj0->getCollisionObject() == m_pIgnore)
			return 1.0;

		if (colObj1->getCollisionObject() == m_pIgnore)
			return 1.0;

		CTraceResult::CTraceHit& th = m_tr.m_aHits.push_back();

		th.m_flFraction = cp.getDistance();
		th.m_iHit = (size_t)colObj1->getCollisionObject()->getUserPointer();
		th.m_vecHit = ToTVector(cp.getPositionWorldOnB());

		if (th.m_flFraction < m_tr.m_flFraction)
		{
			m_tr.m_flFraction = th.m_flFraction;
			m_tr.m_iHit = th.m_iHit;
			m_tr.m_vecHit = th.m_vecHit;
		}

		return 0.0;
	}

protected:
	CTraceResult&        m_tr;

	btCollisionObject*   m_pIgnore;
};

class CMotionState : public btMotionState
{
public:
	CMotionState()
	{
		m_pPhysics = NULL;
	}

public:
	virtual void getWorldTransform(btTransform& mCenterOfMass) const;

	virtual void setWorldTransform(const btTransform& mCenterOfMass);

public:
	class CBulletPhysics*			m_pPhysics;
	IPhysicsEntity*                 m_pEntity;
};

class CPhysicsEntity
{
public:
	CPhysicsEntity()
	{
		m_pRigidBody = NULL;
		m_pExtraShape = NULL;
		m_pGhostObject = NULL;
		m_pCharacterController = NULL;
		m_pTriggerController = NULL;
		m_bCenterMassOffset = true;
		m_eCollisionType = CT_NONE;
		m_bCollisionDisabled = true;
	};

	~CPhysicsEntity()
	{
	};

public:
	btRigidBody*						m_pRigidBody;
	btCollisionShape*                   m_pExtraShape;
	tvector<btRigidBody*>				m_apAreaBodies;
	tvector<btRigidBody*>				m_apPhysicsShapes;
	tvector<btRigidBody*>               m_apPhysicsHulls;
	class btPairCachingGhostObject*		m_pGhostObject;
	class CCharacterController*         m_pCharacterController;
	CTriggerController*					m_pTriggerController;
	CMotionState						m_oMotionState;
	bool								m_bCenterMassOffset;
	bool                                m_bCollisionDisabled;
	collision_type_t					m_eCollisionType;
};

class CBulletPhysics : public CPhysicsModel
{
public:
							CBulletPhysics();
							~CBulletPhysics();

public:
	virtual void			AddEntity(IPhysicsEntity* pEnt, collision_type_t eCollisionType);
	virtual void            AddHull(IPhysicsEntity* pEnt, collision_type_t eCollisionType);
	virtual void			AddModel(IPhysicsEntity* pEnt, collision_type_t eCollisionType, size_t iModel);
	virtual void			AddModelTris(IPhysicsEntity* pEnt, collision_type_t eCollisionType, size_t iModel);
	virtual void			RemoveEntity(IPhysicsEntity* pEnt);
	virtual void			RemoveEntity(CPhysicsEntity* pEntity);
	virtual size_t          AddExtra(size_t iExtraMesh, const Vector& vecOrigin);  // Input is result from LoadExtraCollisionMesh
	virtual size_t          AddExtraBox(const Vector& vecCenter, const Vector& vecSize);
	virtual void            RemoveExtra(size_t iExtra);   // Input is result from AddExtra*
	virtual void			RemoveAllEntities();
	virtual bool            IsEntityAdded(IPhysicsEntity* pEnt);

	virtual void			LoadCollisionMesh(const tstring& sModel, size_t iTris, const int* aiTris, size_t iVerts, const float* aflVerts);
	virtual void			UnloadCollisionMesh(const tstring& sModel);
	virtual size_t          LoadExtraCollisionMesh(size_t iTris, int* aiTris, size_t iVerts, float* aflVerts);
	virtual void			UnloadExtraCollisionMesh(size_t iMesh);

	virtual void			Simulate();

	virtual void			DebugDraw(int iLevel);

	virtual collision_type_t	GetEntityCollisionType(IPhysicsEntity* pEnt);

	virtual bool            IsEntityCollisionDisabled(IPhysicsEntity* pEnt);
	virtual void            SetEntityCollisionDisabled(IPhysicsEntity* pEnt, bool bDisabled);
	virtual void			SetEntityTransform(IPhysicsEntity* pEnt, const Matrix4x4& mTransform);
	virtual void			SetEntityTransform(IPhysicsEntity* pEnt, const Matrix4x4& mTransform, bool bUpdateAABB);
	virtual void			SetEntityVelocity(IPhysicsEntity* pEnt, const Vector& vecVelocity);
	virtual Vector			GetEntityVelocity(IPhysicsEntity* pEnt);
	virtual void			SetControllerMoveVelocity(IPhysicsEntity* pEnt, const Vector& vecVelocity);
	virtual const Vector    GetControllerMoveVelocity(IPhysicsEntity* pEnt);
	virtual void			SetControllerColliding(IPhysicsEntity* pEnt, bool bColliding);
	virtual void			SetEntityGravity(IPhysicsEntity* pEnt, const Vector& vecGravity);
	virtual void			SetEntityUpVector(IPhysicsEntity* pEnt, const Vector& vecUp);
	virtual void			SetLinearFactor(IPhysicsEntity* pEnt, const Vector& vecFactor);
	virtual void			SetAngularFactor(IPhysicsEntity* pEnt, const Vector& vecFactor);

	virtual void            CharacterMovement(IPhysicsEntity* pEnt, class btCollisionWorld* pCollisionWorld, float flDelta);

	virtual void            TraceLine(CTraceResult& tr, const Vector& v1, const Vector& v2, collision_group_t eCollisions = CG_ALL, IPhysicsEntity* pIgnore=nullptr);
	virtual void            TraceEntity(CTraceResult& tr, IPhysicsEntity* pEntity, const Vector& v1, const Vector& v2, collision_group_t eCollisions = CG_ALL);
	virtual void            CheckSphere(CTraceResult& tr, float flRadius, const Vector& vecCenter, IPhysicsEntity* pIgnore=nullptr);

	virtual void			CharacterJump(IPhysicsEntity* pEnt);

	virtual CPhysicsEntity* GetPhysicsEntity(IPhysicsEntity* pEnt);

	short                   GetMaskForGroup(collision_group_t eGroup);

protected:
	tvector<CPhysicsEntity>					m_aEntityList;
	tvector<CPhysicsEntity*>                m_apExtraEntityList;

	btDefaultCollisionConfiguration*		m_pCollisionConfiguration;
	btCollisionDispatcher*					m_pDispatcher;
	btDbvtBroadphase*						m_pBroadphase;
	class btGhostPairCallback*				m_pGhostPairCallback;
	btDiscreteDynamicsWorld*				m_pDynamicsWorld;

	class CPhysicsDebugDrawer*				m_pDebugDrawer;

	class CCollisionMesh
	{
	public:
		btTriangleIndexVertexArray*				m_pIndexVertexArray;
		btCollisionShape*						m_pCollisionShape;
	};

	static tmap<size_t, CCollisionMesh>     s_apCollisionMeshes;
	static tvector<CCollisionMesh*>         s_apExtraCollisionMeshes;
	static tmap<tstring, btConvexShape*>    s_apCharacterShapes;
};

#endif
