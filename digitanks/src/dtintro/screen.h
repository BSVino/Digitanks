#ifndef DT_INTRO_SCREEN_H
#define DT_INTRO_SCREEN_H

#include <game/entities/baseentity.h>

class CScreen : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CScreen, CBaseEntity);

public:
	void			Precache();
	virtual void	Spawn();

	virtual bool	ShouldRender() const { return true; };
	virtual void	ModifyContext(class CRenderingContext* pContext) const;
	virtual void	OnRender(class CGameRenderingContext* pContext) const;

	virtual void	SetScreenshot(size_t iScreenshot);
};

#endif
