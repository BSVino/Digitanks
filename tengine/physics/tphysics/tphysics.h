#pragma once

#include "../physics.h"

class CTPhysics : public CPhysicsModel
{
public:
	CTPhysics();
	~CTPhysics();

public:
	virtual void    AddEntity(IPhysicsEntity* pEnt, collision_type_t eCollisionType) { TUnimplemented(); };
	virtual void    RemoveEntity(IPhysicsEntity* pEnt) { TUnimplemented(); };
	virtual size_t  AddExtra(size_t iExtraMesh, const Vector& vecOrigin) { TUnimplemented(); return ~0; };  // Input is result from LoadExtraCollisionMesh
	virtual size_t  AddExtraBox(const Vector& vecCenter, const Vector& vecSize) { TUnimplemented(); return ~0; };
	virtual void    RemoveExtra(size_t iExtra) { TUnimplemented(); };               // Input is result from AddExtra*
	virtual void    RemoveAllEntities() { TUnimplemented(); };
	virtual bool    IsEntityAdded(IPhysicsEntity* pEnt) { TUnimplemented(); return false; };

	virtual void    LoadCollisionMesh(const tstring& sModel, bool bConcave, size_t iTris, const int* aiTris, size_t iVerts, const float* aflVerts) { TUnimplemented(); };
	virtual void    UnloadCollisionMesh(const tstring& sModel) { TUnimplemented(); };
	virtual size_t  LoadExtraCollisionMesh(size_t iTris, bool bConcave, int* aiTris, size_t iVerts, float* aflVerts) { TUnimplemented(); return ~0; };
	virtual void    UnloadExtraCollisionMesh(size_t iMesh) { TUnimplemented(); };

	virtual void Simulate() { TUnimplemented(); };

	virtual void DebugDraw(int iLevel) { TUnimplemented(); };

	virtual collision_type_t GetEntityCollisionType(IPhysicsEntity* pEnt) { TUnimplemented(); return CT_NONE; };

	virtual bool    IsEntityCollisionDisabled(IPhysicsEntity* pEnt) { TUnimplemented(); return true; };
	virtual void    SetEntityCollisionDisabled(IPhysicsEntity* pEnt, bool bDisabled) { TUnimplemented(); };
	virtual void    SetEntityTransform(IPhysicsEntity* pEnt, const Matrix4x4& mTransform) { TUnimplemented(); };
	virtual void    SetEntityVelocity(IPhysicsEntity* pEnt, const Vector& vecVelocity) { TUnimplemented(); };
	virtual Vector  GetEntityVelocity(IPhysicsEntity* pEnt) { TUnimplemented(); return Vector(0, 0, 0); };
	virtual void    SetControllerMoveVelocity(IPhysicsEntity* pEnt, const Vector& vecVelocity) { TUnimplemented(); };
	virtual const Vector GetControllerMoveVelocity(IPhysicsEntity* pEnt) { TUnimplemented(); return Vector(0, 0, 0); }
	virtual void    SetControllerColliding(IPhysicsEntity* pEnt, bool bColliding) { TUnimplemented(); };
	virtual void    SetEntityGravity(IPhysicsEntity* pEnt, const Vector& vecGravity) { TUnimplemented(); };
	virtual void    SetEntityUpVector(IPhysicsEntity* pEnt, const Vector& vecUp) { TUnimplemented(); };
	virtual void    SetLinearFactor(IPhysicsEntity* pEnt, const Vector& vecFactor) { TUnimplemented(); };
	virtual void    SetAngularFactor(IPhysicsEntity* pEnt, const Vector& vecFactor) { TUnimplemented(); };

	virtual void CharacterMovement(IPhysicsEntity* pEnt, class btCollisionWorld* pCollisionWorld, float flDelta) { TUnimplemented(); };

	virtual void TraceLine(CTraceResult& tr, const Vector& v1, const Vector& v2, collision_group_t eCollisions = CG_ALL, IPhysicsEntity* pIgnore = nullptr) { TUnimplemented(); };
	virtual void TraceEntity(CTraceResult& tr, IPhysicsEntity* pEntity, const Vector& v1, const Vector& v2, collision_group_t eCollisions = CG_ALL) { TUnimplemented(); };
	virtual void CheckSphere(CTraceResult& tr, float flRadius, const Vector& vecCenter, IPhysicsEntity* pIgnore = nullptr) { TUnimplemented(); };

	virtual void CharacterJump(IPhysicsEntity* pEnt) { TUnimplemented(); };
};
