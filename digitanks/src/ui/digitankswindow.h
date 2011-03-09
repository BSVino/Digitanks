#ifndef DT_DIGITANKSWINDOW_H
#define DT_DIGITANKSWINDOW_H

#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <vector.h>
#include <color.h>

#include <game/digitanks/digitanksgame.h>
#include <tinker/application.h>

typedef enum
{
	MOUSECURSOR_NONE = 0,
	MOUSECURSOR_BUILD,
	MOUSECURSOR_BUILDINVALID,
	MOUSECURSOR_SELECT,
	MOUSECURSOR_MOVE,
	MOUSECURSOR_MOVEAUTO,
	MOUSECURSOR_ROTATE,
	MOUSECURSOR_AIM,
	MOUSECURSOR_AIMENEMY,
	MOUSECURSOR_AIMINVALID,
} mousecursor_t;

class CDigitanksWindow : public CApplication
{
	DECLARE_CLASS(CDigitanksWindow, CApplication);

public:
								CDigitanksWindow(int argc, char** argv);
	virtual 					~CDigitanksWindow();

public:
	void						OpenWindow();

	virtual eastl::string		WindowTitle() { return "Digitanks!"; }
	virtual eastl::string16		AppDirectory() { return L"Digitanks"; }

	void						InitUI();

	void						SetServerType(servertype_t eServerType) { m_eServerType = eServerType; };
	void						SetConnectHost(const eastl::string16 sHost) { m_sConnectHost = sHost; };

	void						RenderLoading();
	void						RenderMouseCursor();

	void						CreateGame(gametype_t eGameType);
	void						DestroyGame();

	void						Run();	// Doesn't return

	void						ConstrainMouse();

	void						Layout();

	virtual void				Render();
	virtual int					WindowClose();
	virtual void				WindowResize(int x, int y);
	virtual void				MouseMotion(int x, int y);
	virtual void				MouseInput(int iButton, int iState);
	virtual void				MouseWheel(int iState);

	void						SetMouseCursor(mousecursor_t eCursor) { m_eMouseCursor = eCursor; }

	virtual void				KeyPress(int c);
	virtual void				KeyRelease(int c);
	virtual void				CharPress(int c);
	virtual void				CharRelease(int c);

	bool						GetBoxSelection(size_t& iX, size_t& iY, size_t& iX2, size_t& iY2);
	bool						IsMouseDragging();

	int							GetMouseCurrentX() { return m_iMouseCurrentX; };
	int							GetMouseCurrentY() { return m_iMouseCurrentX; };

	void						SetConfigWindowDimensions(int iWidth, int iHeight) { m_iCfgWidth = iWidth; m_iCfgHeight = iHeight; };
	void						SetConfigFullscreen(bool bFullscreen) { m_bCfgFullscreen = bFullscreen; };
	void						SetConstrainMouse(bool bConstrain) { m_bConstrainMouse = bConstrain; };
	bool						ShouldConstrainMouse();

	void						SetWantsFramebuffers(bool bWantsFramebuffers) { m_bWantsFramebuffers = bWantsFramebuffers; }
	bool						WantsFramebuffers() { return m_bWantsFramebuffers; }

	void						SetWantsShaders(bool bWantsShaders) { m_bWantsShaders = bWantsShaders; }
	bool						WantsShaders() { return m_bWantsShaders; }

	void						SetPlayerNickname(eastl::string16 sNickname) { m_sNickname = sNickname; }
	eastl::string16				GetPlayerNickname() { return m_sNickname; }

	bool						GetMouseGridPosition(Vector& vecPoint, CBaseEntity** pHit = NULL, int iCollisionGroup = 0);

	void						GameOver(bool bPlayerWon);

	void						CloseApplication();

	void						SaveConfig();

	class CMainMenu*			GetMainMenu() { return m_pMainMenu; };
	class CDigitanksMenu*		GetMenu() { return m_pMenu; };
	class CGameServer*			GetGameServer() { return m_pGameServer; };
	class CHUD*					GetHUD() { return m_pHUD; };
	class CInstructor*			GetInstructor();
	class CVictoryPanel*		GetVictoryPanel() { return m_pVictory; };
	class CStoryPanel*			GetStoryPanel() { return m_pStory; };

	float						GetSoundVolume() { return m_flSoundVolume; };
	void						SetSoundVolume(float flSoundVolume);

	float						GetMusicVolume() { return m_flMusicVolume; };
	void						SetMusicVolume(float flMusicVolume);

	int							GetInstallID() { return m_iInstallID; };

	size_t						GetLunarWorkshopLogo() { return m_iLunarWorkshop; }

protected:
	int							m_iMouseLastX;
	int							m_iMouseLastY;

	size_t						m_iLoading;

	class CMainMenu*			m_pMainMenu;
	class CDigitanksMenu*		m_pMenu;
	class CVictoryPanel*		m_pVictory;
	class CPurchasePanel*		m_pPurchase;
	class CStoryPanel*			m_pStory;

	servertype_t				m_eServerType;
	eastl::string16				m_sConnectHost;

	class CGameServer*			m_pGameServer;

	class CHUD*					m_pHUD;

	class CInstructor*			m_pInstructor;

	bool						m_bBoxSelect;
	int							m_iMouseInitialX;
	int							m_iMouseInitialY;
	int							m_iMouseCurrentX;
	int							m_iMouseCurrentY;

	int							m_iMouseMoved;

	bool						m_bMouseDownInGUI;

	float						m_flLastClick;

	size_t						m_iCursors;
	mousecursor_t				m_eMouseCursor;

	bool						m_bCfgFullscreen;
	int							m_iCfgWidth;
	int							m_iCfgHeight;
	bool						m_bConstrainMouse;
	bool						m_bWantsFramebuffers;
	bool						m_bWantsShaders;
	eastl::string16				m_sNickname;

	float						m_flSoundVolume;
	float						m_flMusicVolume;

	int							m_iInstallID;

	size_t						m_iLunarWorkshop;
};

inline CDigitanksWindow* DigitanksWindow()
{
	return dynamic_cast<CDigitanksWindow*>(CDigitanksWindow::Get());
}

#endif