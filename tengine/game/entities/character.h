#ifndef TINKER_CHARACTER_H
#define TINKER_CHARACTER_H

#include <tengine/game/entities/baseentity.h>

class CPlayer;
class CBaseWeapon;

typedef enum
{
	MOVE_FORWARD,
	MOVE_BACKWARD,
	MOVE_LEFT,
	MOVE_RIGHT,
} movetype_t;

class CCharacter : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CCharacter, CBaseEntity);

public:
									CCharacter();

public:
	virtual void					Spawn();
	virtual void					Think();
	virtual void                    Simulate();

	void							Move(movetype_t);
	void							StopMove(movetype_t);
	virtual const TVector			GetGoalVelocity();
	virtual void                    SetGoalVelocityForward(float flForward);
	virtual void                    SetGoalVelocityLeft(float flLeft);
	virtual void					MoveThink();
	virtual void					MoveThink_NoClip();
	virtual void					Jump();
	virtual const TMatrix           GetMovementVelocityTransform() const;

	virtual void                    CharacterMovement(class btCollisionWorld*, float flDelta);

	virtual void					SetNoClip(bool bOn);
	virtual bool					GetNoClip() const { return m_bNoClip; }

	virtual bool					CanAttack() const;
	virtual void					MeleeAttack();
	virtual bool					IsAttacking() const;

	virtual void					MoveToPlayerStart();

	virtual void					PostRender() const;
	virtual void					ShowPlayerVectors() const;

	void                            Weapon_Add(CBaseWeapon* pWeapon);
	void                            Weapon_Remove(CBaseWeapon* pWeapon);
	virtual void                    OnWeaponAdded(CBaseWeapon* pWeapon) {}
	virtual void                    OnWeaponRemoved(CBaseWeapon* pWeapon, bool bWasEquipped) {}
	void                            Weapon_Equip(CBaseWeapon* pWeapon);
	const tvector<CEntityHandle<CBaseWeapon>>& GetWeapons() const { return m_ahWeapons; }
	size_t                          GetEquippedWeaponIndex() const { return m_iEquippedWeapon; }
	CBaseWeapon*                    GetEquippedWeapon() const;

	void                            OnDeleted(const CBaseEntity* pEntity);

	void							SetControllingPlayer(CPlayer* pCharacter);
	CPlayer*						GetControllingPlayer() const;

	virtual TFloat					EyeHeight() const { return 180.0f; }
	virtual TFloat					BaseCharacterSpeed() { return 80.0f; }
	virtual TFloat					CharacterAcceleration() { return 4.0f; }
	virtual TFloat					JumpStrength() { return 150.0f; }
	virtual TFloat					CharacterSpeed();
	virtual bool                    IsFlying() const { return false; }   // Is the character under controlled flight without the influence of gravity?

	virtual float                   MeleeAttackTime() const { return 0.3f; }
	virtual float                   MeleeAttackDamage() const { return 50; }
	virtual float                   MeleeAttackSphereRadius() const { return 40.0f; }
	virtual const TVector           MeleeAttackSphereCenter() const;

	virtual bool					ShouldCollide() const { return false; }

	virtual const EAngle			GetThirdPersonCameraAngles() const { return GetViewAngles(); }

	CBaseEntity*					GetGroundEntity() const { return m_hGround; }
	virtual void                    SetGroundEntity(CBaseEntity* pEntity);
	virtual void                    SetGroundEntityExtra(size_t iExtra);

	TFloat							GetMaxStepHeight() const { return m_flMaxStepSize; }

	double                          GetLastSpawn() const { return m_flLastSpawn; }

	virtual bool					UsePhysicsModelForController() const { return false; }
	collision_group_t               GetCollisionGroup() const { return CG_CHARACTER; }
	const Matrix4x4                 GetPhysicsTransform() const;
	void                            SetPhysicsTransform(const Matrix4x4& m);
	void                            OnSetLocalTransform(TMatrix& m);

	virtual CBaseEntity*            Use();
	virtual CBaseEntity*            FindUseItem() const;

protected:
	CNetworkedHandle<CPlayer>		m_hControllingPlayer;

	CNetworkedHandle<CBaseEntity>	m_hGround;

	bool							m_bNoClip;

	bool							m_bTransformMoveByView;
	TVector							m_vecMoveVelocity;
	double							m_flMoveSimulationTime;

	double							m_flLastAttack;
	double                          m_flLastSpawn;

	TFloat							m_flMaxStepSize;

	size_t                              m_iEquippedWeapon;
	tvector<CEntityHandle<CBaseWeapon>> m_ahWeapons;
};

#endif
