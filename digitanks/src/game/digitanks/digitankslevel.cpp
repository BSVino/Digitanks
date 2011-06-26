#include "digitankslevel.h"

#include <datamanager/data.h>
#include <renderer/renderer.h>

#include <ui/digitankswindow.h>
#include <ui/instructor.h>

CDigitanksLevel::CDigitanksLevel()
{
	m_eGameType = GAMETYPE_EMPTY;
	m_flMaxHeight = 60;
	m_iTerrainHeight = 0;
	m_iTerrainData = 0;

	m_bBuffers = true;
	m_bPSUs = true;
	m_bTankLoaders = true;
	m_bArtilleryLoaders = true;

	m_bInfantryLasers = true;
	m_bInfantryTreeCutters = true;
	m_bInfantryFortify = true;
	m_bEnemyInfantryLasers = true;

	m_iBonusCPUFleetPoints = 0;
}

CDigitanksLevel::~CDigitanksLevel()
{
	CRenderer::UnloadTextureData(m_iTerrainHeight);
	CRenderer::UnloadTextureData(m_iTerrainData);
}

void CDigitanksLevel::OnReadData(const CData* pData)
{
	BaseClass::OnReadData(pData);

	if (pData->GetKey() == _T("GameType"))
	{
		tstring sValue = pData->GetValueTString();
		if (sValue == _T("artillery"))
			m_eGameType = GAMETYPE_ARTILLERY;
		else if (sValue == _T("strategy"))
			m_eGameType = GAMETYPE_STANDARD;
		else if (sValue == _T("campaign"))
			m_eGameType = GAMETYPE_CAMPAIGN;
	}
	else if (pData->GetKey() == _T("Objective"))
	{
		m_sObjective = pData->GetValueTString();
	}
	else if (pData->GetKey() == _T("TerrainHeight"))
	{
		m_sTerrainHeight = pData->GetValueTString();
		m_iTerrainHeight = CRenderer::LoadTextureData(m_sTerrainHeight);

		if (CRenderer::GetTextureHeight(m_iTerrainHeight) != 256)
		{
			CRenderer::UnloadTextureData(m_iTerrainHeight);
			m_iTerrainHeight = 0;
		}

		if (CRenderer::GetTextureWidth(m_iTerrainHeight) != 256)
		{
			CRenderer::UnloadTextureData(m_iTerrainHeight);
			m_iTerrainHeight = 0;
		}
	}
	else if (pData->GetKey() == _T("TerrainData"))
	{
		m_sTerrainData = pData->GetValueTString();
		m_iTerrainData = CRenderer::LoadTextureData(m_sTerrainData);

		if (CRenderer::GetTextureHeight(m_iTerrainData) != 256)
		{
			CRenderer::UnloadTextureData(m_iTerrainData);
			m_iTerrainData = 0;
		}

		if (CRenderer::GetTextureWidth(m_iTerrainData) != 256)
		{
			CRenderer::UnloadTextureData(m_iTerrainData);
			m_iTerrainData = 0;
		}
	}
	else if (pData->GetKey() == _T("MaxHeight"))
		m_flMaxHeight = pData->GetValueFloat();
	else if (pData->GetKey() == _T("Prop"))
		ReadProp(pData);
	else if (pData->GetKey() == _T("Entity"))
		ReadUnit(pData);
	else if (pData->GetKey() == _T("Author"))
		m_sAuthor = pData->GetValueTString();
	else if (pData->GetKey() == _T("Description"))
		m_sDescription = pData->GetValueTString();
	else if (pData->GetKey() == _T("GameRules"))
		ReadGameRules(pData);
	else if (pData->GetKey() == _T("Lesson"))
		DigitanksWindow()->GetInstructor()->ReadLesson(pData);
}

void CDigitanksLevel::ReadProp(const CData* pData)
{
	m_aProps.push_back();
	CLevelProp* pProp = &m_aProps[m_aProps.size()-1];

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChildData = pData->GetChild(i);

		if (pChildData->GetKey() == _T("Model"))
			pProp->m_sModel = pChildData->GetValueTString();
		else if (pChildData->GetKey() == _T("Position"))
			pProp->m_vecPosition = pChildData->GetValueVector2D();
		else if (pChildData->GetKey() == _T("Angle"))
			pProp->m_angOrientation = EAngle(0, pChildData->GetValueFloat(), 0);
	}
}

