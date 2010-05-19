#ifndef POWERUP_H
#define POWERUP_H

#include "baseentity.h"

class CPowerup : public CBaseEntity
{
public:
					CPowerup();

public:
	virtual void	OnRender();
};

#endif