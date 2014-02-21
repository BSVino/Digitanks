#include "toyviewer.h"

#include <tinker_platform.h>
#include <files.h>

#include <glgui/rootpanel.h>
#include <glgui/menu.h>
#include <glgui/filedialog.h>
#include <glgui/checkbox.h>
#include <models/models.h>
#include <tinker/application.h>
#include <renderer/game_renderingcontext.h>
#include <renderer/game_renderer.h>
#include <game/gameserver.h>
#include <tinker/keys.h>
#include <ui/gamewindow.h>
#include <toys/toy.h>

#include "workbench.h"

REGISTER_WORKBENCH_TOOL(ToyViewer);

CToyPreviewPanel::CToyPreviewPanel()
{
	SetBackgroundColor(Color(0, 0, 0, 150));
	SetBorder(glgui::CPanel::BT_SOME);

	m_pInfo = new glgui::CLabel("", "sans-serif", 16);
	AddControl(m_pInfo);

	m_pShowPhysicsLabel = new glgui::CLabel("Show physics:", "sans-serif", 10);
	m_pShowPhysicsLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pShowPhysicsLabel);
	m_pShowPhysics = new glgui::CCheckBox();
	AddControl(m_pShowPhysics);
}

void CToyPreviewPanel::Layout()
{
	float flWidth = glgui::CRootPanel::Get()->GetWidth();
	float flHeight = glgui::CRootPanel::Get()->GetHeight();

	float flMenuBarBottom = glgui::CRootPanel::Get()->GetMenuBar()->GetBottom();

	float flCurrLeft = 20;
	float flCurrTop = flMenuBarBottom + 10;

	SetDimensions(flCurrLeft, flCurrTop, 200, flHeight-30-flMenuBarBottom);

	tstring sFilename = ToyViewer()->GetToyPreview();
	tstring sAbsoluteGamePath = FindAbsolutePath(".");
	tstring sAbsoluteFilename = FindAbsolutePath(sFilename);
	if (sAbsoluteFilename.find(sAbsoluteGamePath) == 0)
		sFilename = ToForwardSlashes(sAbsoluteFilename.substr(sAbsoluteGamePath.length()));
	m_pInfo->SetText(sFilename);

	m_pInfo->SetPos(0, 15);
	m_pInfo->SetSize(GetWidth(), 25);

	m_pShowPhysicsLabel->Layout_AlignTop(m_pInfo);
	m_pShowPhysicsLabel->SetWidth(10);
	m_pShowPhysicsLabel->SetHeight(1);
	m_pShowPhysicsLabel->EnsureTextFits();
	m_pShowPhysicsLabel->Layout_FullWidth();

	m_pShowPhysics->SetTop(m_pShowPhysicsLabel->GetTop()+12);
	m_pShowPhysics->SetLeft(m_pShowPhysicsLabel->GetLeft());

	BaseClass::Layout();
}

CToyViewer* CToyViewer::s_pToyViewer = nullptr;

CToyViewer::CToyViewer()
{
	s_pToyViewer = this;

	m_pToyPreviewPanel = new CToyPreviewPanel();
	m_pToyPreviewPanel->SetVisible(false);
	glgui::CRootPanel::Get()->AddControl(m_pToyPreviewPanel);

	m_iToyPreview = ~0;

	m_bRotatingPreview = false;
	m_angPreview = EAngle(-20, 20, 0);
	m_flPreviewDistance = 10;
}

CToyViewer::~CToyViewer()
{
}

void CToyViewer::Activate()
{
	Layout();

	if (!m_sToyPreview.length() || m_iToyPreview == ~0)
		ChooseToyCallback("");

	BaseClass::Activate();
}

void CToyViewer::Deactivate()
{
	BaseClass::Deactivate();

	m_pToyPreviewPanel->SetVisible(false);
}

void CToyViewer::Layout()
{
	m_pToyPreviewPanel->SetVisible(false);

	if (m_iToyPreview != ~0)
		m_pToyPreviewPanel->SetVisible(true);

	SetupMenu();
}

void CToyViewer::SetupMenu()
{
	GetFileMenu()->ClearSubmenus();

	GetFileMenu()->AddSubmenu("Open", this, ChooseToy);
}

