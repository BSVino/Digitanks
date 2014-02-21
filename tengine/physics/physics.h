#ifndef TINKER_PHYSICS_H
#define TINKER_PHYSICS_H

#include <tengine_config.h>

#include <matrix.h>

#include <models/models.h>

typedef enum collision_type_e
{
	CT_NONE = 0,
	CT_STATIC_MESH,	// Not animated, never moves. World geometry.
	CT_KINEMATIC,	// Not simulated by the engine, but collides with dynamic objects. Animated externally by game code.
	CT_CHARACTER,	// Kinematically animated character controller.
	CT_TRIGGER,		// Does not collide, but reports intersections.
} collision_type_t;

typedef enum
{
	CG_NONE = 0,
	CG_DEFAULT = (1<<0),
	CG_STATIC = (1<<1),
	CG_CHARACTER = (1<<2),
	CG_TRIGGER = (1<<3),
	CG_ALL = ~0,
} collision_group_t;

class IPhysicsEntity
{
public:
	virtual ~IPhysicsEntity() {};

public:
	virtual size_t            GetHandle() const { return ~0; }
	virtual const tstring&    GetName() const { static tstring s; return s; };
	virtual class CModel*     GetModel() const { return nullptr; }
	virtual const char*       GetClassName() const { return ""; }
	virtual size_t            GetModelID() const { return ~0; };
	virtual collision_group_t GetCollisionGroup() const { return CG_DEFAULT; }
	virtual const Vector      GetScale() const { return Vector(); }

	virtual const AABB        GetPhysBoundingBox() const { return AABB(); }
	virtual const AABB        GetVisBoundingBox() const { static AABB r; return r; }
	virtual const TVector     GetGlobalOrigin() const { return TVector(); }
	virtual const Matrix4x4   GetPhysicsTransform() const { return Matrix4x4(); }
	virtual void              SetPhysicsTransform(const Matrix4x4& m) { }

	// Triggers
	virtual void              Touching(size_t iOtherHandle) {};
	virtual void              BeginTouchingList() {};
	virtual void              EndTouchingList() {};
	virtual bool              ShouldCollideWith(size_t iOtherHandle, const TVector& vecPoint) const { return true; }
};

class CTraceResult
{
public:
	CTraceResult()
	{
		m_flFraction = 1.0f;
		m_iHit = ~0;
		m_iHitExtra = ~0;
	}

public:
	class CTraceHit
	{
	public:
		CTraceHit()
		{
			m_flFraction = 1.0f;
			m_iHit = ~0;
			m_iHitExtra = ~0;
		}

	public:
		float              m_flFraction;
		Vector             m_vecHit;
		size_t             m_iHit;
		size_t             m_iHitExtra;
	};

	// All hits.
	tvector<CTraceHit>     m_aHits;

	// Nearest result.
	float                  m_flFraction;
	Vector                 m_vecHit;
	Vector                 m_vecNormal;
	size_t                 m_iHit;
	size_t                 m_iHitExtra;
};

class CPhysicsModel
{
public:
	virtual					~CPhysicsModel() {}

public:
	virtual void			AddEntity(IPhysicsEntity* pEnt, collision_type_t eCollisionType) {};
	virtual void			RemoveEntity(IPhysicsEntity* pEnt) {};
	virtual size_t          AddExtra(size_t iExtraMesh, const Vector& vecOrigin) { return ~0; };  // Input is result from LoadExtraCollisionMesh
	virtual size_t          AddExtraBox(const Vector& vecCenter, const Vector& vecSize) { return ~0; };
	virtual void            RemoveExtra(size_t iExtra) {};               // Input is result from AddExtra*
	virtual void			RemoveAllEntities() {};
	virtual bool            IsEntityAdded(IPhysicsEntity* pEnt) { return false; };

	virtual void			LoadCollisionMesh(const tstring& sModel, size_t iTris, const int* aiTris, size_t iVerts, const float* aflVerts) {};
	virtual void			UnloadCollisionMesh(const tstring& sModel) {};
	virtual size_t          LoadExtraCollisionMesh(size_t iTris, int* aiTris, size_t iVerts, float* aflVerts) { return ~0; };
	virtual void            UnloadExtraCollisionMesh(size_t iMesh) {};

	virtual void			Simulate() {};

	virtual void			DebugDraw(int iLevel) {};

	virtual collision_type_t	GetEntityCollisionType(IPhysicsEntity* pEnt) { return CT_NONE; };

	virtual bool            IsEntityCollisionDisabled(IPhysicsEntity* pEnt) { return true; };
	virtual void            SetEntityCollisionDisabled(IPhysicsEntity* pEnt, bool bDisabled) {};
	virtual void			SetEntityTransform(IPhysicsEntity* pEnt, const Matrix4x4& mTransform) {};
	virtual void			SetEntityVelocity(IPhysicsEntity* pEnt, const Vector& vecVelocity) {};
	virtual Vector			GetEntityVelocity(IPhysicsEntity* pEnt) { return Vector(0, 0, 0); };
	virtual void			SetControllerMoveVelocity(IPhysicsEntity* pEnt, const Vector& vecVelocity) {};
	virtual const Vector    GetControllerMoveVelocity(IPhysicsEntity* pEnt) { return Vector(0, 0, 0); }
	virtual void			SetControllerColliding(IPhysicsEntity* pEnt, bool bColliding) {};
	virtual void			SetEntityGravity(IPhysicsEntity* pEnt, const Vector& vecGravity) {};
	virtual void			SetEntityUpVector(IPhysicsEntity* pEnt, const Vector& vecUp) {};
	virtual void			SetLinearFactor(IPhysicsEntity* pEnt, const Vector& vecFactor) {};
	virtual void			SetAngularFactor(IPhysicsEntity* pEnt, const Vector& vecFactor) {};

	virtual void            CharacterMovement(IPhysicsEntity* pEnt, class btCollisionWorld* pCollisionWorld, float flDelta) {};

	virtual void            TraceLine(CTraceResult& tr, const Vector& v1, const Vector& v2, collision_group_t eCollisions = CG_ALL, IPhysicsEntity* pIgnore=nullptr) {};
	virtual void            TraceEntity(CTraceResult& tr, IPhysicsEntity* pEntity, const Vector& v1, const Vector& v2, collision_group_t eCollisions = CG_ALL) {};
	virtual void            CheckSphere(CTraceResult& tr, float flRadius, const Vector& vecCenter, IPhysicsEntity* pIgnore=nullptr) {};

	virtual void			CharacterJump(IPhysicsEntity* pEnt) {};
};

typedef enum
{
	PHYSWORLD_GAME,
	PHYSWORLD_EDITOR,
} physics_world_t;

CPhysicsModel* GamePhysics();
CPhysicsModel* EditorPhysics();
CPhysicsModel* Physics(physics_world_t ePhysWorld = PHYSWORLD_GAME);

#endif
