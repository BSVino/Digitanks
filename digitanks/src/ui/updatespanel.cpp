#include "updatespanel.h"

#include <renderer/renderer.h>

#include <game/digitanks/digitanksgame.h>
#include <ui/digitankswindow.h>
#include <ui/hud.h>

using namespace glgui;

CUpdatesPanel::CUpdatesPanel()
	: CPanel(0, 0, 600, 600)
{
	m_pCloseButton = new CButton(0, 0, 100, 20, L"Close");
	m_pCloseButton->SetClickedListener(this, Close);
	AddControl(m_pCloseButton);

	m_pInfo = new CLabel(0, 0, 100, 300, L"");
	AddControl(m_pInfo);

	m_pTutorial = new CLabel(0, 0, 100, 300, L"");
	AddControl(m_pTutorial);

	m_iIconCPU = CRenderer::LoadTextureIntoGL(L"textures/hud/update-cpu.png");
	m_iIconBuffer = CRenderer::LoadTextureIntoGL(L"textures/hud/update-buffer.png");
	m_iIconPSU = CRenderer::LoadTextureIntoGL(L"textures/hud/update-psu.png");
	m_iIconInfantryLoader = CRenderer::LoadTextureIntoGL(L"textures/hud/update-infantry-loader.png");
	m_iIconTankLoader = CRenderer::LoadTextureIntoGL(L"textures/hud/update-tank-loader.png");
	m_iIconArtilleryLoader = CRenderer::LoadTextureIntoGL(L"textures/hud/update-artillery-loader.png");
	m_iIconInfantryAttack = CRenderer::LoadTextureIntoGL(L"textures/hud/update-infantry-attack.png");
	m_iIconInfantryDefense = CRenderer::LoadTextureIntoGL(L"textures/hud/update-infantry-defense.png");
	m_iIconInfantryMovement = CRenderer::LoadTextureIntoGL(L"textures/hud/update-infantry-movement.png");
	m_iIconInfantryHealth = CRenderer::LoadTextureIntoGL(L"textures/hud/update-infantry-health.png");
	m_iIconTankAttack = CRenderer::LoadTextureIntoGL(L"textures/hud/update-tank-attack.png");
	m_iIconTankDefense = CRenderer::LoadTextureIntoGL(L"textures/hud/update-tank-defense.png");
	m_iIconTankMovement = CRenderer::LoadTextureIntoGL(L"textures/hud/update-tank-movement.png");
	m_iIconTankHealth = CRenderer::LoadTextureIntoGL(L"textures/hud/update-tank-health.png");
	m_iIconArtilleryAttack = CRenderer::LoadTextureIntoGL(L"textures/hud/update-artillery-attack.png");
	m_iIconArtilleryRange = CRenderer::LoadTextureIntoGL(L"textures/hud/update-artillery-range.png");
	m_iIconArtilleryMovement = CRenderer::LoadTextureIntoGL(L"textures/hud/update-artillery-movement.png");
	m_iIconArtilleryHealth = CRenderer::LoadTextureIntoGL(L"textures/hud/update-artillery-health.png");
	m_iIconBufferRecharge = CRenderer::LoadTextureIntoGL(L"textures/hud/update-buffer-recharge.png");
	m_iIconBufferEnergy = CRenderer::LoadTextureIntoGL(L"textures/hud/update-buffer-energy.png");
	m_iIconBufferFleet = CRenderer::LoadTextureIntoGL(L"textures/hud/update-buffer-fleet.png");
	m_iIconBufferBandwidth = CRenderer::LoadTextureIntoGL(L"textures/hud/update-buffer-bandwidth.png");
	m_iIconCPUPower = CRenderer::LoadTextureIntoGL(L"textures/hud/update-cpu-power.png");
	m_iIconCPUFleet = CRenderer::LoadTextureIntoGL(L"textures/hud/update-cpu-fleet.png");
	m_iIconCPUBandwidth = CRenderer::LoadTextureIntoGL(L"textures/hud/update-cpu-bandwidth.png");
}