void CDigitanksLevel::ReadUnit(const CData* pData)
{
	CLevelUnit* pUnit = &m_aUnits.push_back();

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChildData = pData->GetChild(i);

		if (pChildData->GetKey() == _T("Name"))
			pUnit->m_sName = pChildData->GetValueString();
		else if (pChildData->GetKey() == _T("Class"))
			pUnit->m_sClassName = pChildData->GetValueString();
		else if (pChildData->GetKey() == _T("Team"))
			pUnit->m_sTeamName = pChildData->GetValueString();
		else if (pChildData->GetKey() == _T("Position"))
			pUnit->m_vecPosition = pChildData->GetValueVector2D();
		else if (pChildData->GetKey() == _T("Angle"))
			pUnit->m_angOrientation = EAngle(0, pChildData->GetValueFloat(), 0);
		else if (pChildData->GetKey() == _T("Fortified"))
			pUnit->m_bFortified = pChildData->GetValueBool();
		else if (pChildData->GetKey() == _T("Imprisoned"))
			pUnit->m_bImprisoned = pChildData->GetValueBool();
		else if (pChildData->GetKey() == _T("Active"))
			pUnit->m_bActive = pChildData->GetValueBool();
		else if (pChildData->GetKey() == _T("Objective"))
			pUnit->m_bObjective = pChildData->GetValueBool();
		else if (pChildData->GetKey() == _T("Output"))
			ReadUnitOutput(pChildData, pUnit);
		else if (pChildData->GetKey() == _T("Type"))
			pUnit->m_sType = pChildData->GetValueString();
		else if (pChildData->GetKey() == _T("File"))
			pUnit->m_sFile = pChildData->GetValueTString();
	}
}

void CDigitanksLevel::ReadUnitOutput(const CData* pData, CLevelUnit* pUnit)
{
	CLevelUnitOutput* pOutput = &pUnit->m_aOutputs.push_back();
	pOutput->m_sOutput = pData->GetValueString();

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChildData = pData->GetChild(i);

		if (pChildData->GetKey() == _T("Target"))
			pOutput->m_sTarget = pChildData->GetValueString();
		else if (pChildData->GetKey() == _T("Input"))
			pOutput->m_sInput = pChildData->GetValueString();
		else if (pChildData->GetKey() == _T("Args"))
			pOutput->m_sArgs = pChildData->GetValueString();
		else if (pChildData->GetKey() == _T("Kill"))
			pOutput->m_bKill = pChildData->GetValueBool();
	}
}

void CDigitanksLevel::ReadGameRules(const CData* pData)
{
	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChildData = pData->GetChild(i);

		if (pChildData->GetKey() == _T("MacroBuffers"))
			m_bBuffers = pChildData->GetValueBool();
		else if (pChildData->GetKey() == _T("PSUs"))
			m_bPSUs = pChildData->GetValueBool();
		else if (pChildData->GetKey() == _T("TankFactories"))
			m_bTankLoaders = pChildData->GetValueBool();
		else if (pChildData->GetKey() == _T("ArtilleryFactories"))
			m_bArtilleryLoaders = pChildData->GetValueBool();
		else if (pChildData->GetKey() == _T("ResistorLasers"))
			m_bInfantryLasers = pChildData->GetValueBool();
		else if (pChildData->GetKey() == _T("ResistorTreeCutters"))
			m_bInfantryTreeCutters = pChildData->GetValueBool();
		else if (pChildData->GetKey() == _T("ResistorFortify"))
			m_bInfantryFortify = pChildData->GetValueBool();
		else if (pChildData->GetKey() == _T("EnemyResistorLasers"))
			m_bEnemyInfantryLasers = pChildData->GetValueBool();
		else if (pChildData->GetKey() == _T("StartingLesson"))
			m_sStartingLesson = pChildData->GetValueString();
		else if (pChildData->GetKey() == _T("CPUBonusFleetPoints"))
			m_iBonusCPUFleetPoints = pChildData->GetValueInt();
	}
}
