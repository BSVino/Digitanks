#ifndef DT_LOBBYUI_H
#define DT_LOBBYUI_H

#include <glgui/glgui.h>
#include <network/network.h>

class CPlayerPanel : public glgui::CPanel
{
	DECLARE_CLASS(CPlayerPanel, glgui::CPanel);

public:
									CPlayerPanel();

public:
	virtual void					Delete() { delete this; };

	virtual void					Layout();
	virtual void					Paint(int x, int y, int w, int h);

	void							SetPlayer(size_t iClient);

protected:
	size_t							m_iLobbyPlayer;

	glgui::CLabel*					m_pName;
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

	void							CreateLobby();

	void							UpdatePlayerInfo();

	class CChatBox*					GetChat() { return m_pChatBox; }

	EVENT_CALLBACK(CLobbyPanel,		LeaveLobby);

	static void						LobbyUpdateCallback(INetworkListener*, class CNetworkParameters*);
	void							LobbyUpdate();

protected:
	class CDockPanel*				m_pDockPanel;
	class CChatBox*					m_pChatBox;

	glgui::CButton*					m_pLeave;
	glgui::CButton*					m_pReady;

	size_t							m_iLobby;

	eastl::vector<CPlayerPanel*>	m_apPlayerPanels;
};

#endif
