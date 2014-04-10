#pragma once

#include "../physics.h"

class CPhysicsMesh
{

};

class CPhysicsEntity
{
public:
	CPhysicsEntity()
	{
		m_pGameEntity = nullptr;
		m_bActive = false;
	};

	CPhysicsEntity(IPhysicsEntity* pEntity, collision_type_t eCollisionType)
	{
		m_pGameEntity = pEntity;
		m_eCollisionType = eCollisionType;
		m_bActive = true;
	};

	~CPhysicsEntity()
	{
	};

public:
	IPhysicsEntity*  m_pGameEntity;
	collision_type_t m_eCollisionType;
	CPhysicsMesh*    m_pMesh;

	bool m_bActive;

	Vector m_vecVelocity;
	Vector m_vecGravity;
};

class CTPhysics : public CPhysicsModel
{
public:
	CTPhysics();
	~CTPhysics();

public:
	virtual void    AddEntity(IPhysicsEntity* pEnt, collision_type_t eCollisionType);
	virtual void    RemoveEntity(IPhysicsEntity* pEnt);
	virtual void    RemoveEntity(CPhysicsEntity* pEntity);
	virtual size_t  AddExtra(size_t iExtraMesh, const Vector& vecOrigin);
	virtual size_t  AddExtraBox(const Vector& vecCenter, const Vector& vecSize) { TUnimplemented(); return ~0; };
	virtual void    RemoveExtra(size_t iExtra);
	virtual void    RemoveAllEntities();
	virtual bool    IsEntityAdded(IPhysicsEntity* pEnt) { TUnimplemented(); return false; };

	virtual void    LoadCollisionMesh(const tstring& sModel, mesh_type_t eMeshType, size_t iTris, const int* aiTris, size_t iVerts, const float* aflVerts) { TUnimplemented(); };
	virtual void    UnloadCollisionMesh(const tstring& sModel) { TUnimplemented(); };
	virtual size_t  LoadExtraCollisionMesh(mesh_type_t eMeshType, size_t iTris, int* aiTris, size_t iVerts, float* aflVerts) { TUnimplemented(); return ~0; };
	virtual size_t  LoadExtraCollisionHeightmapMesh(size_t iWidth, size_t iHeight, float* aflVerts);
	virtual void    UnloadExtraCollisionMesh(size_t iMesh);

	virtual void Simulate();

	virtual void DebugDraw(int iLevel) { TUnimplemented(); };

	virtual collision_type_t GetEntityCollisionType(IPhysicsEntity* pEnt) { TUnimplemented(); return CT_NONE; };

	virtual bool    IsEntityCollisionDisabled(IPhysicsEntity* pEnt) { TUnimplemented(); return true; };
	virtual void    SetEntityCollisionDisabled(IPhysicsEntity* pEnt, bool bDisabled);
	virtual void    SetEntityTransform(IPhysicsEntity* pEnt, const Matrix4x4& mTransform);
	virtual void    SetEntityVelocity(IPhysicsEntity* pEnt, const Vector& vecVelocity);
	virtual Vector  GetEntityVelocity(IPhysicsEntity* pEnt);
	virtual void    SetControllerMoveVelocity(IPhysicsEntity* pEnt, const Vector& vecVelocity) { TUnimplemented(); };
	virtual const Vector GetControllerMoveVelocity(IPhysicsEntity* pEnt) { TUnimplemented(); return Vector(0, 0, 0); }
	virtual void    SetControllerColliding(IPhysicsEntity* pEnt, bool bColliding) { TUnimplemented(); };
	virtual void    SetEntityGravity(IPhysicsEntity* pEnt, const Vector& vecGravity);
	virtual void    SetEntityUpVector(IPhysicsEntity* pEnt, const Vector& vecUp) { TUnimplemented(); };
	virtual void    SetLinearFactor(IPhysicsEntity* pEnt, const Vector& vecFactor) { TUnimplemented(); };
	virtual void    SetAngularFactor(IPhysicsEntity* pEnt, const Vector& vecFactor) { TUnimplemented(); };

	virtual void CharacterMovement(IPhysicsEntity* pEnt, class btCollisionWorld* pCollisionWorld, float flDelta) { TUnimplemented(); };

	virtual void TraceLine(CTraceResult& tr, const Vector& v1, const Vector& v2, collision_group_t eCollisions = CG_ALL, IPhysicsEntity* pIgnore = nullptr) { TUnimplemented(); };
	virtual void TraceEntity(CTraceResult& tr, IPhysicsEntity* pEntity, const Vector& v1, const Vector& v2, collision_group_t eCollisions = CG_ALL) { TUnimplemented(); };
	virtual void CheckSphere(CTraceResult& tr, float flRadius, const Vector& vecCenter, IPhysicsEntity* pIgnore = nullptr) { TUnimplemented(); };

	virtual void CharacterJump(IPhysicsEntity* pEnt) { TUnimplemented(); };

	CPhysicsEntity* GetPhysicsEntity(IPhysicsEntity* pEnt);

private:
	tvector<CPhysicsEntity>  m_aEntityList;
	tvector<CPhysicsEntity*> m_apExtraEntityList;
	tvector<CPhysicsEntity*> m_apSimulateList;

	class CCollisionMesh
	{
	public:
		CPhysicsMesh* m_pMesh;
	};

	tvector<CCollisionMesh*> m_apExtraCollisionMeshes;

	double m_flSimulationTime;
	double m_flGameTime;
	double m_flServerTime;
};