void CUpdatesPanel::Layout()
{
	CPanel::Layout();

	SetPos(CRootPanel::Get()->GetWidth()/2-GetWidth()/2, CRootPanel::Get()->GetHeight()/2-GetHeight()/2);

	m_pCloseButton->SetPos(GetWidth()-m_pCloseButton->GetWidth()-20, 20);

	m_pInfo->SetSize(200, 200);
	m_pInfo->SetPos(GetWidth()+2, GetHeight()/2 - m_pInfo->GetHeight()/2);

	m_pTutorial->SetSize(GetWidth(), 20);
	m_pTutorial->SetPos(0, GetHeight() - 30);

	for (size_t i = 0; i < m_apUpdates.size(); i++)
	{
		m_apUpdates[i]->Destructor();
		m_apUpdates[i]->Delete();
	}

	m_apUpdates.clear();

	if (!DigitanksGame() || !DigitanksGame()->GetUpdateGrid())
		return;

	CDigitanksTeam* pCurrentGame = DigitanksGame()->GetCurrentLocalDigitanksTeam();
	if (!pCurrentGame)
		return;

	CUpdateGrid* pUpdates = DigitanksGame()->GetUpdateGrid();

	int iLowestX = pUpdates->m_iLowestX;
	int iLowestY = pUpdates->m_iLowestY;

	int iXSize = pUpdates->m_iHighestX - iLowestX + 1;
	int iYSize = pUpdates->m_iHighestY - iLowestY + 1;

	int iLarger = ((iXSize > iYSize) ? iXSize : iYSize);

	if (iLarger < 11)
	{
		iLarger = 11;
		iLowestX = (pUpdates->m_iHighestX + pUpdates->m_iLowestX)/2 - iLarger/2;
		iLowestY = (pUpdates->m_iHighestY + pUpdates->m_iLowestY)/2 - iLarger/2;
	}

	m_iButtonSize = GetWidth()/(iLarger+2);
	int iXWidth = m_iButtonSize*iXSize;
	int iYWidth = m_iButtonSize*iYSize;
	int iXBuffer = (GetWidth() - iXWidth)/2;
	int iYBuffer = (GetWidth() - iYWidth)/2;

	for (int i = pUpdates->m_iLowestX; i <= pUpdates->m_iHighestX; i++)
	{
		for (int j = pUpdates->m_iLowestY; j <= pUpdates->m_iHighestY; j++)
		{
			if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_EMPTY)
				continue;

			m_apUpdates.push_back(new CUpdateButton(this));
			CUpdateButton* pUpdate = m_apUpdates[m_apUpdates.size()-1];
			pUpdate->SetSize(m_iButtonSize-2, m_iButtonSize-1);
			pUpdate->SetPos((i-iLowestX)*m_iButtonSize + iXBuffer, (j-iLowestY)*m_iButtonSize + iYBuffer);
			pUpdate->SetFontFaceSize(10);
			pUpdate->SetLocation(i, j);
			AddControl(pUpdate);

			bool bCanDownload = pCurrentGame->CanDownloadUpdate(i, j);
			bool bAlreadyDownloaded = pCurrentGame->HasDownloadedUpdate(i, j);

			if (bAlreadyDownloaded)
			{
				pUpdate->SetEnabled(false);
				pUpdate->SetFGColor(Color(0, 0, 0));
			}
			else
			{
				pUpdate->SetFGColor(Color(150, 150, 150));
				if (bCanDownload)
					pUpdate->SetClickedListener(pUpdate, &CUpdateButton::ChooseDownload);
				else
					pUpdate->SetEnabled(false);
			}

			pUpdate->SetTexture(GetTextureForUpdateItem(&pUpdates->m_aUpdates[i][j]));
			pUpdate->SetText(pUpdates->m_aUpdates[i][j].GetName().c_str());

			if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_STRUCTURE)
			{
				Color clrButton = Color(50, 50, 255);
				if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_TANKLOADER)
					clrButton = Color(255, 255, 50);
				else if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_ARTILLERYLOADER)
					clrButton = Color(255, 255, 50);

				if (bAlreadyDownloaded)
					pUpdate->SetButtonColor(Color(clrButton.r()/2, clrButton.g()/2, clrButton.b()/2));
				else if (!bCanDownload)
				{
					pUpdate->SetButtonColor(Color(clrButton.r()/10, clrButton.g()/10, clrButton.b()/10));
					pUpdate->SetAlpha(200/6);
				}
				else
				{
					pUpdate->SetButtonColor(Color(clrButton.r()/3, clrButton.g()/3, clrButton.b()/3));
					pUpdate->SetAlpha(200*2/3);
				}
			}
			else if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_STRUCTUREUPDATE)
			{
				Color clrButton = Color(200, 200, 200);
				if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_CPU)
					clrButton = Color(200, 200, 200);
				else if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_INFANTRYLOADER)
					clrButton = Color(200, 200, 150);
				else if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_TANKLOADER)
					clrButton = Color(200, 200, 150);
				else if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_ARTILLERYLOADER)
					clrButton = Color(200, 200, 150);
				else if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_BUFFER)
					clrButton = Color(150, 150, 200);

				if (bAlreadyDownloaded)
					pUpdate->SetButtonColor(Color(clrButton.r()/2, clrButton.g()/2, clrButton.b()/2));
				else if (!bCanDownload)
				{
					pUpdate->SetButtonColor(Color(clrButton.r()/10, clrButton.g()/10, clrButton.b()/10));
					pUpdate->SetAlpha(200/6);
				}
				else
				{
					pUpdate->SetButtonColor(Color(clrButton.r()/3, clrButton.g()/3, clrButton.b()/3));
					pUpdate->SetAlpha(200*2/3);
				}
			}
			else if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_UNITSKILL)
			{
				Color clrButton = Color(200, 0, 0);

				if (bAlreadyDownloaded)
					pUpdate->SetButtonColor(Color(clrButton.r()/2, clrButton.g()/2, clrButton.b()/2));
				else if (!bCanDownload)
				{
					pUpdate->SetButtonColor(Color(clrButton.r()/10, clrButton.g()/10, clrButton.b()/10));
					pUpdate->SetAlpha(200/6);
				}
				else
				{
					pUpdate->SetButtonColor(Color(clrButton.r()/3, clrButton.g()/3, clrButton.b()/3));
					pUpdate->SetAlpha(200*2/3);
				}
			}

			if (pCurrentGame->IsDownloading(i, j))
			{
				if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_STRUCTURE)
				{
					pUpdate->SetButtonColor(Color(250, 150, 150));
					pUpdate->SetFGColor(Color(50, 50, 50));
				}
				else if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_STRUCTUREUPDATE)
				{
					pUpdate->SetButtonColor(Color(250, 250, 250));
					pUpdate->SetFGColor(Color(50, 50, 50));
				}
			}
		}
	}

	UpdateInfo(NULL);
}

