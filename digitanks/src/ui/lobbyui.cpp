#include "lobbyui.h"

#include <tinker/cvar.h>

#include "menu.h"
#include "digitankswindow.h"
#include "chatbox.h"

CVar lobby_gametype("lobby_gametype", "");

CLobbyPanel::CLobbyPanel()
	: CPanel(0, 0, 100, 100)
{
	SetVisible(false);

	m_pDockPanel = new CDockPanel();
	m_pDockPanel->SetBGColor(Color(12, 13, 12, 0));
	AddControl(m_pDockPanel);

	m_pChatBox = new CChatBox(false);
	AddControl(m_pChatBox);

	m_pLeave = new glgui::CButton(0, 0, 100, 100, L"Leave Lobby");
	m_pLeave->SetClickedListener(this, LeaveLobby);
	AddControl(m_pLeave);

	m_pReady = new glgui::CButton(0, 0, 100, 100, L"Ready");
	AddControl(m_pReady);
}

void CLobbyPanel::Layout()
{
	size_t iWidth = DigitanksWindow()->GetWindowWidth();
	size_t iHeight = DigitanksWindow()->GetWindowHeight();

	SetSize(924, 668);
	SetPos(iWidth/2-GetWidth()/2, iHeight/2-GetHeight()/2);

	m_pDockPanel->SetSize(375, 480);
	m_pDockPanel->SetPos(20, 20);

	m_pChatBox->SetSize(565, 120); 
	m_pChatBox->SetPos(20, 520);

	m_pLeave->SetSize(100, 35);
	m_pLeave->SetPos(925 - 320, 668 - 55);

	m_pReady->SetSize(180, 35);
	m_pReady->SetPos(925 - 200, 668 - 55);
}

void CLobbyPanel::Paint(int x, int y, int w, int h)
{
	glgui::CRootPanel::PaintRect(x, y, w, h, Color(12, 13, 12, 255));

	BaseClass::Paint(x, y, w, h);
}

void CLobbyPanel::CreateLobby()
{
	CNetwork::Disconnect();

	const char* pszPort = DigitanksWindow()->GetCommandLineSwitchValue("--port");
	int iPort = pszPort?atoi(pszPort):0;

	CNetwork::CreateHost(iPort);

	if ((gametype_t)lobby_gametype.GetInt() == GAMETYPE_ARTILLERY)
		m_pDockPanel->SetDockedPanel(new CArtilleryGamePanel(true));
	else
		m_pDockPanel->SetDockedPanel(new CStrategyGamePanel(true));

	SetVisible(true);
	DigitanksWindow()->GetMainMenu()->SetVisible(false);
}

void CLobbyPanel::LeaveLobbyCallback()
{
	CNetwork::Disconnect();

	SetVisible(false);
	DigitanksWindow()->GetMainMenu()->SetVisible(true);
}

