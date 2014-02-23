#ifndef DT_MENUMARCHER_H
#define DT_MENUMARCHER_H

#include <game/entities/baseentity.h>
#include <renderer/particles.h>

class CMenuMarcher : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CMenuMarcher, CBaseEntity);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual void				OnDeleted();

	virtual const TFloat		GetBoundingRadius() const { return 4; };

	virtual void				Think();

	virtual bool				ShouldRender() const { return true; };
	virtual Vector				GetRenderOrigin() const;
	virtual void				ModifyContext(class CRenderingContext* pContext) const;
	virtual void				OnRender(class CGameRenderingContext* pContext) const;
	virtual void				RenderTurret() const;

	void						Speak();

	float						FindHoverHeight(Vector vecPosition) const;

protected:
	double						m_flNextSpeech;

	size_t						m_iTurretModel;

	CParticleSystemInstanceHandle m_hHoverParticles;

	float						m_flBobOffset;
};

#endif