void CUpdatesPanel::Paint(int x, int y, int w, int h)
{
	CRootPanel::PaintRect(x, y, w, h, Color(0, 0, 0, GetAlpha()));

	int ix, iy, iw, ih;
	m_pInfo->GetAbsDimensions(ix, iy, iw, ih);
	CRootPanel::PaintRect(ix, iy, iw, ih, Color(0, 0, 0, GetAlpha()));

	CPanel::Paint(x, y, w, h);
}

void CUpdatesPanel::CloseCallback()
{
	SetVisible(false);
}

void CUpdatesPanel::UpdateInfo(CUpdateItem* pInfo)
{
	if (!pInfo)
	{
		m_pTutorial->SetText("");
		return;
	}

	CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();

	int x, y;
	DigitanksGame()->GetUpdateGrid()->FindUpdate(pInfo, x, y);

	eastl::string16 s;
	eastl::string16 p;
	s += pInfo->GetName() + L"\n \n";
	s += pInfo->GetInfo() + L"\n \n";

	if (pInfo->m_eUpdateClass == UPDATECLASS_STRUCTUREUPDATE)
		s += p.sprintf(L"Increase: %.1f %s\n", pInfo->m_flValue, pInfo->GetUnits());

	s += p.sprintf(L"Download size: %d\n", (int)pInfo->m_flSize);

	if (pTeam && pTeam->GetBandwidth() > 0 && !pTeam->HasDownloadedUpdate(x, y))
	{
		float flDownloaded = pTeam->GetMegabytes();
		int iTurns = (int)((pInfo->m_flSize-flDownloaded)/pTeam->GetBandwidth())+1;

		if (iTurns < 1)
			iTurns = 1;

		s += p.sprintf(L"Turns to download: %d\n", iTurns);
	}

	m_pInfo->SetText(s);

	m_pInfo->SetSize(m_pInfo->GetWidth(), 9999);
	m_pInfo->SetSize(m_pInfo->GetWidth(), (int)m_pInfo->GetTextHeight()+20);

	if (pTeam->HasDownloadedUpdate(x, y))
		m_pTutorial->SetText(L"You already have this update.");
	else if (pTeam->CanDownloadUpdate(x, y))
		m_pTutorial->SetText(L"Click to begin downloading this update.");
	else
		m_pTutorial->SetText(L"This update is not yet available for download.");
}

