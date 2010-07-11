#include "cpu.h"

#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <renderer/renderer.h>

#include <game/team.h>
#include <ui/digitankswindow.h>
#include <ui/hud.h>

#include "buffer.h"
#include "collector.h"
#include "loader.h"

REGISTER_ENTITY(CCPU);

CCPU::CCPU()
{
}

void CCPU::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = CDigitanksWindow::Get()->GetHUD();

	if (m_hConstructing == NULL)
		pHUD->SetButton1Listener(CHUD::BuildBuffer);
	else
		pHUD->SetButton1Listener(CHUD::CancelBuild);
	if (m_hConstructing == NULL)
		pHUD->SetButton2Listener(CHUD::BuildPSU);
	else
		pHUD->SetButton2Listener(CHUD::CancelBuild);
	if (m_hConstructing == NULL)
		pHUD->SetButton3Listener(CHUD::BuildInfantryLoader);
	else
		pHUD->SetButton3Listener(CHUD::CancelBuild);
	if (m_hConstructing == NULL)
		pHUD->SetButton4Listener(CHUD::BuildTankLoader);
	else
		pHUD->SetButton4Listener(CHUD::CancelBuild);
	pHUD->SetButton5Listener(NULL);

	if (m_hConstructing == NULL)
		pHUD->SetButton1Help("Build\nBuffer");
	else
		pHUD->SetButton1Help("Cancel\nBuild");
	if (m_hConstructing == NULL)
		pHUD->SetButton2Help("Build\nPwr Supply");
	else
		pHUD->SetButton2Help("Cancel\nBuild");
	if (m_hConstructing == NULL)
		pHUD->SetButton3Help("Build\nInf Ldr");
	else
		pHUD->SetButton3Help("Cancel\nBuild");
	if (m_hConstructing == NULL)
		pHUD->SetButton4Help("Build\nTank Ldr");
	else
		pHUD->SetButton4Help("Cancel\nBuild");
	pHUD->SetButton5Help("");

	pHUD->SetButton1Texture(0);
	pHUD->SetButton2Texture(0);
	pHUD->SetButton3Texture(0);
	pHUD->SetButton4Texture(0);
	pHUD->SetButton5Texture(0);

	pHUD->SetButton1Color(Color(150, 150, 150));
	pHUD->SetButton2Color(Color(150, 150, 150));
	pHUD->SetButton3Color(Color(150, 150, 150));
	pHUD->SetButton4Color(Color(150, 150, 150));
	pHUD->SetButton5Color(glgui::g_clrBox);
}

bool CCPU::OnControlModeChange(controlmode_t eOldMode, controlmode_t eNewMode)
{
	if (eNewMode == MODE_BUILD)
		return true;

	if (eNewMode == MODE_NONE)
		return true;

	return false;
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

	GetTeam()->AddEntity(m_hConstructing);
	m_hConstructing->BeginConstruction();
	m_hConstructing->SetOrigin(GetPreviewBuild());
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
		else
			pLoader->SetBuildUnit(BUILDUNIT_TANK);
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
			if (pCollector)
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
	if (GetVisibility() == 0)
		return;

	glutSolidCube(12);
}

void CCPU::PostRender()
{
	BaseClass::PostRender();

	if (DigitanksGame()->GetControlMode() == MODE_BUILD)
	{
		if (IsPreviewBuildValid())
		{
			CRenderingContext r(Game()->GetRenderer());
			r.Translate(GetPreviewBuild() + Vector(0, 1, 0));
			r.Rotate(-GetAngles().y, Vector(0, 1, 0));
			r.SetAlpha(50.0f/255);
			r.SetBlend(BLEND_ALPHA);
			r.SetColor(Color(255, 255, 255));
			glutSolidCube(4);
			r.RenderModel(GetModel());
		}
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
		s << "Production left: " << GetProductionRemaining() << "\n";
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
