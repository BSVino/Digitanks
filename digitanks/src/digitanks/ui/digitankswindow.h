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

	virtual class CRenderer*    CreateRenderer();

	void						InitUI();

	void						SetServerType(servertype_t eServerType) { m_eServerType = eServerType; };

	void						RenderLoading();
	void						RenderMouseCursor();

	void						NewCampaign();
	void						RestartCampaignLevel();
	void						NextCampaignLevel();
	void						ContinueCampaign();
	class CCampaignData*		GetCampaignData() { return m_pCampaign; };

	void						RestartLevel();

	void						Restart(gametype_t eRestartAction);

	void						PreFrame();

	void						ConstrainMouse();

	void						Layout();

	virtual void				Render();
	virtual int					WindowClose();
	virtual void				WindowResize(int x, int y);

	void						SetMouseCursor(mousecursor_t eCursor) { m_eMouseCursor = eCursor; }

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

	void						SetPlayerNickname(tstring sNickname) { m_sNickname = sNickname; }
	tstring						GetPlayerNickname() { return m_sNickname; }

	bool						GetMouseGridPosition(Vector& vecPoint, CBaseEntity** pHit = NULL, bool bTerrainOnly = false);
	bool						GetGridPosition(const Vector2D& vecScreen, Vector& vecPoint, CBaseEntity** pHit = NULL, bool bTerrainOnly = false);

	void						GameOver(bool bPlayerWon);

	virtual void				OnClientDisconnect(int iClient);

	void						CloseApplication();

	void						SaveConfig();

	glgui::CControl<class CMainMenu>			GetMainMenu() { return m_pMainMenu; };
	glgui::CControl<class CDigitanksMenu>		GetMenu() { return m_pMenu; };
	class CHUD*					GetHUD();
	class CInstructor*			GetInstructor();
	glgui::CControl<class CVictoryPanel>		GetVictoryPanel() { return m_pVictory; };
	glgui::CControl<class CStoryPanel>			GetStoryPanel() { return m_pStory; };
	glgui::CControl<class CLobbyPanel>			GetLobbyPanel() { return m_pLobby; };

	float						GetSoundVolume() { return m_flSoundVolume; };
	void						SetSoundVolume(float flSoundVolume);

	float						GetMusicVolume() { return m_flMusicVolume; };
	void						SetMusicVolume(float flMusicVolume);

	int							GetInstallID() { return m_iInstallID; };

	CMaterialHandle             GetLunarWorkshopLogo() { return m_hLunarWorkshop; }

	virtual class CChatBox*		GetChatBox();

	// IWorkListener
	virtual void				BeginProgress();
	virtual void				SetAction(const tstring& sAction, size_t iTotalProgress);
	virtual void				WorkProgress(size_t iProgress, bool bForceDraw = false);
	virtual void				EndProgress();

protected:
	int							m_iMouseLastX;
	int							m_iMouseLastY;

	CMaterialHandle				m_hLoading;

	glgui::CControl<class CMainMenu>			m_pMainMenu;
	glgui::CControl<class CDigitanksMenu>		m_pMenu;
	glgui::CControl<class CVictoryPanel>		m_pVictory;
	glgui::CControl<class CStoryPanel>			m_pStory;
	glgui::CControl<class CLobbyPanel>			m_pLobby;

	servertype_t				m_eServerType;

	gametype_t					m_eRestartAction;

	class CCampaignData*		m_pCampaign;

	CMaterialHandle				m_hCursors;
	mousecursor_t				m_eMouseCursor;

	bool						m_bCfgFullscreen;
	int							m_iCfgWidth;
	int							m_iCfgHeight;
	bool						m_bConstrainMouse;
	bool						m_bContextualCommands;
	bool						m_bContextualCommandsOverride;
	bool						m_bReverseSpacebar;
	tstring						m_sNickname;

	float						m_flSoundVolume;
	float						m_flMusicVolume;

	int							m_iInstallID;

	CMaterialHandle				m_hLunarWorkshop;

	tstring						m_sAction;
	size_t						m_iTotalProgress;
	size_t						m_iProgress;
};

inline CDigitanksWindow* DigitanksWindow()
{
	return dynamic_cast<CDigitanksWindow*>(CDigitanksWindow::Get());
}

#endif
