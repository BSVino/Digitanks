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

	if (pData->GetKey() == "GameType")
	{
		tstring sValue = pData->GetValueTString();
		if (sValue == "artillery")
			m_eGameType = GAMETYPE_ARTILLERY;
		else if (sValue == "strategy")
			m_eGameType = GAMETYPE_STANDARD;
		else if (sValue == "campaign")
			m_eGameType = GAMETYPE_CAMPAIGN;
	}
	else if (pData->GetKey() == "Objective")
	{
		m_sObjective = pData->GetValueTString();
	}
	else if (pData->GetKey() == "TerrainHeight")
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
	else if (pData->GetKey() == "TerrainData")
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
	else if (pData->GetKey() == "MaxHeight")
		m_flMaxHeight = pData->GetValueFloat();
	else if (pData->GetKey() == "Prop")
		ReadProp(pData);
	else if (pData->GetKey() == "Entity")
		ReadUnit(pData);
	else if (pData->GetKey() == "Author")
		m_sAuthor = pData->GetValueTString();
	else if (pData->GetKey() == "Description")
		m_sDescription = pData->GetValueTString();
	else if (pData->GetKey() == "GameRules")
		ReadGameRules(pData);
	else if (pData->GetKey() == "Lesson")
		DigitanksWindow()->GetInstructor()->ReadLesson(pData);
}

void CDigitanksLevel::ReadProp(const CData* pData)
{
	m_aProps.push_back();
	CLevelProp* pProp = &m_aProps[m_aProps.size()-1];

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChildData = pData->GetChild(i);

		if (pChildData->GetKey() == "Model")
			pProp->m_sModel = pChildData->GetValueTString();
		else if (pChildData->GetKey() == "Position")
			pProp->m_vecPosition = pChildData->GetValueVector2D();
		else if (pChildData->GetKey() == "Angle")
			pProp->m_angOrientation = EAngle(0, pChildData->GetValueFloat(), 0);
	}
}

void CDigitanksLevel::ReadUnit(const CData* pData)
{
	CLevelUnit* pUnit = &m_aUnits.push_back();

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChildData = pData->GetChild(i);

		if (pChildData->GetKey() == "Name")
			pUnit->m_sName = pChildData->GetValueString();
		else if (pChildData->GetKey() == "Class")
			pUnit->m_sClassName = pChildData->GetValueString();
		else if (pChildData->GetKey() == "Team")
			pUnit->m_sTeamName = pChildData->GetValueString();
		else if (pChildData->GetKey() == "Position")
			pUnit->m_vecPosition = pChildData->GetValueVector2D();
		else if (pChildData->GetKey() == "Angle")
			pUnit->m_angOrientation = EAngle(0, pChildData->GetValueFloat(), 0);
		else if (pChildData->GetKey() == "Fortified")
			pUnit->m_bFortified = pChildData->GetValueBool();
		else if (pChildData->GetKey() == "Imprisoned")
			pUnit->m_bImprisoned = pChildData->GetValueBool();
		else if (pChildData->GetKey() == "Active")
			pUnit->m_bActive = pChildData->GetValueBool();
		else if (pChildData->GetKey() == "Objective")
			pUnit->m_bObjective = pChildData->GetValueBool();
		else if (pChildData->GetKey() == "Output")
			ReadUnitOutput(pChildData, pUnit);
		else if (pChildData->GetKey() == "Type")
			pUnit->m_sType = pChildData->GetValueString();
		else if (pChildData->GetKey() == "File")
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

		if (pChildData->GetKey() == "Target")
			pOutput->m_sTarget = pChildData->GetValueString();
		else if (pChildData->GetKey() == "Input")
			pOutput->m_sInput = pChildData->GetValueString();
		else if (pChildData->GetKey() == "Args")
			pOutput->m_sArgs = pChildData->GetValueString();
		else if (pChildData->GetKey() == "Kill")
			pOutput->m_bKill = pChildData->GetValueBool();
	}
}

void CDigitanksLevel::ReadGameRules(const CData* pData)
{
	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChildData = pData->GetChild(i);

		if (pChildData->GetKey() == "MacroBuffers")
			m_bBuffers = pChildData->GetValueBool();
		else if (pChildData->GetKey() == "PSUs")
			m_bPSUs = pChildData->GetValueBool();
		else if (pChildData->GetKey() == "TankFactories")
			m_bTankLoaders = pChildData->GetValueBool();
		else if (pChildData->GetKey() == "ArtilleryFactories")
			m_bArtilleryLoaders = pChildData->GetValueBool();
		else if (pChildData->GetKey() == "ResistorLasers")
			m_bInfantryLasers = pChildData->GetValueBool();
		else if (pChildData->GetKey() == "ResistorTreeCutters")
			m_bInfantryTreeCutters = pChildData->GetValueBool();
		else if (pChildData->GetKey() == "ResistorFortify")
			m_bInfantryFortify = pChildData->GetValueBool();
		else if (pChildData->GetKey() == "EnemyResistorLasers")
			m_bEnemyInfantryLasers = pChildData->GetValueBool();
		else if (pChildData->GetKey() == "StartingLesson")
			m_sStartingLesson = pChildData->GetValueString();
		else if (pChildData->GetKey() == "CPUBonusFleetPoints")
			m_iBonusCPUFleetPoints = pChildData->GetValueInt();
	}
}
