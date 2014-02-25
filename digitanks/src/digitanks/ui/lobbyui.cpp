#include "lobbyui.h"

#include <tinker/cvar.h>
#include <tengine/lobby/lobby_server.h>
#include <tengine/lobby/lobby_client.h>
#include <tengine/ui/chatbox.h>

#include <renderer/renderer.h>

#include <dt_lobbylistener.h>
#include <digitankslevel.h>

#include "menu.h"
#include "digitankswindow.h"

#define _T(x) x

CVar lobby_gametype("lobby_gametype", "");

CLobbyPanel::CLobbyPanel()
	: CPanel(0, 0, 100, 100)
{
#if 0
	SetVisible(false);

	m_pLobbyName = new glgui::CLabel(0, 0, 100, 100, _T("Lobby"));
	m_pLobbyName->SetFont(_T("header"), 30);
	AddControl(m_pLobbyName);

	m_pPlayerList = new glgui::CLabel(0, 0, 100, 100, _T("Player List"));
	m_pPlayerList->SetFont(_T("header"), 18);
	AddControl(m_pPlayerList);

	m_pAddPlayer = new glgui::CButton(0, 0, 100, 100, _T("+ Add Player"));
	m_pAddPlayer->SetClickedListener(this, AddPlayer);
	m_pAddPlayer->SetFont(_T("header"), 9);
	AddControl(m_pAddPlayer);

	m_pAddBot = new glgui::CButton(0, 0, 100, 100, _T("+ Add Bot"));
	m_pAddBot->SetClickedListener(this, AddBot);
	m_pAddBot->SetFont(_T("header"), 9);
	AddControl(m_pAddBot);

	m_pDockPanel = new CDockPanel();
	m_pDockPanel->SetBGColor(Color(12, 13, 12, 0));
	AddControl(m_pDockPanel);

	m_pChatBox = new CChatBox();
	AddControl(m_pChatBox);

	m_pLeave = new glgui::CButton(0, 0, 100, 100, _T("Leave Lobby"));
	m_pLeave->SetClickedListener(this, LeaveLobby);
	m_pLeave->SetFont(_T("header"));
	AddControl(m_pLeave);

	m_pReady = new glgui::CButton(0, 0, 100, 100, _T("Ready"));
	m_pReady->SetClickedListener(this, PlayerReady);
	m_pReady->SetFont(_T("header"));
	AddControl(m_pReady);

	m_bLayout = false;
#endif
}

void CLobbyPanel::Layout()
{
#if 0
	if (!IsVisible())
		return;

	if (!m_bLayout)
		return;

	if (!CGameLobbyClient::L_GetNumPlayers())
		return;

	if (CGameLobbyClient::L_GetPlayerIndexByClient(LobbyNetwork()->GetClientID()) == ~0)
		return;

	m_bLayout = false;

	size_t iWidth = DigitanksWindow()->GetWindowWidth();
	size_t iHeight = DigitanksWindow()->GetWindowHeight();

	SetSize(924, 668);
	SetPos(iWidth/2-GetWidth()/2, iHeight/2-GetHeight()/2);

	bool bAllButMe = true;

	// Find the lobby leader's name
	for (size_t i = 0; i < CGameLobbyClient::L_GetNumPlayers(); i++)
	{
		CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(i);
		if (pPlayer->GetInfoValue(_T("host")) == _T("1"))
		{
			m_pLobbyName->SetText(pPlayer->GetInfoValue(_T("name")) + _T("'s Lobby"));

			if (pPlayer->GetInfoValue(_T("ready")) == _T("1"))
				bAllButMe = false;

			continue;
		}

		if (pPlayer->GetInfoValue(_T("ready")) != _T("1"))
			bAllButMe = false;
	}

	if (!LobbyNetwork()->IsHost())
		bAllButMe = false;

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

	if (bAllButMe)
		m_pReady->SetText(_T("Start Game"));
	else if (CGameLobbyClient::L_GetPlayerByClient(LobbyNetwork()->GetClientID()))
	{
		bool bReady = !!stoi(CGameLobbyClient::L_GetPlayerByClient(LobbyNetwork()->GetClientID())->GetInfoValue(_T("ready")).c_str());
		if (bReady)
			m_pReady->SetText(_T("Not Ready"));
		else
			m_pReady->SetText(_T("Ready"));
	}
	else
		m_pReady->SetText(_T("Ready"));

	m_pReady->SetEnabled(CGameLobbyClient::L_GetNumPlayers() >= 2);

	if (LobbyNetwork()->IsHost() && CGameLobbyClient::L_GetInfoValue(_T("level")).length() == 0)
		m_pReady->SetEnabled(false);

	gametype_t eGameType = (gametype_t)stoi(CGameLobbyClient::L_GetInfoValue(_T("gametype")).c_str());
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
#endif
}

