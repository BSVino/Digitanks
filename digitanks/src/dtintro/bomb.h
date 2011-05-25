#ifndef DT_INTRO_BOMB_H
#define DT_INTRO_BOMB_H

#include <baseentity.h>

#include <renderer/particles.h>

class CBomb : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CBomb, CBaseEntity);

public:
	void			Precache();
	virtual void	Spawn();

	virtual void	Think();

	virtual bool	ShouldRender() const { return true; };
	virtual void	OnRender(class CRenderingContext* pContext, bool bTransparent);

	void			SetExplodeTime(float flExplodeTime) { m_flExplodeTime = flExplodeTime; };

protected:
	float			m_flExplodeTime;
	CParticleSystemInstanceHandle m_hTrailParticles;
};

#endif
