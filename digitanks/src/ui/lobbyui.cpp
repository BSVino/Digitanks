#include "lobbyui.h"

#include <tinker/cvar.h>
#include <tinker/lobby/lobby_server.h>
#include <tinker/lobby/lobby_client.h>

#include <renderer/renderer.h>

#include <digitanks/dt_lobbylistener.h>
#include <digitanks/digitankslevel.h>

#include "menu.h"
#include "digitankswindow.h"
#include "chatbox.h"

CVar lobby_gametype("lobby_gametype", "");

CLobbyPanel::CLobbyPanel()
	: CPanel(0, 0, 100, 100)
{
	SetVisible(false);

	m_pLobbyName = new glgui::CLabel(0, 0, 100, 100, L"Lobby");
	m_pLobbyName->SetFont(L"header", 30);
	AddControl(m_pLobbyName);

	m_pPlayerList = new glgui::CLabel(0, 0, 100, 100, L"Player List");
	m_pPlayerList->SetFont(L"header", 18);
	AddControl(m_pPlayerList);

	m_pAddPlayer = new glgui::CButton(0, 0, 100, 100, L"+ Add Player");
	m_pAddPlayer->SetClickedListener(this, AddPlayer);
	m_pAddPlayer->SetFont(L"header", 9);
	AddControl(m_pAddPlayer);

	m_pAddBot = new glgui::CButton(0, 0, 100, 100, L"+ Add Bot");
	m_pAddBot->SetClickedListener(this, AddBot);
	m_pAddBot->SetFont(L"header", 9);
	AddControl(m_pAddBot);

	m_pDockPanel = new CDockPanel();
	m_pDockPanel->SetBGColor(Color(12, 13, 12, 0));
	AddControl(m_pDockPanel);

	m_pChatBox = new CChatBox();
	AddControl(m_pChatBox);

	m_pLeave = new glgui::CButton(0, 0, 100, 100, L"Leave Lobby");
	m_pLeave->SetClickedListener(this, LeaveLobby);
	m_pLeave->SetFont(L"header");
	AddControl(m_pLeave);

	m_pReady = new glgui::CButton(0, 0, 100, 100, L"Ready");
	m_pReady->SetClickedListener(this, PlayerReady);
	m_pReady->SetFont(L"header");
	AddControl(m_pReady);

	m_bLayout = false;
}