void CLobbyPanel::Think()
{
	if (m_bLayout)
		Layout();

	BaseClass::Think();
}

void CLobbyPanel::Paint(float x, float y, float w, float h)
{
#if 0
	glgui::CRootPanel::PaintRect(x, y, w, h, Color(12, 13, 12, 255));

	BaseClass::Paint(x, y, w, h);
#endif
}

void CLobbyPanel::CreateLobby(bool bOnline)
{
	m_bOnline = bOnline;

	CGameLobbyClient::SetLobbyUpdateCallback(&LobbyUpdateCallback);
	CGameLobbyClient::SetLobbyJoinCallback(&LobbyJoinCallback);
	CGameLobbyClient::SetLobbyLeaveCallback(&LobbyLeaveCallback);
	CGameLobbyClient::SetBeginGameCallback(&BeginGameCallback);

	const char* pszPort = DigitanksWindow()->GetCommandLineSwitchValue("--lobby-port");
	int iPort = pszPort?atoi(pszPort):0;

	m_iLobby = CGameLobbyServer::CreateLobby();
	CGameLobbyServer::SetListener(DigitanksLobbyListener());

	if (m_bOnline)
	{
		GameNetwork()->Disconnect();
		LobbyNetwork()->Disconnect();
		LobbyNetwork()->SetCallbacks(NULL, NULL, CGameLobbyServer::ClientEnterGame, CGameLobbyServer::ClientDisconnect);
		LobbyNetwork()->CreateHost(iPort);
	}

	CGameLobbyClient::S_JoinLobby(m_iLobby);
	CGameLobbyClient::S_UpdatePlayer(_T("host"), _T("1"));
	CGameLobbyClient::S_UpdateLobby(_T("gametype"), sprintf(tstring("%d"), (gametype_t)lobby_gametype.GetInt()));

	if (!m_bOnline)
	{
		CGameLobbyClient::S_AddBot();
		CGameLobbyClient::S_AddBot();
		CGameLobbyClient::S_AddBot();
	}

	for (size_t i = 0; i < CGameLobbyClient::L_GetNumPlayers(); i++)
		CGameLobbyClient::S_UpdatePlayer(CGameLobbyClient::L_GetPlayer(i)->iID, _T("color"), sprintf(tstring("%d"), i));

	if ((gametype_t)lobby_gametype.GetInt() == GAMETYPE_ARTILLERY)
		m_pDockPanel->SetDockedPanel(new CArtilleryGamePanel(true));
	else
		m_pDockPanel->SetDockedPanel(new CStrategyGamePanel(true));

	SetVisible(true);
	DigitanksWindow()->GetMainMenu()->SetVisible(false);

	m_bLayout = true;
}

