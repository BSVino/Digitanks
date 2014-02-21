#pragma once

#include <game/entities/baseentity.h>

class CProp : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CProp, CBaseEntity);

public:
	void OnRender(class CGameRenderingContext* pContext) const;

public:
	float    m_flThickness;
};
