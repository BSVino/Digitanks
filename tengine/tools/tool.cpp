#include "tool.h"

#include <glgui/menu.h>
#include <game/gameserver.h>
#include <cvar.h>

#include "workbench.h"

CWorkbenchTool::CWorkbenchTool()
{
	m_flEditOrthoHeight = 10;
}

void CWorkbenchTool::Think()
{
}

void CWorkbenchTool::Activate()
{
	Workbench()->GetCameraManager()->SetPermaFreeMode(true);

	if (ShowCameraControls())
	{
		glgui::CMenu* pViewMenu = glgui::CRootPanel::Get()->AddMenu("View");
		pViewMenu->AddSubmenu("3D", this, PerspViewSelected);
		pViewMenu->AddSubmenu("Front", this, FrontViewSelected);
		pViewMenu->AddSubmenu("Back", this, BackViewSelected);
		pViewMenu->AddSubmenu("Left", this, LeftViewSelected);
		pViewMenu->AddSubmenu("Right", this, RightViewSelected);
		pViewMenu->AddSubmenu("Top", this, TopViewSelected);
		pViewMenu->AddSubmenu("Bottom", this, BottomViewSelected);

		glgui::CRootPanel::Get()->Layout();
	}
}

void CWorkbenchTool::Deactivate()
{
	CVar::SetCVar("cam_free", "off");

	Workbench()->GetCameraManager()->SetPermaFreeMode(false);
}

glgui::CMenu* CWorkbenchTool::GetFileMenu()
{
	return Workbench()->m_pFileMenu;
}

void CWorkbenchTool::CameraThink()
{
	if (Workbench()->GetCameraManager()->GetFreeMode())
	{
		m_vecEditCamera = Workbench()->GetCameraManager()->GetFreeCameraPosition();
		m_angEditCamera = Workbench()->GetCameraManager()->GetFreeCameraAngles();
		m_flEditOrthoHeight = Workbench()->GetCameraManager()->GetFreeCameraOrthoHeight();
	}
}

TVector CWorkbenchTool::GetCameraPosition()
{
	return m_vecEditCamera;
}

Vector CWorkbenchTool::GetCameraDirection()
{
	return AngleVector(m_angEditCamera);
}

void CWorkbenchTool::SetCameraOrientation(TVector vecPosition, Vector vecDirection)
{
	m_vecEditCamera = vecPosition;
	m_angEditCamera = VectorAngles(vecDirection);
}

float CWorkbenchTool::GetCameraOrthoHeight()
{
	return m_flEditOrthoHeight;
}

bool CWorkbenchTool::ShouldRenderOrthographic()
{
	return !m_b3DView;
}

void CWorkbenchTool::PerspViewSelectedCallback(const tstring& sArgs)
{
	m_b3DView = true;
}

void CWorkbenchTool::FrontViewSelectedCallback(const tstring& sArgs)
{
	m_b3DView = false;
	m_angEditCamera = EAngle(0, 0, 0);
	Workbench()->GetCameraManager()->SetPermaFreeMode(true);	// Force a re-read
}

void CWorkbenchTool::BackViewSelectedCallback(const tstring& sArgs)
{
	m_b3DView = false;
	m_angEditCamera = EAngle(0, 180, 0);
	Workbench()->GetCameraManager()->SetPermaFreeMode(true);	// Force a re-read
}

void CWorkbenchTool::LeftViewSelectedCallback(const tstring& sArgs)
{
	m_b3DView = false;
	m_angEditCamera = EAngle(0, 90, 0);
	Workbench()->GetCameraManager()->SetPermaFreeMode(true);	// Force a re-read
}

void CWorkbenchTool::RightViewSelectedCallback(const tstring& sArgs)
{
	m_b3DView = false;
	m_angEditCamera = EAngle(0, -90, 0);
	Workbench()->GetCameraManager()->SetPermaFreeMode(true);	// Force a re-read
}

void CWorkbenchTool::TopViewSelectedCallback(const tstring& sArgs)
{
	m_b3DView = false;
	m_angEditCamera = EAngle(90, 0, 0);
	Workbench()->GetCameraManager()->SetPermaFreeMode(true);	// Force a re-read
}

void CWorkbenchTool::BottomViewSelectedCallback(const tstring& sArgs)
{
	m_b3DView = false;
	m_angEditCamera = EAngle(-90, 0, 0);
	Workbench()->GetCameraManager()->SetPermaFreeMode(true);	// Force a re-read
}
