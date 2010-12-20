#ifndef DT_LEVEL_H
#define DT_LEVEL_H

#include "../level.h"

#include <common.h>

#include "digitanksgame.h"

class CLevelProp
{
public:
	eastl::string			m_sModel;
	Vector2D				m_vecPosition;
	EAngle					m_angOrientation;
};

class CDigitanksLevel : public CLevel
{
	DECLARE_CLASS(CDigitanksLevel, CLevel);

public:
	virtual void			OnReadData(const class CData* pData);
	void					ReadProp(const class CData* pData);

	gametype_t				GetGameType() { return m_eGameType; }
	eastl::string			GetTerrainHeight() { return m_sTerrainHeight; }
	eastl::string			GetTerrainData() { return m_sTerrainData; }
	float					GetMaxHeight() { return m_flMaxHeight; }

	size_t					GetNumProps() { return m_aProps.size(); }
	CLevelProp*				GetProp(size_t i) { return &m_aProps[i]; }

protected:
	gametype_t				m_eGameType;
	eastl::string			m_sTerrainHeight;
	eastl::string			m_sTerrainData;
	float					m_flMaxHeight;
	eastl::vector<CLevelProp>	m_aProps;
};

#endif
