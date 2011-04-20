#ifndef DT_LOBBYUI_H
#define DT_LOBBYUI_H

#include <glgui/glgui.h>
#include <network/network.h>

class CPlayerPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CPlayerPanel, glgui::CPanel);

public:
									CPlayerPanel();

public:
	virtual void					Delete() { delete this; };

	virtual void					Layout();
	virtual void					Paint(int x, int y, int w, int h);

	void							SetPlayer(size_t iClient);

	EVENT_CALLBACK(CPlayerPanel,	Kick);

protected:
	size_t							m_iLobbyPlayer;

	glgui::CLabel*					m_pName;

	glgui::CButton*					m_pKick;
};

class CInfoPanel : public glgui::CPanel
{
	DECLARE_CLASS(CInfoPanel, glgui::CPanel);

public:
									CInfoPanel();

public:
	virtual void					Delete() { delete this; };

	virtual void					Layout();

protected:
	glgui::CLabel*					m_pLobbyDescription;
};

class CLobbyPanel : public glgui::CPanel, public glgui::IEventListener, public INetworkListener
{
	DECLARE_CLASS(CLobbyPanel, glgui::CPanel);

public:
									CLobbyPanel();

public:
	virtual void					Delete() { delete this; };

	virtual void					Layout();
	virtual void					Paint(int x, int y, int w, int h);

	void							CreateLobby(bool bOnline);
	void							ConnectToLocalLobby(const eastl::string16& sHost);

	void							UpdatePlayerInfo();

	class CChatBox*					GetChat() { return m_pChatBox; }

	EVENT_CALLBACK(CLobbyPanel,		LeaveLobby);
	EVENT_CALLBACK(CLobbyPanel,		PlayerReady);
	EVENT_CALLBACK(CLobbyPanel,		AddBot);

	static void						LobbyUpdateCallback(INetworkListener*, class CNetworkParameters*);
	void							LobbyUpdate();

	static void						BeginGameCallback(INetworkListener*, class CNetworkParameters*);

protected:
	bool							m_bOnline;

	glgui::CLabel*					m_pLobbyName;
	glgui::CLabel*					m_pPlayerList;
	glgui::CButton*					m_pAddBot;

	class CDockPanel*				m_pDockPanel;
	class CChatBox*					m_pChatBox;

	glgui::CButton*					m_pLeave;
	glgui::CButton*					m_pReady;

	size_t							m_iLobby;

	eastl::vector<CPlayerPanel*>	m_apPlayerPanels;
};

#endif
