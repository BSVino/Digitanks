#include "lobbyui.h"

#include <tinker/cvar.h>
#include <tinker/lobby/lobby_server.h>
#include <tinker/lobby/lobby_client.h>

#include <renderer/renderer.h>

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

	for (size_t i = 0; i < m_apPlayerPanels.size(); i++)
	{
		m_apPlayerPanels[i]->Destructor();
		m_apPlayerPanels[i]->Delete();
	}

	m_apPlayerPanels.clear();

	for (size_t i = 0; i < CGameLobbyClient::GetNumPlayers(); i++)
	{
		m_apPlayerPanels.push_back(new CPlayerPanel());
		CPlayerPanel* pPanel = m_apPlayerPanels[m_apPlayerPanels.size()-1];
		AddControl(pPanel);
		pPanel->SetPlayer(CGameLobbyClient::GetPlayer(i)->iClient);
	}
}

void CLobbyPanel::Paint(int x, int y, int w, int h)
{
	glgui::CRootPanel::PaintRect(x, y, w, h, Color(12, 13, 12, 255));

	BaseClass::Paint(x, y, w, h);
}

void CLobbyPanel::CreateLobby()
{
	CGameLobbyClient::SetLobbyUpdateCallback(this, &LobbyUpdateCallback);

	const char* pszPort = DigitanksWindow()->GetCommandLineSwitchValue("--port");
	int iPort = pszPort?atoi(pszPort):0;

	m_iLobby = CGameLobbyServer::CreateLobby(iPort);

	if ((gametype_t)lobby_gametype.GetInt() == GAMETYPE_ARTILLERY)
		m_pDockPanel->SetDockedPanel(new CArtilleryGamePanel(true));
	else
		m_pDockPanel->SetDockedPanel(new CStrategyGamePanel(true));

	SetVisible(true);
	DigitanksWindow()->GetMainMenu()->SetVisible(false);

	CGameLobbyClient::JoinLobby(m_iLobby);
	UpdatePlayerInfo();
}

void CLobbyPanel::ConnectToLocalLobby(const eastl::string16& sHost)
{
	CGameLobbyClient::SetLobbyUpdateCallback(this, &LobbyUpdateCallback);

	const char* pszPort = DigitanksWindow()->GetCommandLineSwitchValue("--port");
	int iPort = pszPort?atoi(pszPort):0;

	CNetwork::ConnectToHost(convertstring<char16_t, char>(sHost).c_str(), iPort);
	if (!CNetwork::IsConnected())
		return;

	m_pDockPanel->SetDockedPanel(new CInfoPanel());

	SetVisible(true);
	DigitanksWindow()->GetMainMenu()->SetVisible(false);

	UpdatePlayerInfo();
}

void CLobbyPanel::UpdatePlayerInfo()
{
	CGameLobbyClient::UpdateInfo(L"name", DigitanksWindow()->GetPlayerNickname());
}

void CLobbyPanel::LeaveLobbyCallback()
{
	if (CNetwork::IsHost())
		CGameLobbyServer::DestroyLobby(m_iLobby);
	else
		CGameLobbyClient::LeaveLobby();

	SetVisible(false);
	DigitanksWindow()->GetMainMenu()->SetVisible(true);
}

void CLobbyPanel::LobbyUpdateCallback(INetworkListener*, class CNetworkParameters*)
{
	DigitanksWindow()->GetLobbyPanel()->LobbyUpdate();
}

void CLobbyPanel::LobbyUpdate()
{
	Layout();
}

CInfoPanel::CInfoPanel()
	: CPanel(0, 0, 570, 520)
{
	m_pLevelDescription = new glgui::CLabel(0, 0, 32, 32, L"");
	m_pLevelDescription->SetWrap(true);
	m_pLevelDescription->SetFont(L"text");
	m_pLevelDescription->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pLevelDescription);
}

void CInfoPanel::Layout()
{
	m_pLevelDescription->SetSize(GetWidth()-40, 80);
	m_pLevelDescription->SetPos(20, 170);

	BaseClass::Layout();
}

CPlayerPanel::CPlayerPanel()
	: CPanel(0, 0, 100, 100)
{
	m_pName = new glgui::CLabel(0, 0, 100, 100, L"Player");
	AddControl(m_pName);
}

void CPlayerPanel::Layout()
{
	SetSize(260, 60);
	SetPos(925 - 280, 20 + 80*m_iLobbyPlayer);

	m_pName->SetSize(100, 60);
	m_pName->SetPos(20, 0);
	m_pName->SetAlign(glgui::CLabel::TA_MIDDLECENTER);
}

void CPlayerPanel::Paint(int x, int y, int w, int h)
{
	glgui::CRootPanel::PaintRect(x, y, w, h, glgui::g_clrBox);

	BaseClass::Paint(x, y, w, h);
}

void CPlayerPanel::SetPlayer(size_t iClient)
{
	CLobbyPlayer* pPlayer = CGameLobbyClient::GetPlayerByClient(iClient);

	assert(pPlayer);
	if (!pPlayer)
		return;

	m_iLobbyPlayer = CGameLobbyClient::GetPlayerIndex(iClient);

	eastl::string16 sName = pPlayer->GetInfoValue(L"name");
	if (sName.length() == 0)
		sName = L"Player";

	m_pName->SetText(sName);

	Layout();
}
