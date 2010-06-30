#include "cpu.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <renderer/renderer.h>

#include <game/team.h>
#include <ui/digitankswindow.h>
#include <ui/hud.h>

#include "buffer.h"

REGISTER_ENTITY(CCPU);

CCPU::CCPU()
{
}

void CCPU::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = CDigitanksWindow::Get()->GetHUD();

	pHUD->SetButton1Listener(NULL);
	pHUD->SetButton2Listener(NULL);
	pHUD->SetButton3Listener(NULL);
	if (m_hConstructing == NULL)
		pHUD->SetButton4Listener(CHUD::BuildBuffer);
	else
		pHUD->SetButton4Listener(CHUD::CancelBuild);
	pHUD->SetButton5Listener(NULL);

	pHUD->SetButton1Help("");
	pHUD->SetButton2Help("");
	pHUD->SetButton3Help("");
	if (m_hConstructing == NULL)
		pHUD->SetButton4Help("Build\nBuffer");
	else
		pHUD->SetButton4Help("Cancel\nBuild");
	pHUD->SetButton5Help("");

	pHUD->SetButton1Texture(0);
	pHUD->SetButton2Texture(0);
	pHUD->SetButton3Texture(0);
	pHUD->SetButton4Texture(0);
	pHUD->SetButton5Texture(0);

	pHUD->SetButton1Color(glgui::g_clrBox);
	pHUD->SetButton2Color(glgui::g_clrBox);
	pHUD->SetButton3Color(glgui::g_clrBox);
	pHUD->SetButton4Color(Color(150, 150, 150));
	pHUD->SetButton5Color(glgui::g_clrBox);
}

bool CCPU::IsPreviewBuildValid() const
{
	CSupplier* pSupplier = FindClosestSupplier(GetPreviewBuild(), GetTeam());

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
	if (m_hConstructing != NULL)
		CancelConstruction();

	m_hConstructing = Game()->Create<CBuffer>("CBuffer");
	GetTeam()->AddEntity(m_hConstructing);
	m_hConstructing->BeginConstruction(3);
	m_hConstructing->SetOrigin(GetPreviewBuild());
	m_hConstructing->SetSupplier(FindClosestSupplier(GetPreviewBuild(), GetTeam()));
	m_hConstructing->GetSupplier()->AddChild(m_hConstructing);

	CSupplier* pSupplier = dynamic_cast<CSupplier*>(m_hConstructing.GetPointer());
	if (pSupplier)
		pSupplier->GiveDataStrength((size_t)pSupplier->GetSupplier()->GetDataFlow(pSupplier->GetOrigin()));
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

	CalculateDataFlow();
}

void CCPU::PostStartTurn()
{
	BaseClass::PostStartTurn();

	if (m_hConstructing != NULL && !m_hConstructing->IsConstructing())
	{
		m_hConstructing = NULL;
	}
}

void CCPU::OnRender()
{
	glColor4ubv(GetTeam()->GetColor());
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
