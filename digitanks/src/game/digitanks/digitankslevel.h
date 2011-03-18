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
							CDigitanksLevel();
	virtual					~CDigitanksLevel();

public:
	virtual void			OnReadData(const class CData* pData);
	void					ReadProp(const class CData* pData);

	gametype_t				GetGameType() { return m_eGameType; }
	eastl::string			GetTerrainHeight() { return m_sTerrainHeight; }
	size_t					GetTerrainHeightImage() { return m_iTerrainHeight; }
	eastl::string			GetTerrainData() { return m_sTerrainData; }
	size_t					GetTerrainDataImage() { return m_iTerrainData; }
	float					GetMaxHeight() { return m_flMaxHeight; }

	size_t					GetNumProps() { return m_aProps.size(); }
	CLevelProp*				GetProp(size_t i) { return &m_aProps[i]; }

	eastl::string			GetAuthor() { return m_sAuthor; }
	eastl::string			GetDescription() { return m_sDescription; }

protected:
	gametype_t				m_eGameType;
	eastl::string			m_sTerrainHeight;
	size_t					m_iTerrainHeight;
	eastl::string			m_sTerrainData;
	size_t					m_iTerrainData;
	float					m_flMaxHeight;
	eastl::vector<CLevelProp>	m_aProps;

	eastl::string			m_sAuthor;
	eastl::string			m_sDescription;
};

#endif