void CLobbyPanel::ConnectToLocalLobby(const tstring& sHost)
{
	m_sHost = sHost;
	m_bOnline = true;

	CGameLobbyClient::SetLobbyUpdateCallback(&LobbyUpdateCallback);
	CGameLobbyClient::SetLobbyJoinCallback(&LobbyJoinCallback);
	CGameLobbyClient::SetLobbyLeaveCallback(&LobbyLeaveCallback);
	CGameLobbyClient::SetBeginGameCallback(&BeginGameCallback);

	const char* pszPort = DigitanksWindow()->GetCommandLineSwitchValue("--lobby-port");
	int iPort = pszPort?atoi(pszPort):0;

	GameNetwork()->Disconnect();
	LobbyNetwork()->SetCallbacks(NULL, NULL, CGameLobbyClient::ClientEnterGame, CGameLobbyClient::ClientDisconnect);
	LobbyNetwork()->ConnectToHost(convertstring<tchar, char>(sHost).c_str(), iPort);
	if (!LobbyNetwork()->IsConnected())
		return;

	LobbyNetwork()->SetLoading(false);

	m_pDockPanel->SetDockedPanel(new CInfoPanel());

	DigitanksWindow()->GetMainMenu()->SetVisible(false);

	SetVisible(true);
	m_bLayout = true;
}

void CLobbyPanel::UpdatePlayerInfo()
{
	LobbyNetwork()->SetRunningClientFunctions(false);
	CGameLobbyClient::S_UpdatePlayer(_T("name"), DigitanksWindow()->GetPlayerNickname());
	CGameLobbyClient::S_UpdatePlayer(_T("ready"), _T("0"));
	CGameLobbyClient::S_UpdatePlayer(_T("color"), _T("random"));
}

void CLobbyPanel::LeaveLobbyCallback(const tstring& sArgs)
{
	bool bWasHost = LobbyNetwork()->IsHost();

	CGameLobbyClient::S_LeaveLobby();

	if (bWasHost)
		CGameLobbyServer::DestroyLobby(m_iLobby);

	if (m_bOnline)
		LobbyNetwork()->Disconnect();

	SetVisible(false);
	DigitanksWindow()->GetMainMenu()->SetVisible(true);
}

void CLobbyPanel::LobbyUpdateCallback(int iConnection, INetworkListener*, class CNetworkParameters*)
{
	TAssert(iConnection == CONNECTION_LOBBY);

	DigitanksWindow()->GetLobbyPanel()->LobbyUpdate();
}

void CLobbyPanel::LobbyUpdate()
{
	m_bLayout = true;
}

void CLobbyPanel::LobbyJoinCallback(int iConnection, INetworkListener*, class CNetworkParameters*)
{
	TAssert(iConnection == CONNECTION_LOBBY);

	DigitanksWindow()->GetLobbyPanel()->UpdatePlayerInfo();
}

void CLobbyPanel::LobbyLeaveCallback(int iConnection, INetworkListener*, class CNetworkParameters*)
{
	TAssert(iConnection == CONNECTION_LOBBY);

	DigitanksWindow()->GetLobbyPanel()->SetVisible(false);
	DigitanksWindow()->GetMainMenu()->SetVisible(true);

	if (DigitanksWindow()->GetLobbyPanel()->m_bOnline && !LobbyNetwork()->IsHost())
		LobbyNetwork()->Disconnect();
}

void CLobbyPanel::PlayerReadyCallback(const tstring& sArgs)
{
	bool bReady = !!stoi(CGameLobbyClient::L_GetPlayerByClient(LobbyNetwork()->GetClientID())->GetInfoValue(_T("ready")).c_str());

	if (bReady)
		CGameLobbyClient::S_UpdatePlayer(_T("ready"), _T("0"));
	else
		CGameLobbyClient::S_UpdatePlayer(_T("ready"), _T("1"));
}

void CLobbyPanel::AddPlayerCallback(const tstring& sArgs)
{
	CGameLobbyClient::S_AddLocalPlayer();
}

void CLobbyPanel::AddBotCallback(const tstring& sArgs)
{
	CGameLobbyClient::S_AddBot();
}

