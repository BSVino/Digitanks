#include "cpu.h"

#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <mtrand.h>
#include <renderer/renderer.h>

#include <game/team.h>
#include <ui/digitankswindow.h>
#include <ui/instructor.h>
#include <ui/hud.h>
#include <models/models.h>

#include "buffer.h"
#include "collector.h"
#include "loader.h"

REGISTER_ENTITY(CCPU);

void CCPU::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/structures/cpu.obj");
	m_iFanModel = CModelLibrary::Get()->FindModel(L"models/structures/cpu-fan.obj");

	m_flFanRotationSpeed = 0;
	m_flFanRotation = RandomFloat(0, 360);
}

void CCPU::Precache()
{
	BaseClass::Precache();

	PrecacheModel(L"models/structures/cpu.obj", false);
	PrecacheModel(L"models/structures/cpu-fan.obj", false);
}

void CCPU::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = CDigitanksWindow::Get()->GetHUD();

	bool bDisableBuffer = CDigitanksWindow::Get()->GetInstructor()->IsFeatureDisabled(DISABLE_BUFFER);
	bool bDisablePSU = CDigitanksWindow::Get()->GetInstructor()->IsFeatureDisabled(DISABLE_PSU);
	bool bDisableLoaders = CDigitanksWindow::Get()->GetInstructor()->IsFeatureDisabled(DISABLE_LOADERS);

	if (m_hConstructing != NULL)
	{
		pHUD->SetButton1Listener(NULL);
		pHUD->SetButton2Listener(NULL);
		pHUD->SetButton3Listener(NULL);
		pHUD->SetButton4Listener(NULL);
		pHUD->SetButton5Listener(CHUD::CancelBuild);

		pHUD->SetButton1Texture(0);
		pHUD->SetButton2Texture(0);
		pHUD->SetButton3Texture(0);
		pHUD->SetButton4Texture(0);
		pHUD->SetButton5Texture(0);

		pHUD->SetButton1Help("");
		pHUD->SetButton2Help("");
		pHUD->SetButton3Help("");
		pHUD->SetButton4Help("");
		pHUD->SetButton5Help("Cancel\nBuild");

		pHUD->SetButton1Color(glgui::g_clrBox);
		pHUD->SetButton2Color(glgui::g_clrBox);
		pHUD->SetButton3Color(glgui::g_clrBox);
		pHUD->SetButton4Color(glgui::g_clrBox);
		pHUD->SetButton5Color(Color(100, 0, 0));
	}
	else if (!bDisableLoaders && eMenuMode == MENUMODE_LOADERS)
	{
		pHUD->SetButton1Listener(CHUD::BuildInfantryLoader);
		pHUD->SetButton2Listener(CHUD::BuildTankLoader);
		pHUD->SetButton3Listener(CHUD::BuildArtilleryLoader);
		pHUD->SetButton4Listener(NULL);
		pHUD->SetButton5Listener(CHUD::GoToMain);

		pHUD->SetButton1Texture(0);
		pHUD->SetButton2Texture(0);
		pHUD->SetButton3Texture(0);
		pHUD->SetButton4Texture(0);
		pHUD->SetButton5Texture(0);

		pHUD->SetButton1Help("Infantry\nLoader");
		pHUD->SetButton2Help("Main Tank\nLoader");
		pHUD->SetButton3Help("Artillery\nLoader");
		pHUD->SetButton4Help("");
		pHUD->SetButton5Help("Return");

		pHUD->SetButton1Color(Color(150, 150, 150));
		pHUD->SetButton2Color(Color(150, 150, 150));
		pHUD->SetButton3Color(Color(150, 150, 150));
		pHUD->SetButton4Color(glgui::g_clrBox);
		pHUD->SetButton5Color(Color(100, 0, 0));
	}
	else
	{
		if (bDisableBuffer)
			pHUD->SetButton1Listener(NULL);
		else
			pHUD->SetButton1Listener(CHUD::BuildBuffer);

		if (bDisablePSU)
			pHUD->SetButton2Listener(NULL);
		else
			pHUD->SetButton2Listener(CHUD::BuildPSU);

		if (bDisableLoaders)
			pHUD->SetButton3Listener(NULL);
		else
			pHUD->SetButton3Listener(CHUD::BuildLoader);

		pHUD->SetButton4Listener(NULL);
		pHUD->SetButton5Listener(NULL);

		if (bDisableBuffer)
			pHUD->SetButton1Help("");
		else
			pHUD->SetButton1Help("Build\nBuffer");

		if (bDisablePSU)
			pHUD->SetButton2Help("");
		else
			pHUD->SetButton2Help("Build\nPwr Supply");

		if (bDisableLoaders)
			pHUD->SetButton3Help("");
		else
			pHUD->SetButton3Help("Build\nLoader");

		pHUD->SetButton4Help("");
		pHUD->SetButton5Help("");

		pHUD->SetButton1Texture(0);
		pHUD->SetButton2Texture(0);
		pHUD->SetButton3Texture(0);
		pHUD->SetButton4Texture(0);
		pHUD->SetButton5Texture(0);

		if (bDisableBuffer)
			pHUD->SetButton1Color(glgui::g_clrBox);
		else
			pHUD->SetButton1Color(Color(150, 150, 150));

		if (bDisablePSU)
			pHUD->SetButton2Color(glgui::g_clrBox);
		else
			pHUD->SetButton2Color(Color(150, 150, 150));

		if (bDisableLoaders)
			pHUD->SetButton3Color(glgui::g_clrBox);
		else
			pHUD->SetButton3Color(Color(150, 150, 150));

		pHUD->SetButton4Color(glgui::g_clrBox);
		pHUD->SetButton5Color(glgui::g_clrBox);
	}
}

