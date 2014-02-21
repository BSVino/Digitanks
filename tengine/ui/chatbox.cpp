#include "chatbox.h"

#include <tinker_platform.h>
#include <strutils.h>

#include <tinker/keys.h>
#include <tinker/cvar.h>
#include <tinker/console.h>
#include <network/commands.h>
#include <lobby/lobby_server.h>
#include <game/entities/game.h>
#include <glgui/rootpanel.h>
#include <glgui/label.h>
#include <glgui/textfield.h>

#include "gamewindow.h"

SERVER_COMMAND(CONNECTION_UNDEFINED, ServerChatSay)
{
	GameWindow()->GetChatBox()->PrintChat(sParameters);
}

CLIENT_COMMAND(CONNECTION_UNDEFINED, ClientChatSay)
{
	// Once the server gets it, send it to all of the clients, but with the speaker's name in there.

	tstring sName = "Player";

	if (iConnection == CONNECTION_LOBBY)
	{
		size_t iLobby = CGameLobbyServer::GetPlayerLobby(CGameLobbyServer::GetClientPlayerID(iClient));
		CGameLobby* pLobby = CGameLobbyServer::GetLobby(iLobby);
		if (pLobby)
		{
			CLobbyPlayer* pPlayer = pLobby->GetPlayerByClient(iClient);
			if (pPlayer)
				sName = pPlayer->GetInfoValue("name");
		}
	}
	else
	{
		int iIntClient = iClient;
		if (iIntClient < 0)
		{
			if (Game()->GetNumLocalPlayers() > 0)
				sName = Game()->GetLocalPlayer(0)->GetPlayerName();
		}
		else
		{
			for (size_t i = 0; i < Game()->GetNumPlayers(); i++)
			{
				if (iIntClient < 0 && !Game()->GetPlayer(i)->GetTeam()->IsPlayerControlled())
					continue;

				if (Game()->GetPlayer(i)->GetClient() == iIntClient)
				{
					sName = Game()->GetPlayer(i)->GetPlayerName();
					break;
				}
			}
		}
	}

	ServerChatSay.RunCommand(iConnection, sName + ": " + sParameters + "\n");
}