size_t CUpdatesPanel::GetTextureForUpdateItem(class CUpdateItem* pInfo)
{
	if (pInfo->m_eUpdateClass == UPDATECLASS_STRUCTURE)
	{
		switch (pInfo->m_eStructure)
		{
		case STRUCTURE_CPU:
			return m_iIconCPU;

		case STRUCTURE_BUFFER:
			return m_iIconBuffer;

		case STRUCTURE_PSU:
			return m_iIconPSU;

		case STRUCTURE_INFANTRYLOADER:
			return m_iIconInfantryLoader;

		case STRUCTURE_TANKLOADER:
			return m_iIconTankLoader;

		case STRUCTURE_ARTILLERYLOADER:
			return m_iIconArtilleryLoader;

		default:
			return 0;
		}
	}
	else if (pInfo->m_eUpdateClass == UPDATECLASS_STRUCTUREUPDATE)
	{
		switch (pInfo->m_eUpdateType)
		{
		case UPDATETYPE_PRODUCTION:
			return m_iIconCPUPower;

		case UPDATETYPE_BANDWIDTH:
			if (pInfo->m_eStructure == STRUCTURE_CPU)
				return m_iIconCPUBandwidth;
			else
				return m_iIconBufferBandwidth;

		case UPDATETYPE_FLEETSUPPLY:
			if (pInfo->m_eStructure == STRUCTURE_CPU)
				return m_iIconCPUFleet;
			else
				return m_iIconBufferFleet;

		case UPDATETYPE_SUPPORTENERGY:
			return m_iIconBufferEnergy;

		case UPDATETYPE_SUPPORTRECHARGE:
			return m_iIconBufferRecharge;

		case UPDATETYPE_TANKATTACK:
			if (pInfo->m_eStructure == STRUCTURE_INFANTRYLOADER)
				return m_iIconInfantryAttack;
			else if (pInfo->m_eStructure == STRUCTURE_TANKLOADER)
				return m_iIconTankAttack;
			else
				return m_iIconArtilleryAttack;

		case UPDATETYPE_TANKDEFENSE:
			if (pInfo->m_eStructure == STRUCTURE_INFANTRYLOADER)
				return m_iIconInfantryDefense;
			else
				return m_iIconTankDefense;

		case UPDATETYPE_TANKMOVEMENT:
			if (pInfo->m_eStructure == STRUCTURE_INFANTRYLOADER)
				return m_iIconInfantryMovement;
			else if (pInfo->m_eStructure == STRUCTURE_TANKLOADER)
				return m_iIconTankMovement;
			else
				return m_iIconArtilleryMovement;

		case UPDATETYPE_TANKHEALTH:
			if (pInfo->m_eStructure == STRUCTURE_INFANTRYLOADER)
				return m_iIconInfantryHealth;
			else if (pInfo->m_eStructure == STRUCTURE_TANKLOADER)
				return m_iIconTankHealth;
			else
				return m_iIconArtilleryHealth;

		case UPDATETYPE_TANKRANGE:
			return m_iIconArtilleryRange;
		}
	}

	return 0;
}

CUpdateButton::CUpdateButton(CUpdatesPanel* pPanel)
	: CPictureButton(L"")
{
	m_pUpdatesPanel = pPanel;
	m_iX = m_iY = 0;
}

void CUpdateButton::SetLocation(int x, int y)
{
	m_iX = x;
	m_iY = y;
}

void CUpdateButton::CursorIn()
{
	CPictureButton::CursorIn();

	CUpdateGrid* pUpdates = DigitanksGame()->GetUpdateGrid();
	if (!pUpdates)
		return;

	m_pUpdatesPanel->UpdateInfo(&pUpdates->m_aUpdates[m_iX][m_iY]);
}

void CUpdateButton::CursorOut()
{
	CPictureButton::CursorOut();

	m_pUpdatesPanel->UpdateInfo(NULL);
}

void CUpdateButton::ChooseDownloadCallback()
{
	CUpdateGrid* pUpdates = DigitanksGame()->GetUpdateGrid();
	if (!pUpdates)
		return;

	DigitanksGame()->GetCurrentLocalDigitanksTeam()->DownloadUpdate(m_iX, m_iY);

	m_pUpdatesPanel->SetVisible(false);

	int x, y;
	GetAbsPos(x, y);
	DigitanksWindow()->GetHUD()->SlideUpdateIcon(x, y);

	CRootPanel::Get()->Layout();
}