bool CCPU::AllowControlMode(controlmode_t eMode)
{
	if (eMode == MODE_BUILD)
		return true;

	return BaseClass::AllowControlMode(eMode);
}

bool CCPU::IsPreviewBuildValid() const
{
	CSupplier* pSupplier = FindClosestSupplier(GetPreviewBuild(), GetTeam());

	if (m_ePreviewStructure == STRUCTURE_PSU)
	{
		CResource* pResource = CResource::FindClosestResource(GetPreviewBuild(), RESOURCE_ELECTRONODE);
		float flDistance = (pResource->GetOrigin() - GetPreviewBuild()).Length();
		if (flDistance > 15)
			return false;

		if (pResource->HasCollector())
			return false;
	}

	// Don't allow construction too close to other structures.
	CStructure* pClosestStructure = CBaseEntity::FindClosest<CStructure>(GetPreviewBuild());
	if ((pClosestStructure->GetOrigin() - GetPreviewBuild()).Length() < pClosestStructure->GetBoundingRadius()+5)
		return false;

	if (!pSupplier)
		return false;

	return CSupplier::GetDataFlow(GetPreviewBuild(), GetTeam()) > 0;
}

void CCPU::SetPreviewBuild(Vector vecPreviewBuild)
{
	m_vecPreviewBuild = vecPreviewBuild;
}

void CCPU::ClearPreviewBuild()
{
	m_vecPreviewBuild = GetOrigin();
}

void CCPU::BeginConstruction()
{
	if (!IsPreviewBuildValid())
		return;

	if (m_hConstructing != NULL)
		CancelConstruction();

	if (m_ePreviewStructure == STRUCTURE_BUFFER)
		m_hConstructing = Game()->Create<CBuffer>("CBuffer");
	else if (m_ePreviewStructure == STRUCTURE_PSU)
		m_hConstructing = Game()->Create<CCollector>("CCollector");
	else if (m_ePreviewStructure == STRUCTURE_INFANTRYLOADER)
		m_hConstructing = Game()->Create<CLoader>("CLoader");
	else if (m_ePreviewStructure == STRUCTURE_TANKLOADER)
		m_hConstructing = Game()->Create<CLoader>("CLoader");
	else if (m_ePreviewStructure == STRUCTURE_ARTILLERYLOADER)
		m_hConstructing = Game()->Create<CLoader>("CLoader");

	GetTeam()->AddEntity(m_hConstructing);
	m_hConstructing->SetOrigin(GetPreviewBuild());
	m_hConstructing->BeginConstruction();
	m_hConstructing->SetSupplier(FindClosestSupplier(GetPreviewBuild(), GetTeam()));
	m_hConstructing->GetSupplier()->AddChild(m_hConstructing);

	CSupplier* pSupplier = dynamic_cast<CSupplier*>(m_hConstructing.GetPointer());
	if (pSupplier)
		pSupplier->GiveDataStrength((size_t)pSupplier->GetSupplier()->GetDataFlow(pSupplier->GetOrigin()));

	CCollector* pCollector = dynamic_cast<CCollector*>(m_hConstructing.GetPointer());
	if (pCollector)
	{
		pCollector->SetResource(CResource::FindClosestResource(GetPreviewBuild(), pCollector->GetResourceType()));
		pCollector->GetResource()->SetCollector(pCollector);
	}

	CLoader* pLoader = dynamic_cast<CLoader*>(m_hConstructing.GetPointer());
	if (pLoader)
	{
		if (m_ePreviewStructure == STRUCTURE_INFANTRYLOADER)
			pLoader->SetBuildUnit(BUILDUNIT_INFANTRY);
		else if (m_ePreviewStructure == STRUCTURE_TANKLOADER)
			pLoader->SetBuildUnit(BUILDUNIT_TANK);
		else
			pLoader->SetBuildUnit(BUILDUNIT_ARTILLERY);
	}

	size_t iTutorial = CDigitanksWindow::Get()->GetInstructor()->GetCurrentTutorial();

	if (m_ePreviewStructure == STRUCTURE_BUFFER && iTutorial == CInstructor::TUTORIAL_BUFFER)
	{
		CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_BUFFER);
		CDigitanksWindow::Get()->GetInstructor()->NextTutorial();

		// Make sure it's done next turn.
		m_hConstructing->AddProduction(m_hConstructing->GetProductionRemaining());
	}

	if (m_ePreviewStructure == STRUCTURE_PSU && iTutorial == CInstructor::TUTORIAL_PSU)
	{
		CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_PSU);
		CDigitanksWindow::Get()->GetInstructor()->NextTutorial();

		// Make sure it's done next turn.
		m_hConstructing->AddProduction(m_hConstructing->GetProductionRemaining());
	}

	if (iTutorial == CInstructor::TUTORIAL_LOADER && (m_ePreviewStructure == STRUCTURE_INFANTRYLOADER || m_ePreviewStructure == STRUCTURE_TANKLOADER || m_ePreviewStructure == STRUCTURE_ARTILLERYLOADER))
	{
		CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_LOADER);
		CDigitanksWindow::Get()->GetInstructor()->NextTutorial();

		// Make sure it's done next turn.
		m_hConstructing->AddProduction(m_hConstructing->GetProductionRemaining());
	}
}

