#include "updatespanel.h"

#include <game/digitanks/digitanksgame.h>
#include <renderer/renderer.h>

using namespace glgui;

CUpdatesPanel::CUpdatesPanel()
	: CPanel(0, 0, 600, 600)
{
	m_pCloseButton = new CButton(0, 0, 100, 20, L"Close");
	m_pCloseButton->SetClickedListener(this, Close);
	AddControl(m_pCloseButton);

	m_pInfo = new CLabel(0, 0, 100, 300, L"");
	AddControl(m_pInfo);

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

	int iButtonSize = GetWidth()/(iLarger+1);
	int iXWidth = iButtonSize*iXSize;
	int iYWidth = iButtonSize*iYSize;
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
			pUpdate->SetSize(iButtonSize-2, iButtonSize-1);
			pUpdate->SetPos((i-iLowestX)*iButtonSize + iXBuffer, (j-iLowestY)*iButtonSize + iYBuffer);
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
				if (bAlreadyDownloaded)
					pUpdate->SetButtonColor(Color(250/2, 100/2, 100/2));
				else if (!bCanDownload)
				{
					pUpdate->SetButtonColor(Color(250/10, 100/10, 100/10));
					pUpdate->SetAlpha(200/6);
				}
				else
				{
					pUpdate->SetButtonColor(Color(250/3, 100/3, 100/3));
					pUpdate->SetAlpha(200*2/3);
				}
			}
			else if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_STRUCTUREUPDATE)
			{
				if (bAlreadyDownloaded)
					pUpdate->SetButtonColor(Color(200/2, 200/2, 200/2));
				else if (!bCanDownload)
				{
					pUpdate->SetButtonColor(Color(200/10, 200/10, 200/10));
					pUpdate->SetAlpha(200/6);
				}
				else
				{
					pUpdate->SetButtonColor(Color(200/3, 200/3, 200/3));
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
		m_pInfo->SetText("");
		return;
	}

	eastl::string16 s;
	eastl::string16 p;
	s += pInfo->GetName() + L"\n \n";
	s += pInfo->GetInfo() + L"\n \n";

	if (pInfo->m_eUpdateClass != UPDATECLASS_STRUCTURE)
		s += p.sprintf(L"Increase: %f\n", pInfo->m_flValue);

	s += p.sprintf(L"Download size: %d\n", pInfo->m_iSize);

	if (pInfo->m_eUpdateClass != UPDATECLASS_STRUCTURE)
		s += p.sprintf(L"Power to install: %d\n", pInfo->m_iProductionToInstall);

	m_pInfo->SetText(s);

	m_pInfo->SetSize(m_pInfo->GetWidth(), 9999);
	m_pInfo->SetSize(m_pInfo->GetWidth(), (int)m_pInfo->GetTextHeight()+20);
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

void CUpdateButton::ChooseDownloadCallback()
{
	CUpdateGrid* pUpdates = DigitanksGame()->GetUpdateGrid();
	if (!pUpdates)
		return;

	DigitanksGame()->GetCurrentLocalDigitanksTeam()->DownloadUpdate(m_iX, m_iY);

	m_pUpdatesPanel->SetVisible(false);

	CRootPanel::Get()->Layout();
}

