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

class CLevelUnitOutput
{
public:
	CLevelUnitOutput()
	{
		m_bKill = false;
	};

	eastl::string			m_sOutput;
	eastl::string			m_sTarget;
	eastl::string			m_sInput;
	eastl::string			m_sArgs;
	bool					m_bKill;
};

class CLevelUnit
{
public:
	CLevelUnit()
	{
		m_bFortified = false;
		m_bImprisoned = false;
		m_bActive = true;
		m_bObjective = false;
	};

	eastl::string			m_sName;
	eastl::string			m_sClassName;
	eastl::string			m_sTeamName;
	Vector2D				m_vecPosition;
	EAngle					m_angOrientation;
	bool					m_bFortified;
	bool					m_bImprisoned;
	bool					m_bActive;
	bool					m_bObjective;
	eastl::string			m_sType;

	eastl::vector<CLevelUnitOutput>	m_aOutputs;
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
	void					ReadUnit(const class CData* pData);
	void					ReadUnitOutput(const class CData* pData, CLevelUnit* pUnit);
	void					ReadGameRules(const class CData* pData);

	gametype_t				GetGameType() { return m_eGameType; }
	eastl::string			GetTerrainHeight() { return m_sTerrainHeight; }
	size_t					GetTerrainHeightImage() { return m_iTerrainHeight; }
	eastl::string			GetTerrainData() { return m_sTerrainData; }
	size_t					GetTerrainDataImage() { return m_iTerrainData; }
	float					GetMaxHeight() { return m_flMaxHeight; }

	size_t					GetNumProps() { return m_aProps.size(); }
	CLevelProp*				GetProp(size_t i) { return &m_aProps[i]; }

	size_t					GetNumUnits() { return m_aUnits.size(); }
	CLevelUnit*				GetUnit(size_t i) { return &m_aUnits[i]; }

	eastl::string			GetAuthor() { return m_sAuthor; }
	eastl::string			GetDescription() { return m_sDescription; }

	bool					AllowBuffers() { return m_bBuffers; }
	bool					AllowPSUs() { return m_bPSUs; }
	bool					AllowTankLoaders() { return m_bTankLoaders; }
	bool					AllowArtilleryLoaders() { return m_bArtilleryLoaders; }

	bool					AllowInfantryLasers() { return m_bInfantryLasers; }
	bool					AllowInfantryTreeCutters() { return m_bInfantryTreeCutters; }
	bool					AllowInfantryFortify() { return m_bInfantryFortify; }

	bool					AllowEnemyInfantryLasers() { return m_bEnemyInfantryLasers; }

	int						GetBonusCPUFleetPoints() { return m_iBonusCPUFleetPoints; }

	eastl::string			GetStartingLesson() { return m_sStartingLesson; }

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

	eastl::vector<CLevelUnit>	m_aUnits;

	bool					m_bBuffers;
	bool					m_bPSUs;
	bool					m_bTankLoaders;
	bool					m_bArtilleryLoaders;

	bool					m_bInfantryLasers;
	bool					m_bInfantryTreeCutters;
	bool					m_bInfantryFortify;

	bool					m_bEnemyInfantryLasers;

	int						m_iBonusCPUFleetPoints;

	eastl::string			m_sStartingLesson;
};

#endif
