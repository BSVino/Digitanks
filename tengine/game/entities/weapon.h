#pragma once

#include <tengine/game/entities/baseentity.h>

class CCharacter;

class CBaseWeapon : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CBaseWeapon, CBaseEntity);

public:
	virtual void                    MeleeAttack() {};

	virtual void                    DrawViewModel(class CGameRenderingContext* pContext) {};

	virtual void                    OwnerMouseInput(int iButton, int iState);

	virtual float                   MeleeAttackTime() const { return 0.3f; }
	virtual float                   MeleeAttackDamage() const { return 50; }
	virtual float                   MeleeAttackSphereRadius() const { return 40.0f; }
	virtual const TVector           MeleeAttackSphereCenter() const;

	void                            SetOwner(CCharacter* pOwner);
	CCharacter*                     GetOwner() const;

private:
	CNetworkedHandle<CCharacter>    m_hOwner;
};
