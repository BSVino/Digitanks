#ifndef DT_DIGITANKSWINDOW_H
#define DT_DIGITANKSWINDOW_H

#include <string>
#include <vector>
#include <vector.h>
#include <color.h>

#include <game/digitanks/digitanksgame.h>

typedef enum
{
	SERVER_LOCAL,
	SERVER_HOST,
	SERVER_CLIENT,
} servertype_t;

class CDigitanksWindow
{
public:
								CDigitanksWindow(int argc, char** argv);
								~CDigitanksWindow();

public:
	void						InitUI();

	void						SetPlayers(int iPlayers) { m_iPlayers = iPlayers; };
	void						SetTanks(int iTanks) { m_iTanks = iTanks; };
	void						SetServerType(servertype_t eServerType) { m_eServerType = eServerType; };
	void						SetConnectHost(const std::wstring sHost) { m_sConnectHost = sHost; };

	void						CreateGame(gametype_t eGameType);
	void						DestroyGame();

	void						Run();	// Doesn't return

	void						Layout();

	static void					RenderCallback() { Get()->Render(); };
	void						Render();

	static void					WindowResizeCallback(int x, int y) { Get()->WindowResize(x, y); };
	void						WindowResize(int x, int y);

	static void					MouseMotionCallback(int x, int y) { Get()->MouseMotion(x, y); };
	void						MouseMotion(int x, int y);

	static void					MouseInputCallback(int iButton, int iState) { Get()->MouseInput(iButton, iState); };
	void						MouseInput(int iButton, int iState);

	static void					MouseWheelCallback(int iState) { Get()->MouseWheel(iState); };
	void						MouseWheel(int iState);

	static void					KeyEventCallback(int c, int e) { Get()->KeyEvent(c, e); };
	void						KeyEvent(int c, int e);

	static void					CharEventCallback(int c, int e) { Get()->CharEvent(c, e); };
	void						CharEvent(int c, int e);

	void						KeyPress(int c);
	void						KeyRelease(int c);

	void						CharPress(int c);

	static void					SpecialCallback(int k, int x, int y) { Get()->Special(k, x, y); };
	void						Special(int k, int x, int y);

	bool						IsCtrlDown();
	bool						IsAltDown();
	bool						IsShiftDown();

	bool						GetBoxSelection(size_t& iX, size_t& iY, size_t& iX2, size_t& iY2);

	int							GetWindowWidth() { return (int)m_iWindowWidth; };
	int							GetWindowHeight() { return (int)m_iWindowHeight; };

	void						SetConfigWindowDimensions(int iWidth, int iHeight) { m_iCfgWidth = iWidth; m_iCfgHeight = iHeight; };
	void						SetFullscreen(bool bFullscreen) { m_bFullscreen = bFullscreen; };
	bool						IsFullscreen() { return m_bFullscreen; };

	bool						GetMouseGridPosition(Vector& vecPoint, CBaseEntity** pHit = NULL);

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

	bool						HasCommandLineSwitch(const char* pszSwitch);
	const char*					GetCommandLineSwitchValue(const char* pszSwitch);

	float						GetSoundVolume() { return m_flSoundVolume; };
	void						SetSoundVolume(float flSoundVolume);

	float						GetMusicVolume() { return m_flMusicVolume; };
	void						SetMusicVolume(float flMusicVolume);

	static CDigitanksWindow*	Get() { return s_pDigitanksWindow; };

protected:
	int							m_iMouseStartX;
	int							m_iMouseStartY;

	size_t						m_iWindowWidth;
	size_t						m_iWindowHeight;

	class CMainMenu*			m_pMainMenu;
	class CDigitanksMenu*		m_pMenu;
	class CVictoryPanel*		m_pVictory;
	class CPurchasePanel*		m_pPurchase;
	class CStoryPanel*			m_pStory;

	int							m_iPlayers;
	int							m_iTanks;

	servertype_t				m_eServerType;

	std::wstring				m_sConnectHost;

	class CGameServer*			m_pGameServer;

	class CHUD*					m_pHUD;

	class CInstructor*			m_pInstructor;

	static CDigitanksWindow*	s_pDigitanksWindow;

	bool						m_bBoxSelect;
	int							m_iMouseInitialX;
	int							m_iMouseInitialY;
	int							m_iMouseCurrentX;
	int							m_iMouseCurrentY;

	int							m_iMouseMoved;

	bool						m_bMouseDownInGUI;

	std::vector<const char*>	m_apszCommandLine;

	bool						m_bCheatsOn;

	bool						m_bFullscreen;
	int							m_iCfgWidth;
	int							m_iCfgHeight;

	float						m_flSoundVolume;
	float						m_flMusicVolume;
};

#endif