void CLobbyPanel::Layout()
{
	if (!m_bLayout)
		return;

	m_bLayout = false;

	if (!IsVisible())
		return;

	size_t iWidth = DigitanksWindow()->GetWindowWidth();
	size_t iHeight = DigitanksWindow()->GetWindowHeight();

	SetSize(924, 668);
	SetPos(iWidth/2-GetWidth()/2, iHeight/2-GetHeight()/2);

	// Find the lobby leader's name
	for (size_t i = 0; i < CGameLobbyClient::L_GetNumPlayers(); i++)
	{
		CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(i);
		if (pPlayer->GetInfoValue(L"host") == L"1")
		{
			m_pLobbyName->SetText(pPlayer->GetInfoValue(L"name") + L"'s Lobby");
			break;
		}
	}

	m_pLobbyName->SetSize(GetWidth(), 30);
	m_pLobbyName->SetPos(0, 0);
	m_pLobbyName->SetAlign(glgui::CLabel::TA_MIDDLECENTER);

	m_pPlayerList->SetSize(260, 20);
	m_pPlayerList->SetPos(925 - 280, 30);
	m_pPlayerList->SetAlign(glgui::CLabel::TA_MIDDLECENTER);

	m_pAddPlayer->SetSize(70, 15);
	m_pAddPlayer->SetPos(925 - 90, 20);

	m_pAddBot->SetSize(70, 15);
	m_pAddBot->SetPos(925 - 90, 40);

	if (CGameLobbyClient::L_IsHost())
	{
		m_pAddPlayer->SetVisible(!m_bOnline);
		m_pAddBot->SetVisible(true);
	}
	else
	{
		m_pAddPlayer->SetVisible(false);
		m_pAddBot->SetVisible(false);
	}

	m_pDockPanel->SetSize(375, 480);
	m_pDockPanel->SetPos(20, 50);

	m_pChatBox->SetSize(565, 120); 
	m_pChatBox->SetPos(20, 520);

	m_pLeave->SetSize(100, 35);
	m_pLeave->SetPos(925 - 320, 668 - 55);

	m_pReady->SetSize(180, 35);
	m_pReady->SetPos(925 - 200, 668 - 55);
	if (CGameLobbyClient::L_GetPlayerByClient(CNetwork::GetClientID()))
	{
		bool bReady = !!_wtoi(CGameLobbyClient::L_GetPlayerByClient(CNetwork::GetClientID())->GetInfoValue(L"ready").c_str());
		if (bReady)
			m_pReady->SetText(L"Not Ready");
		else
			m_pReady->SetText(L"Ready");
	}
	else
		m_pReady->SetText(L"Ready");

	m_pReady->SetEnabled(CGameLobbyClient::L_GetNumPlayers() >= 2);

	gametype_t eGameType = (gametype_t)_wtoi(CGameLobbyClient::L_GetInfoValue(L"gametype").c_str());
	if (CGameLobbyClient::L_GetNumPlayers() > 8 && eGameType == GAMETYPE_ARTILLERY)
		m_pReady->SetEnabled(false);
	else if (CGameLobbyClient::L_GetNumPlayers() > 4 && eGameType == GAMETYPE_STANDARD)
		m_pReady->SetEnabled(false);

	if (CGameLobbyClient::L_GetNumPlayers() >= 8 && eGameType == GAMETYPE_ARTILLERY)
		m_pAddPlayer->SetEnabled(false);
	else if (CGameLobbyClient::L_GetNumPlayers() >= 4 && eGameType == GAMETYPE_STANDARD)
		m_pAddPlayer->SetEnabled(false);
	else
		m_pAddPlayer->SetEnabled(true);

	if (CGameLobbyClient::L_GetNumPlayers() >= 8 && eGameType == GAMETYPE_ARTILLERY)
		m_pAddBot->SetEnabled(false);
	else if (CGameLobbyClient::L_GetNumPlayers() >= 4 && eGameType == GAMETYPE_STANDARD)
		m_pAddBot->SetEnabled(false);
	else
		m_pAddBot->SetEnabled(true);

	for (size_t i = 0; i < m_apPlayerPanels.size(); i++)
	{
		m_apPlayerPanels[i]->Destructor();
		m_apPlayerPanels[i]->Delete();
	}

	m_apPlayerPanels.clear();

	for (size_t i = 0; i < CGameLobbyClient::L_GetNumPlayers(); i++)
	{
		m_apPlayerPanels.push_back(new CPlayerPanel());
		CPlayerPanel* pPanel = m_apPlayerPanels[m_apPlayerPanels.size()-1];
		AddControl(pPanel);
		pPanel->SetPlayer(CGameLobbyClient::L_GetPlayer(i)->iID);
	}

	BaseClass::Layout();
}

void CLobbyPanel::Think()
{
	if (m_bLayout)
		Layout();

	BaseClass::Think();
}

void CLobbyPanel::Paint(int x, int y, int w, int h)
{
	glgui::CRootPanel::PaintRect(x, y, w, h, Color(12, 13, 12, 255));

	BaseClass::Paint(x, y, w, h);
}

