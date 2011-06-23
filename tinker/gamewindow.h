#ifndef TINKER_GAME_WINDOW_H
#define TINKER_GAME_WINDOW_H

#include <tinker/application.h>

#include <common.h>

class CGameWindow : public CApplication
{
	DECLARE_CLASS(CGameWindow, CApplication);

public:
								CGameWindow(int argc, char** argv);
	virtual						~CGameWindow();

public:
	void						OpenWindow();

	void						Run();
	virtual void				PreFrame() {};
	virtual void				RenderLoading() {};

	virtual void				Render();

	virtual void				KeyPress(int c);
	virtual void				KeyRelease(int c);

	virtual void				MouseMotion(int x, int y);
	virtual void				MouseInput(int iButton, int iState);

	class CGameServer*			GetGameServer() { return m_pGameServer; };
	class CRenderer*			GetRenderer() { return m_pRenderer; };

	void						OpenChat();
	void						CloseChat();
	void						ToggleChat();
	bool						IsChatOpen();
	void						PrintChat(tstring sText);
	void						PrintChat(eastl::string sText);
	virtual class CChatBox*		GetChatBox();

protected:
	class CGameServer*			m_pGameServer;
	class CRenderer*			m_pRenderer;

	class CChatBox*				m_pChatBox;
};

inline CGameWindow* GameWindow()
{
	return dynamic_cast<CGameWindow*>(CApplication::Get());
}

#endif
