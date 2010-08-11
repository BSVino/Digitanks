#include "cpu.h"

#include <sstream>
#include <GL/glew.h>

#include <mtrand.h>
#include <renderer/renderer.h>
#include <renderer/dissolver.h>

#include <game/team.h>
#include <ui/digitankswindow.h>
#include <ui/instructor.h>
#include <ui/hud.h>
#include <models/models.h>

#include "buffer.h"
#include "collector.h"
#include "loader.h"

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

	PrecacheModel(L"models/structures/cpu.obj");
	PrecacheModel(L"models/structures/cpu-fan.obj");
}

void CCPU::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = CDigitanksWindow::Get()->GetHUD();

	bool bDisableBuffer = !GetDigitanksTeam()->CanBuildBuffers();
	bool bDisablePSU = !GetDigitanksTeam()->CanBuildPSUs();
	bool bDisableLoaders = !GetDigitanksTeam()->CanBuildLoaders();

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
	else if (IsInstalling())
	{
		pHUD->SetButton1Listener(NULL);
		pHUD->SetButton2Listener(NULL);
		pHUD->SetButton3Listener(NULL);
		pHUD->SetButton4Listener(NULL);
		pHUD->SetButton5Listener(CHUD::CancelInstall);

		pHUD->SetButton1Texture(0);
		pHUD->SetButton2Texture(0);
		pHUD->SetButton3Texture(0);
		pHUD->SetButton4Texture(0);
		pHUD->SetButton5Texture(0);

		pHUD->SetButton1Help("");
		pHUD->SetButton2Help("");
		pHUD->SetButton3Help("");
		pHUD->SetButton4Help("");
		pHUD->SetButton5Help("Cancel\nInstall");

		pHUD->SetButton1Color(glgui::g_clrBox);
		pHUD->SetButton2Color(glgui::g_clrBox);
		pHUD->SetButton3Color(glgui::g_clrBox);
		pHUD->SetButton4Color(glgui::g_clrBox);
		pHUD->SetButton5Color(Color(100, 0, 0));
	}
	else if (!bDisableLoaders && eMenuMode == MENUMODE_LOADERS)
	{
		if (GetDigitanksTeam()->CanBuildInfantryLoaders())
		{
			pHUD->SetButton1Listener(CHUD::BuildInfantryLoader);
			pHUD->SetButton1Texture(0);
			pHUD->SetButton1Help("Infantry\nLoader");
			pHUD->SetButton1Color(Color(150, 150, 150));
		}
		else
		{
			pHUD->SetButton1Listener(NULL);
			pHUD->SetButton1Texture(0);
			pHUD->SetButton1Help("");
			pHUD->SetButton1Color(glgui::g_clrBox);
		}

		if (GetDigitanksTeam()->CanBuildTankLoaders())
		{
			pHUD->SetButton2Listener(CHUD::BuildTankLoader);
			pHUD->SetButton2Texture(0);
			pHUD->SetButton2Help("Main Tank\nLoader");
			pHUD->SetButton2Color(Color(150, 150, 150));
		}
		else
		{
			pHUD->SetButton2Listener(NULL);
			pHUD->SetButton2Texture(0);
			pHUD->SetButton2Help("");
			pHUD->SetButton2Color(glgui::g_clrBox);
		}

		if (GetDigitanksTeam()->CanBuildArtilleryLoaders())
		{
			pHUD->SetButton3Listener(CHUD::BuildArtilleryLoader);
			pHUD->SetButton3Texture(0);
			pHUD->SetButton3Help("Artillery\nLoader");
			pHUD->SetButton3Color(Color(150, 150, 150));
		}
		else
		{
			pHUD->SetButton3Listener(NULL);
			pHUD->SetButton3Texture(0);
			pHUD->SetButton3Help("");
			pHUD->SetButton3Color(glgui::g_clrBox);
		}

		pHUD->SetButton4Listener(NULL);
		pHUD->SetButton4Texture(0);
		pHUD->SetButton4Help("");
		pHUD->SetButton4Color(glgui::g_clrBox);

		pHUD->SetButton5Listener(CHUD::GoToMain);
		pHUD->SetButton5Texture(0);
		pHUD->SetButton5Help("Return");
		pHUD->SetButton5Color(Color(100, 0, 0));
	}
	else if (eMenuMode == MENUMODE_INSTALL)
	{
		if (GetFirstUninstalledUpdate(UPDATETYPE_PRODUCTION) >= 0)
		{
			pHUD->SetButton1Listener(CHUD::InstallProduction);
			pHUD->SetButton1Texture(0);
			pHUD->SetButton1Help("Install\nPower");
			pHUD->SetButton1Color(Color(150, 150, 150));
		}
		else
		{
			pHUD->SetButton1Listener(NULL);
			pHUD->SetButton1Texture(0);
			pHUD->SetButton1Help("");
			pHUD->SetButton1Color(glgui::g_clrBox);
		}

		if (GetFirstUninstalledUpdate(UPDATETYPE_BANDWIDTH) >= 0)
		{
			pHUD->SetButton2Listener(CHUD::InstallBandwidth);
			pHUD->SetButton2Texture(0);
			pHUD->SetButton2Help("Install\nBandwidth");
			pHUD->SetButton2Color(Color(150, 150, 150));
		}
		else
		{
			pHUD->SetButton2Listener(NULL);
			pHUD->SetButton2Texture(0);
			pHUD->SetButton2Help("");
			pHUD->SetButton2Color(glgui::g_clrBox);
		}

		if (GetFirstUninstalledUpdate(UPDATETYPE_FLEETSUPPLY) >= 0)
		{
			pHUD->SetButton3Listener(CHUD::InstallFleetSupply);
			pHUD->SetButton3Texture(0);
			pHUD->SetButton3Help("Install\nFleet Sply");
			pHUD->SetButton3Color(Color(150, 150, 150));
		}
		else
		{
			pHUD->SetButton3Listener(NULL);
			pHUD->SetButton3Texture(0);
			pHUD->SetButton3Help("");
			pHUD->SetButton3Color(glgui::g_clrBox);
		}

		pHUD->SetButton4Listener(NULL);
		pHUD->SetButton4Texture(0);
		pHUD->SetButton4Help("");
		pHUD->SetButton4Color(glgui::g_clrBox);

		pHUD->SetButton5Listener(CHUD::GoToMain);
		pHUD->SetButton5Texture(0);
		pHUD->SetButton5Help("Return");
		pHUD->SetButton5Color(Color(100, 0, 0));
	}
	else
	{
		if (bDisableBuffer)
		{
			pHUD->SetButton1Listener(NULL);
			pHUD->SetButton1Help("");
			pHUD->SetButton1Texture(0);
			pHUD->SetButton1Color(glgui::g_clrBox);
		}
		else
		{
			pHUD->SetButton1Listener(CHUD::BuildBuffer);
			pHUD->SetButton1Help("Build\nBuffer");
			pHUD->SetButton1Texture(0);
			pHUD->SetButton1Color(Color(150, 150, 150));
		}

		if (bDisablePSU)
		{
			pHUD->SetButton2Listener(NULL);
			pHUD->SetButton2Help("");
			pHUD->SetButton2Texture(0);
			pHUD->SetButton2Color(glgui::g_clrBox);
		}
		else
		{
			pHUD->SetButton2Listener(CHUD::BuildPSU);
			pHUD->SetButton2Help("Build\nPwr Supply");
			pHUD->SetButton2Texture(0);
			pHUD->SetButton2Color(Color(150, 150, 150));
		}

		if (bDisableLoaders)
		{
			pHUD->SetButton3Listener(NULL);
			pHUD->SetButton3Help("");
			pHUD->SetButton3Texture(0);
			pHUD->SetButton3Color(glgui::g_clrBox);
		}
		else
		{
			pHUD->SetButton3Listener(CHUD::BuildLoader);
			pHUD->SetButton3Help("Build\nLoader");
			pHUD->SetButton3Texture(0);
			pHUD->SetButton3Color(Color(150, 150, 150));
		}

		if (HasUpdatesAvailable())
		{
			pHUD->SetButton4Listener(CHUD::InstallMenu);
			pHUD->SetButton4Help("Install\nUpdates");
			pHUD->SetButton4Texture(0);
			pHUD->SetButton4Color(Color(150, 150, 150));
		}
		else
		{
			pHUD->SetButton4Listener(NULL);
			pHUD->SetButton4Help("");
			pHUD->SetButton4Texture(0);
			pHUD->SetButton4Color(glgui::g_clrBox);
		}

		pHUD->SetButton5Listener(NULL);
		pHUD->SetButton5Help("");
		pHUD->SetButton5Texture(0);
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
		m_hConstructing->AddProduction(m_hConstructing->GetProductionToConstruct());
	}

	if (m_ePreviewStructure == STRUCTURE_PSU && iTutorial == CInstructor::TUTORIAL_PSU)
	{
		// Make sure it's done next turn.
		m_hConstructing->AddProduction(m_hConstructing->GetProductionToConstruct());
	}

	if (iTutorial == CInstructor::TUTORIAL_LOADER && (m_ePreviewStructure == STRUCTURE_INFANTRYLOADER || m_ePreviewStructure == STRUCTURE_TANKLOADER || m_ePreviewStructure == STRUCTURE_ARTILLERYLOADER))
	{
		// Make sure it's done next turn.
		m_hConstructing->AddProduction(m_hConstructing->GetProductionToConstruct());
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

bool CCPU::HasUpdatesAvailable()
{
	if (GetFirstUninstalledUpdate(UPDATETYPE_BANDWIDTH) >= 0)
		return true;

	if (GetFirstUninstalledUpdate(UPDATETYPE_FLEETSUPPLY) >= 0)
		return true;

	if (GetFirstUninstalledUpdate(UPDATETYPE_PRODUCTION) >= 0)
		return true;

	return false;
}

void CCPU::StartTurn()
{
	BaseClass::StartTurn();

	if (m_hConstructing != NULL && m_hConstructing->IsConstructing())
	{
		if (!m_hConstructing->GetSupplier() || m_hConstructing->GetSupplier()->IsDeleted())
		{
			CancelConstruction();
		}
	}

	if (m_hConstructing != NULL && m_hConstructing->IsConstructing())
	{
		if (GetDigitanksTeam()->GetProductionPerLoader() > m_hConstructing->GetProductionToConstruct())
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
		{
			r.SetColorSwap(Color(255, 255, 255));
			r.SetAlpha(0.5f);
			r.SetBlend(BLEND_ALPHA);
		}
		else
		{
			r.SetColorSwap(Color(255, 0, 0));
			r.SetAlpha(0.3f);
			r.SetBlend(BLEND_ADDITIVE);
		}

		size_t iModel = 0;
		switch (m_ePreviewStructure)
		{
		case STRUCTURE_BUFFER:
			iModel = CModelLibrary::Get()->FindModel(L"models/structures/buffer.obj");
			break;

		case STRUCTURE_PSU:
			iModel = CModelLibrary::Get()->FindModel(L"models/structures/psu.obj");
			break;

		case STRUCTURE_INFANTRYLOADER:
			iModel = CModelLibrary::Get()->FindModel(L"models/structures/loader-infantry.obj");
			break;

		case STRUCTURE_TANKLOADER:
			iModel = CModelLibrary::Get()->FindModel(L"models/structures/loader-main.obj");
			break;

		case STRUCTURE_ARTILLERYLOADER:
			iModel = CModelLibrary::Get()->FindModel(L"models/structures/loader-artillery.obj");
			break;
		}

		r.RenderModel(iModel);
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
		s << "Power to build: " << GetProductionToConstruct() << "\n";
		s << "Turns left: " << GetTurnsToConstruct() << "\n";
		sInfo = s.str();
		return;
	}

	if (HasConstruction())
	{
		s << "[Constructing " << m_hConstructing->GetName() << "...]\n";
		s << "Power to build: " << m_hConstructing->GetProductionToConstruct() << "\n";
		s << "Turns left: " << m_hConstructing->GetTurnsToConstruct() << "\n";
		sInfo = s.str();
		return;
	}

	if (IsInstalling())
	{
		s << "[Installing update '" << GetUpdateInstalling()->GetName() << "'...]\n";
		s << "Power to install: " << GetProductionToInstall() << "\n";
		s << "Turns left: " << GetTurnsToInstall() << "\n";
		sInfo = s.str();
		return;
	}

	s << "Strength: " << m_iDataStrength << "\n";
	s << "Growth: " << (int)GetDataFlowRate() << "\n";
	s << "Size: " << (int)GetDataFlowRadius() << "\n";
	s << "Efficiency: " << (int)(GetChildEfficiency()*100) << "%\n";

	sInfo = s.str();
}

void CCPU::OnDeleted()
{
	for (size_t i = GetTeam()->GetNumMembers()-1; i < GetTeam()->GetNumMembers(); i--)
	{
		if (GetTeam()->GetMember(i) == this)
			continue;

		CModelDissolver::AddModel(GetTeam()->GetMember(i));
		GetTeam()->GetMember(i)->Delete();
	}
}