void CLobbyPanel::BeginGameCallback(int iConnection, INetworkListener*, class CNetworkParameters*)
{
	TAssert(iConnection == CONNECTION_LOBBY);

	CDigitanksLevel* pLevel = CDigitanksGame::GetLevel(CGameLobbyClient::L_GetInfoValue(_T("level_file")));
	if (!pLevel)
		return;

	CVar::SetCVar(_T("game_level"), pLevel->GetFile());

	DigitanksWindow()->GetLobbyPanel()->SetVisible(false);

	const char* pszPort = DigitanksWindow()->GetCommandLineSwitchValue("--port");
	int iPort = pszPort?atoi(pszPort):0;

	if (LobbyNetwork()->IsHost())
		GameNetwork()->CreateHost(iPort);
	else
		GameNetwork()->ConnectToHost(convertstring<tchar, char>(DigitanksWindow()->GetLobbyPanel()->m_sHost).c_str(), iPort);

	if (GameNetwork()->IsConnected())
		DigitanksWindow()->Restart(GAMETYPE_FROM_LOBBY);
	else
		DigitanksWindow()->Restart(GAMETYPE_MENU);
}

CInfoPanel::CInfoPanel()
	: CPanel(0, 0, 570, 520)
{
#if 0
	m_pLobbyDescription = new glgui::CLabel(0, 0, 32, 32, _T(""));
	m_pLobbyDescription->SetWrap(true);
	m_pLobbyDescription->SetFont(_T("text"));
	m_pLobbyDescription->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pLobbyDescription);
#endif
}

void CInfoPanel::Layout()
{
#if 0
	m_pLobbyDescription->SetSize(GetWidth()-40, 80);
	m_pLobbyDescription->SetPos(20, 20);

	gametype_t eGameType = (gametype_t)stoi(CGameLobbyClient::L_GetInfoValue(_T("gametype")).c_str());

	if (eGameType == GAMETYPE_ARTILLERY)
		m_pLobbyDescription->SetText(_T("Game Mode: Artillery\n"));
	else
		m_pLobbyDescription->SetText(_T("Game Mode: Standard\n"));

	m_pLobbyDescription->AppendText(tstring(_T("Level: ")) + CGameLobbyClient::L_GetInfoValue(_T("level")) + _T("\n"));

	if (eGameType == GAMETYPE_ARTILLERY)
	{
		m_pLobbyDescription->AppendText(tstring(_T("Tanks per player: ")) + CGameLobbyClient::L_GetInfoValue(_T("tanks")) + _T("\n"));

		tstring sHeight = CGameLobbyClient::L_GetInfoValue(_T("terrain"));
		float flHeight = (float)stof(sHeight.c_str());
		if (fabs(flHeight - 10.0f) < 0.5f)
			sHeight = _T("Flatty");
		else if (fabs(flHeight - 50.0f) < 0.5f)
			sHeight = _T("Hilly");
		else if (fabs(flHeight - 80.0f) < 0.5f)
			sHeight = _T("Mountainy");
		else if (fabs(flHeight - 120.0f) < 0.5f)
			sHeight = _T("Everesty");
		m_pLobbyDescription->AppendText(tstring(_T("Terrain height: ")) + sHeight + _T("\n"));
	}

	BaseClass::Layout();
#endif
}

CPlayerPanel::CPlayerPanel()
	: CPanel(0, 0, 100, 100)
{
#if 0
	m_iLobbyPlayer = ~0;

	m_pName = new glgui::CLabel(0, 0, 100, 100, _T("Player"));
	m_pName->SetFont(_T("text"));
	m_pName->SetWrap(false);
	AddControl(m_pName);

	if (CGameLobbyClient::L_IsHost())
	{
		m_pKick = new glgui::CButton(0, 0, 100, 100, _T("Kick"));
		m_pKick->SetClickedListener(this, Kick);
		m_pKick->SetFont(_T("header"), 11);
		m_pKick->SetButtonColor(Color(255, 0, 0));
		AddControl(m_pKick);
	}
	else
		m_pKick = NULL;

	m_pColor = new glgui::CMenu(_T("Color"));
	m_pColor->SetFont(_T("text"), 11);
	AddControl(m_pColor);
	m_bRandomColor = true;

	for (int i = 0; i < 8; i++)
		m_aiAvailableColors.push_back(i);

	for (size_t i = 0; i < CGameLobbyClient::L_GetNumPlayers(); i++)
	{
		CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(i);
		tstring sColor = pPlayer->GetInfoValue(_T("color"));
		if (sColor == _T("random") || sColor == _T(""))
			continue;

		size_t iColor = stoi(sColor.c_str());
		for (size_t j = 0; j < m_aiAvailableColors.size(); j++)
		{
			if (m_aiAvailableColors[j] == iColor)
			{
				m_aiAvailableColors.erase(m_aiAvailableColors.begin() + j);
				break;
			}
		}
	}

	m_pColor->AddSubmenu(_T("Random"), this, ColorChosen);
	for (size_t i = 0; i < m_aiAvailableColors.size(); i++)
		m_pColor->AddSubmenu(g_aszTeamNames[m_aiAvailableColors[i]], this, ColorChosen);
#endif
}