void ChatSay(class CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
{
	if (sCommand.find(' ') == ~0)
		return;

	if (LobbyNetwork()->IsConnected())
		ClientChatSay.RunCommand(CONNECTION_LOBBY, sCommand.substr(sCommand.find(' ')));
	else
		ClientChatSay.RunCommand(CONNECTION_GAME, sCommand.substr(sCommand.find(' ')));
}

CCommand chat_say("say", ::ChatSay);
CCommand chat_say2("chat_say", ::ChatSay);

void ChatOpen(class CCommand* pCommand, tvector<tstring>& asTokens, const tstring& sCommand)
{
	GameWindow()->OpenChat();
}

CCommand chat_open("chat_open", ::ChatOpen);

CChatBox::CChatBox(bool bFloating)
	: glgui::CPanel(0, 0, 100, 100)
{
	if (bFloating)
		glgui::CRootPanel::Get()->AddControl(this, true);

	m_pOutput = new glgui::CLabel(0, 0, 100, 100, "");
	m_pOutput->SetAlign(glgui::CLabel::TA_BOTTOMLEFT);
	m_pOutput->SetScissor(true);
	AddControl(m_pOutput);

	m_pInput = new glgui::CTextField();
	AddControl(m_pInput);

	m_flLastMessage = 0;

	SetSize(glgui::CRootPanel::Get()->GetWidth()/3, 250);
	SetPos(glgui::CRootPanel::Get()->GetWidth()/6, glgui::CRootPanel::Get()->GetHeight()/2);

	m_bFloating = bFloating;
}

CChatBox::~CChatBox()
{
	if (m_bFloating)
		glgui::CRootPanel::Get()->RemoveControl(this);
}

bool CChatBox::IsVisible()
{
	if (!m_bFloating)
		return BaseClass::IsVisible();

	if (CApplication::Get()->GetConsole()->IsVisible())
		return false;

	if (!GameServer())
		return false;

	float flTimeSinceLastMessage = (float)(GameServer()->GetGameTime() - m_flLastMessage);
	if (flTimeSinceLastMessage > 0 && flTimeSinceLastMessage < 6)
		return true;

	return BaseClass::IsVisible();
}

void CChatBox::SetVisible(bool bVisible)
{
	BaseClass::SetVisible(bVisible);

	m_pInput->SetFocus(bVisible);

	if (IsVisible())
		Layout();

	if (!m_bFloating)
		return;

	m_flLastMessage = GameServer()->GetGameTime();
}

bool CChatBox::IsOpen()
{
	return BaseClass::IsVisible();
}

void CChatBox::Layout()
{
	m_pInput->SetSize(GetWidth(), 20);
	m_pInput->SetPos(0, GetHeight()-20);

	m_pOutput->SetSize(GetWidth(), GetHeight()-24);
	m_pOutput->SetPos(0, 0);

	BaseClass::Layout();
}

void CChatBox::Paint(float x, float y, float w, float h)
{
	float flAlpha;
	float flTimeSinceLastMessage = (float)(GameServer()->GetGameTime() - m_flLastMessage);
	if (IsOpen() || flTimeSinceLastMessage < 1)
		flAlpha = 1;
	else if (!m_bFloating)
		flAlpha = 1;
	else if (flTimeSinceLastMessage < 5)
		flAlpha = RemapValClamped(flTimeSinceLastMessage, 1, 2, 1, 0.5f);
	else
		flAlpha = RemapValClamped(flTimeSinceLastMessage, 5, 6, 0.5f, 0);

	if (flAlpha < 0)
		return;

	if (IsOpen())
		glgui::CRootPanel::PaintRect(x, y, w, h, Color(0, 0, 0, (int)(200*flAlpha)), 5, true);
	else
		glgui::CRootPanel::PaintRect(x, y, w, h - 20, Color(0, 0, 0, (int)(200*flAlpha)));

	m_pOutput->SetTextColor(Color(255, 255, 255, (int)(255*flAlpha)));

	if (m_bFloating)
		m_pInput->SetVisible(IsOpen());

	BaseClass::Paint(x, y, w, h);
}

void CChatBox::PrintChat(tstring sText)
{
	TMsg(sText);

	m_pOutput->AppendText(sText);

	Layout();

	m_flLastMessage = GameServer()->GetGameTime();
}

bool CChatBox::KeyPressed(int code, bool bCtrlDown)
{
	if (!IsOpen())
		return false;

	if (code == TINKER_KEY_ESCAPE)
	{
		GameWindow()->CloseChat();
		m_pInput->SetText("");
		return true;
	}

	if (code == TINKER_KEY_ENTER || code == TINKER_KEY_KP_ENTER)
	{
		tstring sText = m_pInput->GetText();
		m_pInput->SetText("");

		CCommand::Run(tstring("say ") + sText);

		GameWindow()->CloseChat();

		return true;
	}

	bool bReturn = BaseClass::KeyPressed(code, bCtrlDown);

	if (bReturn)
		return true;

	return false;
}

bool CChatBox::CharPressed(int iKey)
{
	if (!IsOpen())
		return false;

	return BaseClass::CharPressed(iKey);
}

void CGameWindow::OpenChat()
{
	if (!Get())
		return;

	CChatBox* pChat = GetChatBox();

	if (!pChat->IsFloating())
		return;

	pChat->Layout();
	pChat->SetVisible(true);

	CApplication::Get()->GetConsole()->SetRenderBackground(false);

	glgui::CRootPanel::Get()->MoveToTop(pChat);
}

void CGameWindow::CloseChat()
{
	if (!Get())
		return;

	CChatBox* pChat = GetChatBox();

	if (!pChat->IsFloating())
		return;

	pChat->SetVisible(false);

	CApplication::Get()->GetConsole()->SetRenderBackground(true);
}

void CGameWindow::ToggleChat()
{
	if (!Get())
		return;

	CChatBox* pChat = GetChatBox();
	if (IsChatOpen())
		CloseChat();
	else
		OpenChat();
}

bool CGameWindow::IsChatOpen()
{
	if (!Get())
		return false;

	CChatBox* pChat = GetChatBox();
	return pChat->IsOpen();
}

void CGameWindow::PrintChat(tstring sText)
{
	if (!Get())
	{
		puts(sText.c_str());
		return;
	}

	CChatBox* pChat = GetChatBox();
	pChat->PrintChat(sText);
}

CChatBox* CGameWindow::GetChatBox()
{
	if (m_pChatBox == NULL)
	{
		m_pChatBox = new CChatBox(true);
		m_pChatBox->SetVisible(false);
	}

	return m_pChatBox;
}
