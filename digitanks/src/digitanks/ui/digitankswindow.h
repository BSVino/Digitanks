#ifndef DT_DIGITANKSWINDOW_H
#define DT_DIGITANKSWINDOW_H

#include <vector.h>
#include <color.h>
#include <worklistener.h>

#include <tengine/ui/gamewindow.h>

#include <digitanksgame.h>

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

class CDigitanksWindow : public CGameWindow, public IWorkListener
{
	DECLARE_CLASS(CDigitanksWindow, CGameWindow);

public:
								CDigitanksWindow(int argc, char** argv);
	virtual 					~CDigitanksWindow();

public:
	void						OpenWindow();

	virtual tstring				WindowTitle() { return "Digitanks!"; }
	virtual tstring				AppDirectory() { return "Digitanks"; }

	void						InitUI();

	void						SetServerType(servertype_t eServerType) { m_eServerType = eServerType; };

	void						RenderLoading();
	void						RenderMouseCursor();

	void						CreateGame(gametype_t eGameType = GAMETYPE_EMPTY);
	void						DestroyGame();

	void						NewCampaign();
	void						RestartCampaignLevel();
	void						NextCampaignLevel();
	void						ContinueCampaign();
	class CCampaignData*		GetCampaignData() { return m_pCampaign; };

	void						RestartLevel();

	void						Restart(gametype_t eRestartAction);

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

	virtual bool				KeyPress(int c);
	virtual void				KeyRelease(int c);
	virtual void				CharPress(int c);
	virtual void				CharRelease(int c);

	bool						GetBoxSelection(size_t& iX, size_t& iY, size_t& iX2, size_t& iY2);
	bool						IsMouseDragging();

	int							GetMouseCurrentX() { return m_iMouseCurrentX; };
	int							GetMouseCurrentY() { return m_iMouseCurrentY; };

	void						SetConfigWindowDimensions(int iWidth, int iHeight) { m_iCfgWidth = iWidth; m_iCfgHeight = iHeight; };
	void						SetConfigFullscreen(bool bFullscreen) { m_bCfgFullscreen = bFullscreen; };
	void						SetConstrainMouse(bool bConstrain) { m_bConstrainMouse = bConstrain; };
	bool						ShouldConstrainMouse();
	bool						WantsConstrainMouse() { return m_bConstrainMouse; };

	void						SetContextualCommands(bool b) { m_bContextualCommands = b; };
	void						SetContextualCommandsOverride(bool b) { m_bContextualCommandsOverride = b; };
	bool						WantsContextualCommands() { return m_bContextualCommands; };
	bool						ShouldUseContextualCommands() { return m_bContextualCommands && !m_bContextualCommandsOverride; };

	void						SetReverseSpacebar(bool b) { m_bReverseSpacebar = b; };
	bool						ShouldReverseSpacebar() { return m_bReverseSpacebar; };

	void						SetWantsFramebuffers(bool bWantsFramebuffers) { m_bWantsFramebuffers = bWantsFramebuffers; }
	bool						WantsFramebuffers() { return m_bWantsFramebuffers; }

	void						SetWantsShaders(bool bWantsShaders) { m_bWantsShaders = bWantsShaders; }
	bool						WantsShaders() { return m_bWantsShaders; }

	void						SetPlayerNickname(tstring sNickname) { m_sNickname = sNickname; }
	tstring						GetPlayerNickname() { return m_sNickname; }

	bool						GetMouseGridPosition(Vector& vecPoint, CBaseEntity** pHit = NULL, int iCollisionGroup = 0);

	void						GameOver(bool bPlayerWon);

	virtual void				OnClientDisconnect(int iClient);

	void						CloseApplication();

	void						SaveConfig();

	class CMainMenu*			GetMainMenu() { return m_pMainMenu; };
	class CDigitanksMenu*		GetMenu() { return m_pMenu; };
	class CHUD*					GetHUD() { return m_pHUD; };
	class CInstructor*			GetInstructor();
	class CVictoryPanel*		GetVictoryPanel() { return m_pVictory; };
	class CStoryPanel*			GetStoryPanel() { return m_pStory; };
	class CLobbyPanel*			GetLobbyPanel() { return m_pLobby; };

	float						GetSoundVolume() { return m_flSoundVolume; };
	void						SetSoundVolume(float flSoundVolume);

	float						GetMusicVolume() { return m_flMusicVolume; };
	void						SetMusicVolume(float flMusicVolume);

	int							GetInstallID() { return m_iInstallID; };

	size_t						GetLunarWorkshopLogo() { return m_iLunarWorkshop; }

	virtual class CChatBox*		GetChatBox();

	// IWorkListener
	virtual void				BeginProgress();
	virtual void				SetAction(const tstring& sAction, size_t iTotalProgress);
	virtual void				WorkProgress(size_t iProgress, bool bForceDraw = false);
	virtual void				EndProgress();

protected:
	int							m_iMouseLastX;
	int							m_iMouseLastY;

	size_t						m_iLoading;

	class CMainMenu*			m_pMainMenu;
	class CDigitanksMenu*		m_pMenu;
	class CVictoryPanel*		m_pVictory;
	class CPurchasePanel*		m_pPurchase;
	class CStoryPanel*			m_pStory;
	class CLobbyPanel*			m_pLobby;

	servertype_t				m_eServerType;

	gametype_t					m_eRestartAction;

	class CHUD*					m_pHUD;

	class CInstructor*			m_pInstructor;

	class CCampaignData*		m_pCampaign;

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
	bool						m_bContextualCommands;
	bool						m_bContextualCommandsOverride;
	bool						m_bReverseSpacebar;
	bool						m_bWantsFramebuffers;
	bool						m_bWantsShaders;
	tstring						m_sNickname;

	float						m_flSoundVolume;
	float						m_flMusicVolume;

	int							m_iInstallID;

	size_t						m_iLunarWorkshop;

	tstring						m_sAction;
	size_t						m_iTotalProgress;
	size_t						m_iProgress;
};

inline CDigitanksWindow* DigitanksWindow()
{
	return dynamic_cast<CDigitanksWindow*>(CDigitanksWindow::Get());
}

#endif