void CLobbyPanel::CreateLobby(bool bOnline)
{
	m_bOnline = bOnline;

	CGameLobbyClient::SetLobbyUpdateCallback(&LobbyUpdateCallback);
	CGameLobbyClient::SetLobbyJoinCallback(&LobbyJoinCallback);
	CGameLobbyClient::SetLobbyLeaveCallback(&LobbyLeaveCallback);
	CGameLobbyClient::SetBeginGameCallback(&BeginGameCallback);

	const char* pszPort = DigitanksWindow()->GetCommandLineSwitchValue("--port");
	int iPort = pszPort?atoi(pszPort):0;

	m_iLobby = CGameLobbyServer::CreateLobby(iPort);
	CGameLobbyServer::SetListener(DigitanksLobbyListener());

	if (m_bOnline)
	{
		CNetwork::Disconnect();
		CNetwork::SetCallbacks(NULL, CGameLobbyServer::ClientConnect, CGameLobbyServer::ClientDisconnect);
		CNetwork::CreateHost(iPort);
	}

	CGameLobbyClient::S_JoinLobby(m_iLobby);
	CGameLobbyClient::S_UpdatePlayer(L"host", L"1");
	CGameLobbyClient::S_UpdateLobby(L"gametype", sprintf(L"%d", (gametype_t)lobby_gametype.GetInt()));

	if (!m_bOnline)
	{
		CGameLobbyClient::S_AddBot();
		CGameLobbyClient::S_AddBot();
		CGameLobbyClient::S_AddBot();
	}

	for (size_t i = 0; i < CGameLobbyClient::L_GetNumPlayers(); i++)
		CGameLobbyClient::S_UpdatePlayer(CGameLobbyClient::L_GetPlayer(i)->iID, L"color", sprintf(L"%d", i));

	if ((gametype_t)lobby_gametype.GetInt() == GAMETYPE_ARTILLERY)
		m_pDockPanel->SetDockedPanel(new CArtilleryGamePanel(true));
	else
		m_pDockPanel->SetDockedPanel(new CStrategyGamePanel(true));

	SetVisible(true);
	DigitanksWindow()->GetMainMenu()->SetVisible(false);

	m_bLayout = true;
}

void CLobbyPanel::ConnectToLocalLobby(const eastl::string16& sHost)
{
	m_bOnline = true;

	CGameLobbyClient::SetLobbyUpdateCallback(&LobbyUpdateCallback);
	CGameLobbyClient::SetLobbyJoinCallback(&LobbyJoinCallback);
	CGameLobbyClient::SetLobbyLeaveCallback(&LobbyLeaveCallback);
	CGameLobbyClient::SetBeginGameCallback(&BeginGameCallback);

	const char* pszPort = DigitanksWindow()->GetCommandLineSwitchValue("--port");
	int iPort = pszPort?atoi(pszPort):0;

	CNetwork::ConnectToHost(convertstring<char16_t, char>(sHost).c_str(), iPort);
	if (!CNetwork::IsConnected())
		return;

	m_pDockPanel->SetDockedPanel(new CInfoPanel());

	SetVisible(true);
	DigitanksWindow()->GetMainMenu()->SetVisible(false);

	m_bLayout = true;
}

void CLobbyPanel::UpdatePlayerInfo()
{
	CNetwork::SetRunningClientFunctions(false);
	CGameLobbyClient::S_UpdatePlayer(L"name", DigitanksWindow()->GetPlayerNickname());
	CGameLobbyClient::S_UpdatePlayer(L"ready", L"0");
	CGameLobbyClient::S_UpdatePlayer(L"color", L"random");
}

void CLobbyPanel::LeaveLobbyCallback()
{
	CGameLobbyClient::S_LeaveLobby();

	if (CNetwork::IsHost())
		CGameLobbyServer::DestroyLobby(m_iLobby);

	if (m_bOnline)
		CNetwork::Disconnect();

	SetVisible(false);
	DigitanksWindow()->GetMainMenu()->SetVisible(true);
}

void CLobbyPanel::LobbyUpdateCallback(INetworkListener*, class CNetworkParameters*)
{
	DigitanksWindow()->GetLobbyPanel()->LobbyUpdate();
}

void CLobbyPanel::LobbyUpdate()
{
	m_bLayout = true;
}

void CLobbyPanel::LobbyJoinCallback(INetworkListener*, class CNetworkParameters*)
{
	DigitanksWindow()->GetLobbyPanel()->UpdatePlayerInfo();
}

void CLobbyPanel::LobbyLeaveCallback(INetworkListener*, class CNetworkParameters*)
{
	DigitanksWindow()->GetLobbyPanel()->SetVisible(false);
	DigitanksWindow()->GetMainMenu()->SetVisible(true);

	if (DigitanksWindow()->GetLobbyPanel()->m_bOnline)
		CNetwork::Disconnect();
}

