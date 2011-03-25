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

	m_bInfantryLasers = true;
	m_bInfantryTreeCutters = true;
	m_bInfantryFortify = true;
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
		eastl::string sValue = pData->GetValueString();
		if (sValue == "artillery")
			m_eGameType = GAMETYPE_ARTILLERY;
		else if (sValue == "strategy")
			m_eGameType = GAMETYPE_STANDARD;
	}
	else if (pData->GetKey() == "TerrainHeight")
	{
		m_sTerrainHeight = pData->GetValueString();
		m_iTerrainHeight = CRenderer::LoadTextureData(convertstring<char, char16_t>(m_sTerrainHeight));

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
		m_sTerrainData = pData->GetValueString();
		m_iTerrainData = CRenderer::LoadTextureData(convertstring<char, char16_t>(m_sTerrainData));

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
	else if (pData->GetKey() == "Unit")
		ReadUnit(pData);
	else if (pData->GetKey() == "Author")
		m_sAuthor = pData->GetValueString();
	else if (pData->GetKey() == "Description")
		m_sDescription = pData->GetValueString();
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
			pProp->m_sModel = pChildData->GetValueString();
		else if (pChildData->GetKey() == "Position")
			pProp->m_vecPosition = pChildData->GetValueVector2D();
		else if (pChildData->GetKey() == "Angle")
			pProp->m_angOrientation = EAngle(0, pChildData->GetValueFloat(), 0);
	}
}

void CDigitanksLevel::ReadUnit(const CData* pData)
{
	CLevelUnit* pProp = &m_aUnits.push_back();

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChildData = pData->GetChild(i);

		if (pChildData->GetKey() == "Class")
			pProp->m_sClassName = pChildData->GetValueString();
		else if (pChildData->GetKey() == "Team")
			pProp->m_sTeamName = pChildData->GetValueString();
		else if (pChildData->GetKey() == "Position")
			pProp->m_vecPosition = pChildData->GetValueVector2D();
		else if (pChildData->GetKey() == "Angle")
			pProp->m_angOrientation = EAngle(0, pChildData->GetValueFloat(), 0);
		else if (pChildData->GetKey() == "Fortified")
			pProp->m_bFortified = pChildData->GetValueBool();
		else if (pChildData->GetKey() == "Imprisoned")
			pProp->m_bImprisoned = pChildData->GetValueBool();
	}
}

void CDigitanksLevel::ReadGameRules(const CData* pData)
{
	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChildData = pData->GetChild(i);

		if (pChildData->GetKey() == "InfantryLasers")
			m_bInfantryLasers = pChildData->GetValueBool();
		else if (pChildData->GetKey() == "InfantryTreeCutters")
			m_bInfantryTreeCutters = pChildData->GetValueBool();
		else if (pChildData->GetKey() == "InfantryFortify")
			m_bInfantryFortify = pChildData->GetValueBool();
		else if (pChildData->GetKey() == "StartingLesson")
			m_sStartingLesson = pChildData->GetValueString();
	}
}