void CPlayerPanel::Layout()
{
#if 0
	SetSize(260, 40);
	SetPos(925 - 280, 70 + 60*m_iLobbyPlayer);

	m_pName->SetSize(100, 40);
	m_pName->SetPos(40, 0);
	m_pName->SetAlign(glgui::CLabel::TA_LEFTCENTER);

	CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(m_iLobbyPlayer);

	if (pPlayer && pPlayer->GetInfoValue(_T("ready")) == _T("1"))
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
		m_pColor->SetText(_T("Rnd"));
	}
	else
	{
		m_pColor->SetButtonColor(g_aclrTeamColors[m_iColor]);
		m_pColor->SetText(_T(""));
	}
#endif
}

void CPlayerPanel::Paint(float x, float y, float w, float h)
{
#if 0
	glgui::CRootPanel::PaintRect(x, y, w, h, glgui::g_clrBox);

	BaseClass::Paint(x, y, w, h);
#endif
}

void CPlayerPanel::SetPlayer(size_t iID)
{
#if 0
	CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayerByID(iID);

	TAssert(pPlayer);
	if (!pPlayer)
		return;

	m_iLobbyPlayer = CGameLobbyClient::L_GetPlayerIndexByID(iID);

	tstring sName = pPlayer->GetInfoValue(_T("name"));
	if (sName.length() == 0)
		sName = _T("Player");

	tstring sHost = pPlayer->GetInfoValue(_T("host"));
	if (sHost == _T("1"))
		sName += _T(" - Host");

	m_pName->SetText(sName);

	if (m_pKick && iID == CGameLobbyClient::L_GetLocalPlayerID())
		m_pKick->SetVisible(false);

	m_pColor->SetEnabled(iID == CGameLobbyClient::L_GetLocalPlayerID() || CGameLobbyClient::L_IsHost());

	bool bReady = !!stoi(CGameLobbyClient::L_GetPlayerByClient(LobbyNetwork()->GetClientID())->GetInfoValue(_T("ready")).c_str());
	if (bReady)
		m_pColor->SetEnabled(false);

	tstring sColor = pPlayer->GetInfoValue(_T("color"));
	m_bRandomColor = (sColor == _T("random") || sColor == _T(""));
	m_iColor = stoi(sColor.c_str());

	Layout();
#endif
}

void CPlayerPanel::KickCallback(const tstring& sArgs)
{
	CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(m_iLobbyPlayer);

	TAssert(pPlayer);
	if (!pPlayer)
		return;

	CGameLobbyClient::S_RemovePlayer(pPlayer->iID);
}

void CPlayerPanel::ColorChosenCallback(const tstring& sArgs)
{
#if 0
	CLobbyPlayer* pPlayer = CGameLobbyClient::L_GetPlayer(m_iLobbyPlayer);

	TAssert(pPlayer);
	if (!pPlayer)
		return;

	size_t iSelected = m_pColor->GetSelectedMenu();
	m_bRandomColor = (iSelected == 0);
	if (!m_bRandomColor)
		m_iColor = m_aiAvailableColors[iSelected-1];

	m_pColor->Pop(true, true);

	if (m_bRandomColor)
		CGameLobbyClient::S_UpdatePlayer(pPlayer->iID, _T("color"), _T("random"));
	else
		CGameLobbyClient::S_UpdatePlayer(pPlayer->iID, _T("color"), sprintf(tstring("%d"), m_iColor));
#endif
}