void CLobbyPanel::PlayerReadyCallback()
{
	bool bReady = !!_wtoi(CGameLobbyClient::L_GetPlayerByClient(CNetwork::GetClientID())->GetInfoValue(L"ready").c_str());

	if (bReady)
		CGameLobbyClient::S_UpdatePlayer(L"ready", L"0");
	else
		CGameLobbyClient::S_UpdatePlayer(L"ready", L"1");
}

void CLobbyPanel::AddPlayerCallback()
{
	CGameLobbyClient::S_AddLocalPlayer();
}

void CLobbyPanel::AddBotCallback()
{
	CGameLobbyClient::S_AddBot();
}

void CLobbyPanel::BeginGameCallback(INetworkListener*, class CNetworkParameters*)
{
	CDigitanksLevel* pLevel = CDigitanksGame::GetLevel(CGameLobbyClient::L_GetInfoValue(L"level_file"));
	if (!pLevel)
		return;

	CVar::SetCVar(L"game_level", pLevel->GetFile());

	DigitanksWindow()->GetLobbyPanel()->SetVisible(false);

	DigitanksWindow()->Restart(GAMETYPE_FROM_LOBBY);
}

CInfoPanel::CInfoPanel()
	: CPanel(0, 0, 570, 520)
{
	m_pLobbyDescription = new glgui::CLabel(0, 0, 32, 32, L"");
	m_pLobbyDescription->SetWrap(true);
	m_pLobbyDescription->SetFont(L"text");
	m_pLobbyDescription->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pLobbyDescription);
}

void CInfoPanel::Layout()
{
	m_pLobbyDescription->SetSize(GetWidth()-40, 80);
	m_pLobbyDescription->SetPos(20, 20);

	gametype_t eGameType = (gametype_t)_wtoi(CGameLobbyClient::L_GetInfoValue(L"gametype").c_str());

	if (eGameType == GAMETYPE_ARTILLERY)
		m_pLobbyDescription->SetText(L"Game Mode: Artillery\n");
	else
		m_pLobbyDescription->SetText(L"Game Mode: Standard\n");

	m_pLobbyDescription->AppendText(eastl::string16(L"Level: ") + CGameLobbyClient::L_GetInfoValue(L"level") + L"\n");

	if (eGameType == GAMETYPE_ARTILLERY)
	{
		m_pLobbyDescription->AppendText(eastl::string16(L"Tanks per player: ") + CGameLobbyClient::L_GetInfoValue(L"tanks") + L"\n");

		eastl::string16 sHeight = CGameLobbyClient::L_GetInfoValue(L"terrain");
		float flHeight = (float)_wtof(sHeight.c_str());
		if (fabs(flHeight - 10.0f) < 0.5f)
			sHeight = L"Flatty";
		else if (fabs(flHeight - 50.0f) < 0.5f)
			sHeight = L"Hilly";
		else if (fabs(flHeight - 80.0f) < 0.5f)
			sHeight = L"Mountainy";
		else if (fabs(flHeight - 120.0f) < 0.5f)
			sHeight = L"Everesty";
		m_pLobbyDescription->AppendText(eastl::string16(L"Terrain height: ") + sHeight + L"\n");
	}

	BaseClass::Layout();
}

CPlayerPanel::CPlayerPanel()
	: CPanel(0, 0, 100, 100)
{
	m_iLobbyPlayer = ~0;

	m_pName = new glgui::CLabel(0, 0, 100, 100, L"Player");
	m_pName->SetFont(L"text");
	m_pName->SetWrap(false);
	AddControl(m_pName);

	if (CGameLobbyClient::L_IsHost())
	{
		m_pKick = new glgui::CButton(0, 0, 100, 100, L"Kick");
		m_pKick->SetClickedListener(this, Kick);
		m_pKick->SetFont(L"header", 11);
		m_pKick->SetButtonColor(Color(255, 0, 0));
		AddControl(m_pKick);
	}
	else
		m_pKick = NULL;

	m_pColor = new glgui::CMenu(L"Color");
	m_pColor->SetFont(L"text", 11);
	AddControl(m_pColor);
	m_bRandomColor = true;

	for (int i = 0; i < 8; i++)
		m_aiAvailableColors.push_back(i);

	for (size_t i = 0; i < CGameLobbyClient::L_GetNumPlayers(); i++)
	{
		CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(i);
		eastl::string16 sColor = pPlayer->GetInfoValue(L"color");
		if (sColor == L"random" || sColor == L"")
			continue;

		size_t iColor = _wtoi(sColor.c_str());
		for (size_t j = 0; j < m_aiAvailableColors.size(); j++)
		{
			if (m_aiAvailableColors[j] == iColor)
			{
				m_aiAvailableColors.erase(m_aiAvailableColors.begin() + j);
				break;
			}
		}
	}

	m_pColor->AddSubmenu(L"Random", this, ColorChosen);
	for (size_t i = 0; i < m_aiAvailableColors.size(); i++)
		m_pColor->AddSubmenu(g_aszTeamNames[m_aiAvailableColors[i]], this, ColorChosen);
}

