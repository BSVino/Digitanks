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

	void						CreateGame(const tstring& sRequestedGameType);
	void						DestroyGame();
	void						Restart(tstring sGameType);
	void						QueueReloadLevel();
	void                        ReloadLevelNow();

	void						Run();
	virtual void				PreFrame();
	virtual void				PostFrame();
	virtual void				RenderLoading() {};

	virtual void				Render();

	virtual bool				KeyPress(int c);
	virtual void				KeyRelease(int c);

	virtual void				MouseMotion(int x, int y);
	virtual bool				MouseInput(int iButton, tinker_mouse_state_t iState);
	bool						GetLastMouse(int& x, int& y);
	virtual void				MouseWheel(int x, int y);
	virtual bool				JoystickButtonPress(int iJoystick, int c);
	virtual void				JoystickButtonRelease(int iJoystick, int c);
	virtual void				JoystickAxis(int iJoystick, int iAxis, float flValue, float flChange);

	class CGameServer*			GetGameServer() { return m_pGameServer; };
	class CGameRenderer*		GetGameRenderer();

	void						OpenChat();
	void						CloseChat();
	void						ToggleChat();
	bool						IsChatOpen();
	void						PrintChat(tstring sText);
	virtual class CChatBox*		GetChatBox();

	class CInstructor*          GetInstructor() const { return m_pInstructor; }

protected:
	class CGameServer*			m_pGameServer;

	class CChatBox*				m_pChatBox;

	bool						m_bHaveLastMouse;
	int							m_iLastMouseX;
	int							m_iLastMouseY;

	class CHUDViewport*			m_pHUD;

	class CInstructor*          m_pInstructor;

	tstring						m_sRestartGameMode;
	bool                        m_bReloadLevel;
};

inline CGameWindow* GameWindow()
{
	return dynamic_cast<CGameWindow*>(CApplication::Get());
}

#endif