void CToyViewer::RenderScene()
{
	CModel* pModel = CModelLibrary::GetModel(m_iToyPreview);

	if (m_iToyPreview != ~0)
	{
		TAssert(pModel);
		if (!pModel)
			m_iToyPreview = ~0;
	}

	GameServer()->GetRenderer()->SetRenderingTransparent(false);

	if (m_iToyPreview != ~0 && pModel)
	{
		CGameRenderingContext c(GameServer()->GetRenderer(), true);

		if (!c.GetActiveFrameBuffer())
			c.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

		c.SetColor(Color(255, 255, 255));

		c.RenderModel(m_iToyPreview);

		if (m_pToyPreviewPanel->m_pShowPhysics->GetState() && pModel->m_pToy)
		{
			CGameRenderingContext c(GameServer()->GetRenderer(), true);

			c.ClearDepth();
			c.UseProgram("model");
			c.SetUniform("bDiffuse", false);
			c.SetColor(Color(0, 100, 155, (int)(255*0.3f)));
			c.SetBlend(BLEND_ALPHA);
			c.SetUniform("vecColor", Color(0, 100, 155, (char)(255*0.3f)));

			for (size_t i = 0; i < pModel->m_pToy->GetPhysicsNumBoxes(); i++)
			{
				CGameRenderingContext c(GameServer()->GetRenderer(), true);

				c.Transform(pModel->m_pToy->GetPhysicsBox(i).GetMatrix4x4());
				c.RenderWireBox(CToy::s_aabbBoxDimensions);
			}

			if (pModel->m_pToy->GetPhysicsNumTris())
			{
				CGameRenderingContext c(GameServer()->GetRenderer(), true);

				c.BeginRenderVertexArray();
				c.SetPositionBuffer(pModel->m_pToy->GetPhysicsVerts());
				c.EndRenderVertexArrayTriangles(pModel->m_pToy->GetPhysicsNumTris(), pModel->m_pToy->GetPhysicsTris());
			}

			// Reset for other stuff.
			c.SetUniform("bDiffuse", true);
			c.SetUniform("vecColor", Color(255, 255, 255, 255));
		}
	}
}

void CToyViewer::ChooseToyCallback(const tstring& sArgs)
{
	glgui::CFileDialog::ShowOpenDialog(".", ".toy", this, OpenToy);
}

void CToyViewer::OpenToyCallback(const tstring& sArgs)
{
	tstring sGamePath = GetRelativePath(sArgs, ".");

	CModelLibrary::ReleaseModel(m_iToyPreview);

	m_iToyPreview = CModelLibrary::AddModel(sGamePath);

	if (m_iToyPreview != ~0)
	{
		m_sToyPreview = sGamePath;
		m_flPreviewDistance = CModelLibrary::GetModel(m_iToyPreview)->m_aabbVisBoundingBox.Size().Length()*2;
	}

	Layout();
}

bool CToyViewer::MouseInput(int iButton, tinker_mouse_state_t iState)
{
	if (iButton == TINKER_KEY_MOUSE_LEFT)
	{
		m_bRotatingPreview = (iState == TINKER_MOUSE_PRESSED);
		return true;
	}

	return false;
}

void CToyViewer::MouseMotion(int x, int y)
{
	if (m_bRotatingPreview)
	{
		int lx, ly;
		if (GameWindow()->GetLastMouse(lx, ly))
		{
			m_angPreview.y -= (float)(x-lx);
			m_angPreview.p -= (float)(y-ly);
		}
	}
}

void CToyViewer::MouseWheel(int x, int y)
{
	if (y > 0)
	{
		for (int i = 0; i < y; i++)
			m_flPreviewDistance *= 0.9f;
	}
	else if (y < 0)
	{
		for (int i = 0; i < -y; i++)
			m_flPreviewDistance *= 1.1f;
	}
}

TVector CToyViewer::GetCameraPosition()
{
	if (m_iToyPreview == ~0)
		return TVector(0, 0, 0);

	CModel* pMesh = CModelLibrary::GetModel(m_iToyPreview);

	if (!pMesh)
		return TVector(0, 0, 0);

	return pMesh->m_aabbVisBoundingBox.Center() - AngleVector(m_angPreview)*m_flPreviewDistance;
}

Vector CToyViewer::GetCameraDirection()
{
	return AngleVector(m_angPreview);
}
