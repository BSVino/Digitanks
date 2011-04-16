#ifndef DT_LOBBYUI_H
#define DT_LOBBYUI_H

#include <glgui/glgui.h>

class CLobbyPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CLobbyPanel, glgui::CPanel);

public:
									CLobbyPanel();

public:
	virtual void					Delete() { delete this; };

	virtual void					Layout();
	virtual void					Paint(int x, int y, int w, int h);

	void							CreateLobby();

	class CChatBox*					GetChat() { return m_pChatBox; }

	EVENT_CALLBACK(CLobbyPanel,		LeaveLobby);

protected:
	class CDockPanel*				m_pDockPanel;
	class CChatBox*					m_pChatBox;

	glgui::CButton*					m_pLeave;
	glgui::CButton*					m_pReady;
};

#endif
