#ifndef DT_LEVEL_H
#define DT_LEVEL_H

#include <common.h>

#include <game/level.h>

#include "digitanksgame.h"

class CLevelProp
{
public:
	tstring					m_sModel;
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

	tstring			m_sOutput;
	tstring			m_sTarget;
	tstring			m_sInput;
	tstring			m_sArgs;
	bool			m_bKill;
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

	tstring			m_sName;
	tstring			m_sClassName;
	tstring			m_sTeamName;
	Vector2D		m_vecPosition;
	EAngle			m_angOrientation;
	bool			m_bFortified;
	bool			m_bImprisoned;
	bool			m_bActive;
	bool			m_bObjective;
	tstring			m_sType;
	tstring			m_sFile;

	tvector<CLevelUnitOutput>	m_aOutputs;
};

class CDigitanksLevel : public CLevel
{
	DECLARE_CLASS(CDigitanksLevel, CLevel);

public:
							CDigitanksLevel();
	virtual					~CDigitanksLevel();

public:
	virtual void			OnReadInfo(const class CData* pData);
	void					ReadProp(const class CData* pData);
	void					ReadUnit(const class CData* pData);
	void					ReadUnitOutput(const class CData* pData, CLevelUnit* pUnit);
	void					ReadGameRules(const class CData* pData);

	tstring					GetObjective() { return m_sObjective; }
	gametype_t				GetGameType() { return m_eGameType; }
	tstring					GetTerrainHeight() { return m_sTerrainHeight; }
	CTextureHandle          GetTerrainHeightImage() { return m_hTerrainHeight; }
	tstring					GetTerrainData() { return m_sTerrainData; }
	CTextureHandle          GetTerrainDataImage() { return m_hTerrainData; }
	float					GetMaxHeight() { return m_flMaxHeight; }

	size_t					GetNumProps() { return m_aProps.size(); }
	CLevelProp*				GetProp(size_t i) { return &m_aProps[i]; }

	size_t					GetNumUnits() { return m_aUnits.size(); }
	CLevelUnit*				GetUnit(size_t i) { return &m_aUnits[i]; }

	tstring					GetAuthor() { return m_sAuthor; }
	tstring					GetDescription() { return m_sDescription; }

	bool					AllowBuffers() { return m_bBuffers; }
	bool					AllowPSUs() { return m_bPSUs; }
	bool					AllowTankLoaders() { return m_bTankLoaders; }
	bool					AllowArtilleryLoaders() { return m_bArtilleryLoaders; }

	bool					AllowInfantryLasers() { return m_bInfantryLasers; }
	bool					AllowInfantryTreeCutters() { return m_bInfantryTreeCutters; }
	bool					AllowInfantryFortify() { return m_bInfantryFortify; }

	bool					AllowEnemyInfantryLasers() { return m_bEnemyInfantryLasers; }

	int						GetBonusCPUFleetPoints() { return m_iBonusCPUFleetPoints; }

	tstring					GetStartingLesson() { return m_sStartingLesson; }

protected:
	gametype_t				m_eGameType;
	tstring					m_sObjective;
	tstring					m_sTerrainHeight;
	CTextureHandle			m_hTerrainHeight;
	tstring					m_sTerrainData;
	CTextureHandle			m_hTerrainData;
	float					m_flMaxHeight;
	tvector<CLevelProp>		m_aProps;

	tstring					m_sAuthor;
	tstring					m_sDescription;

	tvector<CLevelUnit>		m_aUnits;

	bool					m_bBuffers;
	bool					m_bPSUs;
	bool					m_bTankLoaders;
	bool					m_bArtilleryLoaders;

	bool					m_bInfantryLasers;
	bool					m_bInfantryTreeCutters;
	bool					m_bInfantryFortify;

	bool					m_bEnemyInfantryLasers;

	int						m_iBonusCPUFleetPoints;

	tstring					m_sStartingLesson;
};

#endif
