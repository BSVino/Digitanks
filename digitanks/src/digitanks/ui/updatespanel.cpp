#include "updatespanel.h"

#include <strutils.h>

#include <renderer/renderer.h>
#include <models/texturelibrary.h>

#include <digitanksgame.h>
#include <ui/digitankswindow.h>
#include <ui/hud.h>

using namespace glgui;

CUpdatesPanel::CUpdatesPanel()
	: CPanel(0, 0, 600, 600)
{
	m_pCloseButton = new CButton(0, 0, 100, 20, _T("Close"));
	m_pCloseButton->SetClickedListener(this, Close);
	m_pCloseButton->SetFont(_T("header"));
	AddControl(m_pCloseButton);

	m_pAvailable = new CLabel(0, 0, 100, 300, _T(""));
	m_pAvailable->SetFont(_T("text"), 18);
	AddControl(m_pAvailable);

	m_pInfo = new CLabel(0, 0, 100, 300, _T(""));
	m_pInfo->SetFont(_T("text"));
	AddControl(m_pInfo);

	m_pTutorial = new CLabel(0, 0, 100, 300, _T(""));
	m_pTutorial->SetFont(_T("text"));
	AddControl(m_pTutorial);
}

void CUpdatesPanel::Layout()
{
	CPanel::Layout();

	SetPos(CRootPanel::Get()->GetWidth()/2-GetWidth()/2, CRootPanel::Get()->GetHeight()/2-GetHeight()/2);

	m_pCloseButton->SetPos(GetWidth()-m_pCloseButton->GetWidth()-20, 20);

	m_pAvailable->SetPos(20, 20);
	m_pAvailable->SetSize(GetWidth(), 20);
	m_pAvailable->SetAlign(glgui::CLabel::TA_LEFTCENTER);

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
			pUpdate->SetFont(_T("text"), 10);
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

			size_t iSheet;
			int sx, sy, sw, sh, tw, th;
			GetTextureForUpdateItem(&pUpdates->m_aUpdates[i][j], iSheet, sx, sy, sw, sh, tw, th);
			pUpdate->SetSheetTexture(iSheet, sx, sy, sw, sh, tw, th);
			pUpdate->SetText(pUpdates->m_aUpdates[i][j].GetName().c_str());

			if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_STRUCTURE)
			{
				Color clrButton = Color(10, 10, 200);
				if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_TANKLOADER)
					clrButton = Color(200, 200, 10);
				else if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_ARTILLERYLOADER)
					clrButton = Color(200, 200, 10);

				if (bAlreadyDownloaded)
				{
					pUpdate->SetButtonColor(clrButton/2);
					pUpdate->SetEnabled(false);
				}
				else if (!bCanDownload)
				{
					pUpdate->SetButtonColor((Vector(clrButton)*0.1f + Vector(0.2f, 0.2f, 0.2f)*0.9f));
					pUpdate->SetAlpha(0.2f);
				}
				else
				{
					pUpdate->SetButtonColor((Vector(clrButton) + Vector(0.3f, 0.3f, 0.3f))/2);
					pUpdate->SetAlpha(200);
				}
			}
			else if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_STRUCTUREUPDATE)
			{
				Color clrButton = Color(10, 10, 10);
				if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_CPU)
					clrButton = Color(10, 10, 10);
				else if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_INFANTRYLOADER)
					clrButton = Color(150, 150, 10);
				else if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_TANKLOADER)
					clrButton = Color(150, 150, 10);
				else if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_ARTILLERYLOADER)
					clrButton = Color(150, 150, 10);
				else if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_BUFFER)
					clrButton = Color(10, 10, 150);

				if (bAlreadyDownloaded)
				{
					pUpdate->SetButtonColor(clrButton/2);
					pUpdate->SetEnabled(false);
				}
				else if (!bCanDownload)
				{
					pUpdate->SetButtonColor((Vector(clrButton)*0.1f + Vector(0.2f, 0.2f, 0.2f)*0.9f));
					pUpdate->SetAlpha(0.2f);
				}
				else
				{
					pUpdate->SetButtonColor((Vector(clrButton) + Vector(0.3f, 0.3f, 0.3f))/2);
					pUpdate->SetAlpha(200);
				}
			}
			else if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_UNITSKILL)
			{
				Color clrButton = Color(200, 0, 0);

				if (bAlreadyDownloaded)
				{
					pUpdate->SetButtonColor(clrButton/2);
					pUpdate->SetEnabled(false);
				}
				else if (!bCanDownload)
				{
					pUpdate->SetButtonColor((Vector(clrButton)*0.1f + Vector(0.2f, 0.2f, 0.2f)*0.9f));
					pUpdate->SetAlpha(0.2f);
				}
				else
				{
					pUpdate->SetButtonColor((Vector(clrButton) + Vector(0.3f, 0.3f, 0.3f))/2);
					pUpdate->SetAlpha(200);
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

	for (size_t i = 0; i < m_apUpdates.size(); i++)
	{
		CUpdateButton* pButton = m_apUpdates[i];
		if (!pButton)
			continue;

		if (pButton->m_flFocusRamp > 0)
		{
			int x, y, w, h;
			pButton->GetAbsDimensions(x, y, w, h);
			pButton->Paint(x, y, w, h);
		}
	}
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

	m_pAvailable->SetText(sprintf(tstring("Available: %dMB"), (int)pTeam->GetMegabytes()));

	int x, y;
	DigitanksGame()->GetUpdateGrid()->FindUpdate(pInfo, x, y);

	tstring s;
	tstring p;
	s += pInfo->GetName() + _T("\n \n");

	if (pInfo->m_eUpdateClass == UPDATECLASS_STRUCTUREUPDATE)
		s += sprintf(tstring("Increase: %.1f %s\n"), pInfo->m_flValue, pInfo->GetUnits().c_str());

	s += sprintf(tstring("Download cost: %dMB\n"), (int)pInfo->m_flSize);

	if (pTeam && pTeam->GetBandwidth() > 0 && !pTeam->HasDownloadedUpdate(x, y))
	{
		float flDownloaded = pTeam->GetMegabytes();
		int iTurns = (int)((pInfo->m_flSize-flDownloaded)/pTeam->GetBandwidth())+1;

		if (iTurns < 1)
			iTurns = 1;

		s += sprintf(tstring("Turns to download: %d\n"), iTurns);
	}

	s += _T(" \n");
	s += pInfo->GetInfo();

	m_pInfo->SetText(s);

	m_pInfo->SetSize(m_pInfo->GetWidth(), 9999);
	m_pInfo->SetSize(m_pInfo->GetWidth(), (int)m_pInfo->GetTextHeight()+20);

	if (pTeam->HasDownloadedUpdate(x, y))
		m_pTutorial->SetText(_T("You already have this update."));
	else if (pTeam->CanDownloadUpdate(x, y))
	{
		float flDownloaded = pTeam->GetMegabytes();
		int iTurns = (int)((pInfo->m_flSize-flDownloaded)/pTeam->GetBandwidth())+1;

		if (pInfo->m_flSize <= flDownloaded)
			iTurns = 0;

		if (iTurns < 0)
			iTurns = 0;

		if (iTurns == 0)
			m_pTutorial->SetText(sprintf(tstring("This update costs %dMB to download."), (int)pInfo->m_flSize));
		else if (iTurns == 1)
			m_pTutorial->SetText(sprintf(tstring("This update costs %dMB to download. This update would take 1 turn to download."), (int)pInfo->m_flSize, iTurns));
		else
			m_pTutorial->SetText(sprintf(tstring("This update costs %dMB to download. This update would take %d turns to download."), (int)pInfo->m_flSize, iTurns));
	}
	else
		m_pTutorial->SetText(_T("This update is not yet available for download."));
}

void CUpdatesPanel::GetTextureForUpdateItem(class CUpdateItem* pInfo, size_t& iSheet, int& sx, int& sy, int& sw, int& sh, int& tw, int& th)
{
	if (!pInfo)
	{
		iSheet = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheet("DownloadButton");
		Rect rArea = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetArea("DownloadButton");
		sx = rArea.x;
		sy = rArea.y;
		sw = rArea.w;
		sh = rArea.h;
		tw = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetWidth("DownloadButton");
		th = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetHeight("DownloadButton");
		return;
	}

	int iMenuWidth = 512;
	int iMenuHeight = 256;

	if (pInfo->m_eUpdateClass == UPDATECLASS_STRUCTURE)
	{
		Rect rArea;
		switch (pInfo->m_eStructure)
		{
		case STRUCTURE_CPU:
			rArea = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetArea("CPU");
			iSheet = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheet("CPU");
			tw = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetWidth("CPU");
			th = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetHeight("CPU");
			break;

		case STRUCTURE_BUFFER:
			rArea = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetArea("MacroBuffer");
			iSheet = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheet("MacroBuffer");
			tw = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetWidth("MacroBuffer");
			th = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetHeight("MacroBuffer");
			break;

		case STRUCTURE_PSU:
			rArea = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetArea("PSU");
			iSheet = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheet("PSU");
			tw = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetWidth("PSU");
			th = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetHeight("PSU");
			break;

		case STRUCTURE_INFANTRYLOADER:
			rArea = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetArea("ResistorFactory");
			iSheet = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheet("ResistorFactory");
			tw = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetWidth("ResistorFactory");
			th = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetHeight("ResistorFactory");
			break;

		case STRUCTURE_TANKLOADER:
			rArea = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetArea("DigitankFactory");
			iSheet = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheet("DigitankFactory");
			tw = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetWidth("DigitankFactory");
			th = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetHeight("DigitankFactory");
			break;

		case STRUCTURE_ARTILLERYLOADER:
			rArea = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetArea("ArtilleryFactory");
			iSheet = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheet("ArtilleryFactory");
			tw = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetWidth("ArtilleryFactory");
			th = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetHeight("ArtilleryFactory");
			break;

		default:
			break;
		}

		sx = rArea.x;
		sy = rArea.y;
		sw = rArea.w;
		sh = rArea.h;
		return;
	}
	else if (pInfo->m_eUpdateClass == UPDATECLASS_STRUCTUREUPDATE)
	{
		eastl::string sArea;
		const CTextureSheet* pSheet;
		switch (pInfo->m_eUpdateType)
		{
		case UPDATETYPE_PRODUCTION:
			sArea = "CPUPower";
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			break;

		case UPDATETYPE_BANDWIDTH:
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			if (pInfo->m_eStructure == STRUCTURE_CPU)
				sArea = "CPUBandwidth";
			else
				sArea = "MacroBufferBandwidth";
			break;

		case UPDATETYPE_FLEETSUPPLY:
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			if (pInfo->m_eStructure == STRUCTURE_CPU)
				sArea = "CPUFleet";
			else
				sArea = "MacroBufferFleet";
			break;

		case UPDATETYPE_SUPPORTENERGY:
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			sArea = "MacroBufferEnergy";
			break;

		case UPDATETYPE_SUPPORTRECHARGE:
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			sArea = "MacroBufferRecharge";
			break;

		case UPDATETYPE_TANKATTACK:
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			if (pInfo->m_eStructure == STRUCTURE_INFANTRYLOADER)
				sArea = "ResistorAttack";
			else if (pInfo->m_eStructure == STRUCTURE_TANKLOADER)
				sArea = "DigitankAttack";
			else
				sArea = "ArtilleryAttack";
			break;

		case UPDATETYPE_TANKDEFENSE:
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			if (pInfo->m_eStructure == STRUCTURE_INFANTRYLOADER)
				sArea = "ResistorShields";
			else
				sArea = "DigitankShields";
			break;

		case UPDATETYPE_TANKMOVEMENT:
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			if (pInfo->m_eStructure == STRUCTURE_INFANTRYLOADER)
				sArea = "ResistorMovement";
			else if (pInfo->m_eStructure == STRUCTURE_TANKLOADER)
				sArea = "DigitankMovement";
			else
				sArea = "ArtilleryMovement";
			break;

		case UPDATETYPE_TANKHEALTH:
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			if (pInfo->m_eStructure == STRUCTURE_INFANTRYLOADER)
				sArea = "ResistorHealth";
			else if (pInfo->m_eStructure == STRUCTURE_TANKLOADER)
				sArea = "DigitankHealth";
			else
				sArea = "ArtilleryHealth";
			break;

		case UPDATETYPE_TANKRANGE:
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			sArea = "ArtilleryRange";
			break;
		}

		iSheet = pSheet->GetSheet(sArea);
		Rect rArea = pSheet->GetArea(sArea);
		sx = rArea.x;
		sy = rArea.y;
		sw = rArea.w;
		sh = rArea.h;
		tw = pSheet->GetSheetWidth(sArea);
		th = pSheet->GetSheetHeight(sArea);
		return;
	}
	else if (pInfo->m_eUpdateClass == UPDATECLASS_UNITSKILL)
	{
		eastl::string sArea;
		const CTextureSheet* pSheet;

		switch (pInfo->m_eUpdateType)
		{
		case UPDATETYPE_SKILL_CLOAK:
			sArea = "Stealth";
			pSheet = &DigitanksWindow()->GetHUD()->GetButtonSheet();
			break;

		case UPDATETYPE_WEAPON_CHARGERAM:
			sArea = "ChargingRAM";
			pSheet = &DigitanksWindow()->GetHUD()->GetWeaponSheet();
			break;

		case UPDATETYPE_WEAPON_AOE:
			sArea = "AOE";
			pSheet = &DigitanksWindow()->GetHUD()->GetWeaponSheet();
			break;

		case UPDATETYPE_WEAPON_CLUSTER:
			sArea = "ClusterBomb";
			pSheet = &DigitanksWindow()->GetHUD()->GetWeaponSheet();
			break;

		case UPDATETYPE_WEAPON_ICBM:
			sArea = "ICBM";
			pSheet = &DigitanksWindow()->GetHUD()->GetWeaponSheet();
			break;

		case UPDATETYPE_WEAPON_DEVASTATOR:
			sArea = "Devastator";
			pSheet = &DigitanksWindow()->GetHUD()->GetWeaponSheet();
			break;
		}

		iSheet = pSheet->GetSheet(sArea);
		Rect rArea = pSheet->GetArea(sArea);
		sx = rArea.x;
		sy = rArea.y;
		sw = rArea.w;
		sh = rArea.h;
		tw = pSheet->GetSheetWidth(sArea);
		th = pSheet->GetSheetHeight(sArea);
		return;
	}

	return;
}

CUpdateButton::CUpdateButton(CUpdatesPanel* pPanel)
	: CPictureButton(_T(""))
{
	m_pUpdatesPanel = pPanel;
	m_iX = m_iY = 0;
	m_flFocusRamp = m_flFocusRampGoal = 0;
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

	if (IsEnabled())
		m_flFocusRampGoal = 1;
}

void CUpdateButton::CursorOut()
{
	CPictureButton::CursorOut();

	m_pUpdatesPanel->UpdateInfo(NULL);

	m_flFocusRampGoal = 0;
}

void CUpdateButton::Think()
{
	m_flFocusRamp = Approach(m_flFocusRampGoal, m_flFocusRamp, GameServer()->GetFrameTime()*3);

	CPictureButton::Think();
}

void CUpdateButton::Paint(int x, int y, int w, int h)
{
	float flScale = RemapVal(m_flFocusRamp, 0, 1, 1, 1.5f);
	CPictureButton::Paint(x + w/2 - (int)(flScale*w)/2, y + h/2 - (int)(flScale*h)/2, (int)(w*flScale), (int)(h*flScale));
}

void CUpdateButton::ChooseDownloadCallback()
{
	CUpdateGrid* pUpdates = DigitanksGame()->GetUpdateGrid();
	if (!pUpdates)
		return;

	DigitanksGame()->GetCurrentLocalDigitanksTeam()->DownloadUpdate(m_iX, m_iY);

	if (DigitanksGame()->GetCurrentLocalDigitanksTeam()->IsDownloading(m_iX, m_iY))
	{
		int x, y;
		GetAbsPos(x, y);
		DigitanksWindow()->GetHUD()->SlideUpdateIcon(x, y);

		GetParent()->SetVisible(false);
	}

	// Do this very last thing because since it calls Layout() this button will be deleted.
	DigitanksGame()->GetCurrentLocalDigitanksTeam()->HandledActionItem(ACTIONTYPE_DOWNLOADCOMPLETE);
	DigitanksGame()->GetCurrentLocalDigitanksTeam()->HandledActionItem(ACTIONTYPE_DOWNLOADUPDATES);

	CRootPanel::Get()->Layout();
}