void CCPU::CancelConstruction()
{
	if (m_hConstructing != NULL)
	{
		Game()->Delete(m_hConstructing);
		m_hConstructing = NULL;
	}
}

void CCPU::StartTurn()
{
	BaseClass::StartTurn();

	if (m_hConstructing != NULL && m_hConstructing->IsConstructing())
	{
		if (GetDigitanksTeam()->GetProductionPerLoader() > m_hConstructing->GetProductionRemaining())
		{
			std::stringstream s;
			s << "Construction finished on " << m_hConstructing->GetName();
			DigitanksGame()->AppendTurnInfo(s.str().c_str());

			CCollector* pCollector = dynamic_cast<CCollector*>(m_hConstructing.GetPointer());
			if (pCollector && pCollector->GetSupplier())
				GetDigitanksTeam()->AddProduction((size_t)(pCollector->GetResource()->GetProduction() * pCollector->GetSupplier()->GetChildEfficiency()));

			m_hConstructing->CompleteConstruction();
			m_hConstructing = NULL;
		}
		else
		{
			m_hConstructing->AddProduction((size_t)GetDigitanksTeam()->GetProductionPerLoader());

			std::stringstream s;
			s << "Constructing " << m_hConstructing->GetName() << " (" << m_hConstructing->GetTurnsToConstruct() << " turns left)";
			DigitanksGame()->AppendTurnInfo(s.str().c_str());
		}
	}
}

void CCPU::OnRender()
{
	BaseClass::OnRender();

	if (m_iFanModel == ~0)
		return;

	if (GetVisibility() == 0)
		return;

	CRenderingContext r(Game()->GetRenderer());
	r.SetColorSwap(GetTeam()->GetColor());

	r.SetAlpha(GetVisibility());
	if (r.GetAlpha() < 1)
		r.SetBlend(BLEND_ALPHA);

	float flSlowSpeed = 50.0f;
	float flFastSpeed = 200.0f;

	m_flFanRotationSpeed = Approach(HasConstruction()?flFastSpeed:flSlowSpeed, m_flFanRotationSpeed, Game()->GetFrameTime()*(flFastSpeed-flSlowSpeed));
	m_flFanRotation -= RemapVal(Game()->GetFrameTime(), 0, 1, 0, m_flFanRotationSpeed);

	r.Rotate(m_flFanRotation, Vector(0, 1, 0));

	r.RenderModel(m_iFanModel);
}

void CCPU::PostRender()
{
	BaseClass::PostRender();

	if (DigitanksGame()->GetControlMode() == MODE_BUILD)
	{
		CRenderingContext r(Game()->GetRenderer());
		r.Translate(GetPreviewBuild() + Vector(0, 1, 0));
		r.Rotate(-GetAngles().y, Vector(0, 1, 0));

		if (IsPreviewBuildValid())
			r.SetColor(Color(255, 255, 255));
		else
			r.SetColor(Color(255, 0, 0));

		glutSolidCube(4);
	}
}

void CCPU::UpdateInfo(std::string& sInfo)
{
	std::stringstream s;

	s << "CENTRAL PROCESSING UNIT\n";
	s << "Command center\n \n";

	if (IsConstructing())
	{
		s << "(Constructing)\n";
		s << "Power to build: " << GetProductionRemaining() << "\n";
		s << "Turns left: " << GetTurnsToConstruct() << "\n";
		sInfo = s.str();
		return;
	}

	s << "Strength: " << m_iDataStrength << "\n";
	s << "Growth: " << (int)GetDataFlowRate() << "\n";
	s << "Size: " << (int)GetDataFlowRadius() << "\n";
	s << "Efficiency: " << (int)(GetChildEfficiency()*100) << "%\n";

	sInfo = s.str();
}