void CPlayerPanel::Layout()
{
	SetSize(260, 40);
	SetPos(925 - 280, 70 + 60*m_iLobbyPlayer);

	m_pName->SetSize(100, 40);
	m_pName->SetPos(40, 0);
	m_pName->SetAlign(glgui::CLabel::TA_LEFTCENTER);

	CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(m_iLobbyPlayer);

	if (pPlayer && pPlayer->GetInfoValue(L"ready") == L"1")
		m_pName->SetFGColor(Color(0, 255, 0));
	else
		m_pName->SetFGColor(Color(255, 255, 255));

	if (m_pKick)
	{
		m_pKick->SetSize(40, 15);
		m_pKick->SetPos(200, GetHeight()/2-m_pKick->GetHeight()/2);
	}

	m_pColor->SetPos(5, 5);
	m_pColor->SetSize(30, 30);

	if (m_bRandomColor)
	{
		m_pColor->SetButtonColor(Color(0, 0, 0));
		m_pColor->SetText(L"Rnd");
	}
	else
	{
		m_pColor->SetButtonColor(g_aclrTeamColors[m_iColor]);
		m_pColor->SetText(L"");
	}
}

void CPlayerPanel::Paint(int x, int y, int w, int h)
{
	glgui::CRootPanel::PaintRect(x, y, w, h, glgui::g_clrBox);

	BaseClass::Paint(x, y, w, h);
}

void CPlayerPanel::SetPlayer(size_t iID)
{
	CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayerByID(iID);

	assert(pPlayer);
	if (!pPlayer)
		return;

	m_iLobbyPlayer = CGameLobbyClient::L_GetPlayerIndexByID(iID);

	eastl::string16 sName = pPlayer->GetInfoValue(L"name");
	if (sName.length() == 0)
		sName = L"Player";

	eastl::string16 sHost = pPlayer->GetInfoValue(L"host");
	if (sHost == L"1")
		sName += L" - Host";

	m_pName->SetText(sName);

	if (m_pKick && iID == CGameLobbyClient::L_GetLocalPlayerID())
		m_pKick->SetVisible(false);

	m_pColor->SetEnabled(iID == CGameLobbyClient::L_GetLocalPlayerID() || CGameLobbyClient::L_IsHost());

	eastl::string16 sColor = pPlayer->GetInfoValue(L"color");
	m_bRandomColor = (sColor == L"random" || sColor == L"");
	m_iColor = _wtoi(sColor.c_str());

	Layout();
}

void CPlayerPanel::KickCallback()
{
	CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(m_iLobbyPlayer);

	assert(pPlayer);
	if (!pPlayer)
		return;

	CGameLobbyClient::S_RemovePlayer(pPlayer->iID);
}

void CPlayerPanel::ColorChosenCallback()
{
	CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(m_iLobbyPlayer);

	assert(pPlayer);
	if (!pPlayer)
		return;

	size_t iSelected = m_pColor->GetSelectedMenu();
	m_bRandomColor = (iSelected == 0);
	if (!m_bRandomColor)
		m_iColor = m_aiAvailableColors[iSelected-1];

	m_pColor->Pop(true, true);

	if (m_bRandomColor)
		CGameLobbyClient::S_UpdatePlayer(pPlayer->iID, L"color", L"random");
	else
		CGameLobbyClient::S_UpdatePlayer(pPlayer->iID, L"color", sprintf(L"%d", m_iColor));
}
