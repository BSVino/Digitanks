#ifndef DT_INTRO_TANK_H
#define DT_INTRO_TANK_H

#include <game/baseentity.h>

class CIntroTank : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CIntroTank, CBaseEntity);

public:
					CIntroTank();

public:
	virtual void	Precache();

	virtual bool	ShouldRender() const { return true; };
	virtual EAngle	GetRenderAngles() const;
	virtual void	ModifyContext(class CRenderingContext* pContext, bool bTransparent);
	virtual void	OnRender(class CRenderingContext* pContext, bool bTransparent);

	void			FaceTurret(float flYaw) { m_flGoalTurretYaw = flYaw; };

	void			FireBomb(Vector vecLandingSpot, CBaseEntity* pTarget = NULL);

	void			RockTheBoat(float flIntensity, Vector vecDirection);
	bool			IsRocking() const;

protected:
	float			m_flCurrentTurretYaw;
	float			m_flGoalTurretYaw;
	size_t			m_iTurretModel;

	float			m_flStartedRock;
	float			m_flRockIntensity;
	Vector			m_vecRockDirection;
};

#endif
