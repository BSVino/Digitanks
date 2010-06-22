#ifndef POWERUP_H
#define POWERUP_H

#include "baseentity.h"

class CPowerup : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CPowerup, CBaseEntity);

public:
					CPowerup();

public:
	void			Precache();

	virtual EAngle	GetRenderAngles() const;
	virtual void	PreRender();
	virtual void	ModifyContext(class CRenderingContext* pContext);
};

#endif