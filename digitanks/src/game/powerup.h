#ifndef POWERUP_H
#define POWERUP_H

#include "baseentity.h"

class CPowerup : public CBaseEntity
{
public:
					CPowerup();

public:
	void			Precache();

	virtual EAngle	GetRenderAngles() const;
	virtual void	PreRender();
	virtual void	ModifyContext(class CRenderingContext* pContext);
};

#endif