#ifndef TINKER_CHARACTER_CONTROLLER_H
#define TINKER_CHARACTER_CONTROLLER_H

#include "LinearMath/btVector3.h"

#include "BulletDynamics/Character/btCharacterControllerInterface.h"

#include "BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h"

#include <game/entityhandle.h>

#include "bullet_physics.h"

class CBaseEntity;
class CCharacter;
class btCollisionShape;
class btRigidBody;
class btCollisionWorld;
class btCollisionDispatcher;
class btPairCachingGhostObject;
class btConvexShape;

// This class was originally forked from the Bullet library's btKinematicCharacterController class.

// btKinematicCharacterController is an object that supports a sliding motion in a world.
// It uses a ghost object and convex sweep test to test for upcoming collisions. This is combined with discrete collision detection to recover from penetrations.
// Interaction between btKinematicCharacterController and dynamic rigid bodies needs to be explicity implemented by the user.
class CCharacterController : public btCharacterControllerInterface
{
public:
	CCharacterController (CCharacter* pEntity, btPairCachingGhostObject* ghostObject,btConvexShape* convexShape,btScalar stepHeight);
	~CCharacterController ();

public:
	// btActionInterface
	virtual void	updateAction(btCollisionWorld* collisionWorld, btScalar deltaTime);
	void			debugDraw(btIDebugDraw* debugDrawer) {};

	// btCharacterControllerInterface
	virtual void	preStep(btCollisionWorld* collisionWorld);
	virtual void	playerStep(btCollisionWorld* collisionWorld, btScalar dt);
	virtual void	setWalkDirection(const btVector3& walkDirection);
	virtual void	setVelocityForTimeInterval(const btVector3& velocity, btScalar timeInterval);
	virtual void	reset() {};
	virtual void	warp(const btVector3& origin);
	virtual bool	canJump() const;
	virtual void	jump();
	virtual bool	onGround() const;

	virtual void	CharacterMovement(btCollisionWorld* collisionWorld, float flDelta);
	virtual void    PreStep(btCollisionWorld* collisionWorld);
	virtual void    PlayerWalk(btCollisionWorld* collisionWorld, btScalar dt);
	virtual void    PlayerFall(btCollisionWorld* collisionWorld, btScalar dt);
	virtual void    PlayerFly(btCollisionWorld* collisionWorld, btScalar dt);

	virtual bool    PlayerTrace(btCollisionWorld* pCollisionWorld, const btVector3& vecStart, const btVector3& vecEnd, CTraceResult& tr);

	virtual void    SetMoveVelocity(const btVector3& velocity);
	btVector3       GetMoveVelocity() const;

	void			SetMaxSpeed(btScalar flMaxSpeed);
	void			SetJumpSpeed(btScalar jumpSpeed);
	void			SetMaxJumpHeight(btScalar maxJumpHeight);

	void			SetGravity(const btVector3& gravity);
	btVector3		GetGravity() const;

	btVector3		GetVelocity() const;

	void			SetUpVector(const btVector3& v) { m_vecUpVector = v; };
	btVector3		GetUpVector() const { return m_vecUpVector; };

	void			SetLinearFactor(const btVector3& v) { m_vecLinearFactor = v; };
	btVector3		GetLinearFactor() const { return m_vecLinearFactor; };

	/// The max slope determines the maximum angle that the controller can walk up.
	/// The slope angle is measured in radians.
	void			SetMaxSlope(btScalar slopeRadians);
	btScalar		GetMaxSlope() const;

	void			SetColliding(bool bColliding) { m_bColliding = bColliding; }
	bool			IsColliding() { return m_bColliding; }

	btPairCachingGhostObject* getGhostObject();

	class CBaseEntity*	GetEntity() const;

protected:
	btVector3	ComputeReflectionDirection(const btVector3& direction, const btVector3& normal);
	btVector3	ParallelComponent(const btVector3& direction, const btVector3& normal);
	btVector3	PerpendicularComponent(const btVector3& direction, const btVector3& normal);

	bool        RecoverFromPenetration(btCollisionWorld* collisionWorld);
	void        StepForwardAndStrafe(btCollisionWorld* collisionWorld, const btVector3& walkMove);

	void        FindGround(btCollisionWorld* pCollisionWorld);

protected:
	CEntityHandle<CCharacter>	m_hEntity;

	btScalar		m_flHalfHeight;

	btPairCachingGhostObject* m_pGhostObject;
	btConvexShape*	m_pConvexShape;		//is also in m_ghostObject, but it needs to be convex, so we store it here to avoid upcast

	btScalar		m_flMaxSpeed;
	btScalar		m_flJumpSpeed;
	btScalar		m_flMaxSlopeRadians; // Slope angle that is set (used for returning the exact value)
	btScalar		m_flMaxSlopeCosine;  // Cosine equivalent of m_maxSlopeRadians (calculated once when set, for optimization)
	btVector3		m_vecGravity;

	btVector3		m_vecUpVector;

	btVector3		m_vecLinearFactor;

	btScalar		m_flStepHeight;

	btScalar		m_flAddedMargin;	//@todo: remove this and fix the code

	///this is the desired walk direction, set by the user
	btVector3       m_vecMoveVelocity;

	///keep track of the contact manifolds
	btManifoldArray	m_aManifolds;

	bool			m_bColliding;
};

#endif
