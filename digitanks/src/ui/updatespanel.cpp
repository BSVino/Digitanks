#include "updatespanel.h"

#include <sstream>

#include <game/digitanks/digitanksgame.h>

using namespace glgui;

CUpdatesPanel::CUpdatesPanel()
	: CPanel(0, 0, 600, 600)
{
	m_pCloseButton = new CButton(0, 0, 100, 20, "Close");
	m_pCloseButton->SetClickedListener(this, Close);
	AddControl(m_pCloseButton);

	m_pInfo = new CLabel(0, 0, 100, 300, "");
	AddControl(m_pInfo);
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

	if (!DigitanksGame()->GetUpdateGrid())
		return;

	CUpdateGrid* pUpdates = DigitanksGame()->GetUpdateGrid();

	int iLowestX = pUpdates->m_iLowestX;
	int iLowestY = pUpdates->m_iLowestY;

	int iXSize = pUpdates->m_iHighestX - iLowestX;
	int iYSize = pUpdates->m_iHighestY - iLowestY;

	int iLarger = (iXSize > iYSize) ? iXSize : iYSize;

	if (iLarger < 11)
	{
		iLarger = 11;
		iLowestX = (pUpdates->m_iHighestX + pUpdates->m_iLowestX)/2 - iLarger/2;
		iLowestY = (pUpdates->m_iHighestY + pUpdates->m_iLowestY)/2 - iLarger/2;
	}

	int iButtonSize = GetWidth()/iLarger;

	for (int i = pUpdates->m_iLowestX; i <= pUpdates->m_iHighestX; i++)
	{
		for (int j = pUpdates->m_iLowestY; j <= pUpdates->m_iHighestY; j++)
		{
			if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_EMPTY)
				continue;

			m_apUpdates.push_back(new CUpdateButton(this));
			CUpdateButton* pUpdate = m_apUpdates[m_apUpdates.size()-1];
			pUpdate->SetSize(iButtonSize-2, iButtonSize-1);
			pUpdate->SetPos((i-iLowestX)*iButtonSize, (j-iLowestY)*iButtonSize);
			pUpdate->SetFontFaceSize(10);
			pUpdate->SetLocation(i, j);
			AddControl(pUpdate);

			bool bCanDownload = DigitanksGame()->GetLocalDigitanksTeam()->CanDownloadUpdate(i, j);
			bool bAlreadyDownloaded = DigitanksGame()->GetLocalDigitanksTeam()->HasDownloadedUpdate(i, j);

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

			pUpdate->SetText(pUpdates->m_aUpdates[i][j].GetName().c_str());

			if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_STRUCTURE)
			{
				if (bAlreadyDownloaded)
					pUpdate->SetButtonColor(Color(250, 100, 100));
				else if (!bCanDownload)
					pUpdate->SetButtonColor(Color(250/6, 100/6, 100/6));
				else
					pUpdate->SetButtonColor(Color(250/2, 100/2, 100/2));
			}
			else if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_STRUCTUREUPDATE)
			{
				if (bAlreadyDownloaded)
					pUpdate->SetButtonColor(Color(200, 200, 200));
				else if (!bCanDownload)
					pUpdate->SetButtonColor(Color(200/6, 200/6, 200/6));
				else
					pUpdate->SetButtonColor(Color(200/2, 200/2, 200/2));
			}

			if (DigitanksGame()->GetLocalDigitanksTeam()->IsDownloading(i, j))
			{
				pUpdate->SetButtonColor(Color(250, 250, 250));
				pUpdate->SetFGColor(Color(50, 50, 50));
			}
		}
	}
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

	std::stringstream s;
	s << pInfo->GetName() << "\n \n";
	s << "Increase: " << pInfo->m_flValue << "\n";
	s << "Download size: " << pInfo->m_iSize << "\n";
	s << "Power to install: " << pInfo->m_iProductionToInstall << "\n";

	m_pInfo->SetText(s.str().c_str());
}

CUpdateButton::CUpdateButton(CUpdatesPanel* pPanel)
	: CPictureButton("")
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

	DigitanksGame()->GetLocalDigitanksTeam()->DownloadUpdate(m_iX, m_iY);

	m_pUpdatesPanel->SetVisible(false);

	CRootPanel::Get()->Layout();
}
