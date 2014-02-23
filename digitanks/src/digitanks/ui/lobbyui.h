#ifndef DT_LOBBYUI_H
#define DT_LOBBYUI_H

#include <glgui/panel.h>
#include <network/network.h>

class CPlayerPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CPlayerPanel, glgui::CPanel);

public:
									CPlayerPanel();

public:
	virtual void					Delete() { delete this; };

	virtual void					Layout();
	virtual void					Paint(float x, float y, float w, float h);

	void							SetPlayer(size_t iClient);

	EVENT_CALLBACK(CPlayerPanel,	Kick);
	EVENT_CALLBACK(CPlayerPanel,	ColorChosen);

protected:
	size_t							m_iLobbyPlayer;

	glgui::CMenu*					m_pColor;
	bool							m_bRandomColor;
	size_t							m_iColor;
	tvector<size_t>					m_aiAvailableColors;

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
	virtual void					Think();
	virtual void					Paint(float x, float y, float w, float h);

	void							CreateLobby(bool bOnline);
	void							ConnectToLocalLobby(const tstring& sHost);

	void							UpdatePlayerInfo();

	class CChatBox*					GetChat() { return m_pChatBox; }

	EVENT_CALLBACK(CLobbyPanel,		LeaveLobby);
	EVENT_CALLBACK(CLobbyPanel,		PlayerReady);
	EVENT_CALLBACK(CLobbyPanel,		AddPlayer);
	EVENT_CALLBACK(CLobbyPanel,		AddBot);

	static void						LobbyUpdateCallback(int iConnection, INetworkListener*, class CNetworkParameters*);
	void							LobbyUpdate();

	static void						LobbyJoinCallback(int iConnection, INetworkListener*, class CNetworkParameters*);
	static void						LobbyLeaveCallback(int iConnection, INetworkListener*, class CNetworkParameters*);

	static void						BeginGameCallback(int iConnection, INetworkListener*, class CNetworkParameters*);

protected:
	tstring					m_sHost;
	bool							m_bOnline;

	glgui::CLabel*					m_pLobbyName;
	glgui::CLabel*					m_pPlayerList;
	glgui::CButton*					m_pAddPlayer;
	glgui::CButton*					m_pAddBot;

	class CDockPanel*				m_pDockPanel;
	class CChatBox*					m_pChatBox;

	glgui::CButton*					m_pLeave;
	glgui::CButton*					m_pReady;

	size_t							m_iLobby;

	tvector<CPlayerPanel*>			m_apPlayerPanels;

	bool							m_bLayout;
};

#endif
