#ifndef DT_MENU_H
#define DT_MENU_H

#include <common.h>

#include <glgui/glgui.h>

#include <game/digitanks/digitanksgame.h>

class CDockPanel : public glgui::CPanel
{
	DECLARE_CLASS(CDockPanel, glgui::CPanel);

public:
									CDockPanel();

public:
	virtual void					Destructor();
	virtual void					Delete() { delete this; };

	virtual void					Layout();
	virtual void					Paint(int x, int y, int w, int h);

	virtual void					SetDockedPanel(glgui::CPanel* pDock);

	void							SetBGColor(Color clrBG) { m_clrBackground = clrBG; };

protected:
	glgui::CPanel*					m_pDockedPanel;

	Color							m_clrBackground;
};

class CCampaignPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CCampaignPanel, glgui::CPanel);

public:
									CCampaignPanel();

public:
	virtual void					Delete() { delete this; };

	virtual void					Layout();

	EVENT_CALLBACK(CCampaignPanel,	NewCampaign);
	EVENT_CALLBACK(CCampaignPanel,	NewCampaignHint);
	EVENT_CALLBACK(CCampaignPanel,	ContinueCampaign);
	EVENT_CALLBACK(CCampaignPanel,	ContinueCampaignHint);

protected:
	glgui::CButton*					m_pNewCampaign;
	glgui::CButton*					m_pContinueCampaign;
};

class CGamesPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CGamesPanel, glgui::CPanel);

public:
									CGamesPanel();

public:
	virtual void					Delete() { delete this; };

	virtual void					Layout();

	EVENT_CALLBACK(CGamesPanel,	Artillery);
	EVENT_CALLBACK(CGamesPanel,	Strategy);
	EVENT_CALLBACK(CGamesPanel,	Load);

	EVENT_CALLBACK(CGamesPanel,	ArtilleryHint);
	EVENT_CALLBACK(CGamesPanel,	StrategyHint);

protected:
	glgui::CButton*					m_pArtillery;
	glgui::CButton*					m_pStrategy;

	glgui::CButton*					m_pLoad;

	CDockPanel*						m_pDockPanel;
};

class CMultiplayerPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CMultiplayerPanel, glgui::CPanel);

public:
									CMultiplayerPanel();

public:
	virtual void					Delete() { delete this; };

	virtual void					Layout();

	EVENT_CALLBACK(CMultiplayerPanel,	Connect);
	EVENT_CALLBACK(CMultiplayerPanel,	CreateArtilleryLobby);
	EVENT_CALLBACK(CMultiplayerPanel,	CreateStrategyLobby);
	EVENT_CALLBACK(CMultiplayerPanel,	Load);

	EVENT_CALLBACK(CMultiplayerPanel,	ClientHint);
	EVENT_CALLBACK(CMultiplayerPanel,	CreateArtilleryHint);
	EVENT_CALLBACK(CMultiplayerPanel,	CreateStrategyHint);

protected:
	glgui::CButton*					m_pConnect;
	glgui::CButton*					m_pCreateArtilleryLobby;
	glgui::CButton*					m_pCreateStrategyLobby;

	CDockPanel*						m_pDockPanel;
};

class CCreateLobbyPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CCreateLobbyPanel, glgui::CPanel);

public:
									CCreateLobbyPanel(gametype_t eGameType);

public:
	virtual void					Delete() { delete this; };

	virtual void					Layout();

	EVENT_CALLBACK(CCreateLobbyPanel,	CreateHotseatLobby);
	EVENT_CALLBACK(CCreateLobbyPanel,	CreateOnlineLobby);
	EVENT_CALLBACK(CCreateLobbyPanel,	CreateHotseatHint);
	EVENT_CALLBACK(CCreateLobbyPanel,	CreateOnlineHint);

protected:
	gametype_t						m_eGameType;

	glgui::CButton*					m_pCreateHotseatLobby;
	glgui::CButton*					m_pCreateOnlineLobby;
};

class CConnectPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CConnectPanel, glgui::CPanel);

public:
									CConnectPanel();

public:
	virtual void					Delete() { delete this; };

	virtual void					Layout();

	EVENT_CALLBACK(CConnectPanel,	Connect);

protected:
	glgui::CLabel*					m_pHostnameLabel;
	glgui::CTextField*				m_pHostname;

	glgui::CButton*					m_pConnect;
};

class CArtilleryGamePanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CArtilleryGamePanel, glgui::CPanel);

public:
									CArtilleryGamePanel(bool bMultiplayer = false);
	virtual 						~CArtilleryGamePanel();

public:
	virtual void					Delete() { delete this; };

	virtual void					Layout();
	virtual void					Paint(int x, int y, int w, int h);

	EVENT_CALLBACK(CArtilleryGamePanel,	BeginGame);
	EVENT_CALLBACK(CArtilleryGamePanel,	UpdateLayout);
	EVENT_CALLBACK(CArtilleryGamePanel,	LevelChosen);
	EVENT_CALLBACK(CArtilleryGamePanel,	LevelPreview);
	EVENT_CALLBACK(CArtilleryGamePanel,	LevelRevertPreview);
	EVENT_CALLBACK(CArtilleryGamePanel,	TanksSelected);
	EVENT_CALLBACK(CArtilleryGamePanel,	TerrainSelected);

	void							PreviewLevel(size_t iLevel);

protected:
	glgui::CTree*					m_pLevels;
	size_t							m_iLevelSelected;

	glgui::CLabel*					m_pLevelDescription;

	size_t							m_iLevelPreview;

	glgui::CScrollSelector<int>*	m_pDifficulty;
	glgui::CLabel*					m_pDifficultyLabel;

	glgui::CScrollSelector<int>*	m_pBotPlayers;
	glgui::CLabel*					m_pBotPlayersLabel;

	glgui::CScrollSelector<int>*	m_pTanks;
	glgui::CLabel*					m_pTanksLabel;

	glgui::CScrollSelector<float>*	m_pTerrain;
	glgui::CLabel*					m_pTerrainLabel;

	glgui::CButton*					m_pBeginGame;
};

class CStrategyGamePanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CStrategyGamePanel, glgui::CPanel);

public:
									CStrategyGamePanel(bool bMultiplayer = false);
									~CStrategyGamePanel();

public:
	virtual void					Delete() { delete this; };

	virtual void					Layout();
	virtual void					Paint(int x, int y, int w, int h);

	EVENT_CALLBACK(CStrategyGamePanel,	BeginGame);
	EVENT_CALLBACK(CStrategyGamePanel,	UpdateLayout);
	EVENT_CALLBACK(CStrategyGamePanel,	LevelChosen);
	EVENT_CALLBACK(CStrategyGamePanel,	LevelPreview);
	EVENT_CALLBACK(CStrategyGamePanel,	LevelRevertPreview);

	void							PreviewLevel(size_t iLevel);

protected:
	glgui::CTree*					m_pLevels;
	size_t							m_iLevelSelected;

	glgui::CLabel*					m_pLevelDescription;

	size_t							m_iLevelPreview;

	glgui::CScrollSelector<int>*	m_pDifficulty;
	glgui::CLabel*					m_pDifficultyLabel;

	glgui::CScrollSelector<int>*	m_pBotPlayers;
	glgui::CLabel*					m_pBotPlayersLabel;

	glgui::CButton*					m_pBeginGame;
};

class COptionsPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(COptionsPanel, glgui::CPanel);

public:
									COptionsPanel();

public:
	virtual void					Delete() { delete this; };

	virtual void					Layout();
	virtual void					Paint(int x, int y, int w, int h);

	void							SetStandalone(bool bStandalone) { m_bStandalone = bStandalone; }

	EVENT_CALLBACK(COptionsPanel,	NewNickname);
	EVENT_CALLBACK(COptionsPanel,	SoundVolumeChanged);
	EVENT_CALLBACK(COptionsPanel,	MusicVolumeChanged);
	EVENT_CALLBACK(COptionsPanel,	VideoModeChosen);
	EVENT_CALLBACK(COptionsPanel,	WindowedChanged);
	EVENT_CALLBACK(COptionsPanel,	FramebuffersChanged);
	EVENT_CALLBACK(COptionsPanel,	ShadersChanged);
	EVENT_CALLBACK(COptionsPanel,	ConstrainChanged);
	EVENT_CALLBACK(COptionsPanel,	ContextualChanged);
	EVENT_CALLBACK(COptionsPanel,	ReverseSpacebarChanged);
	EVENT_CALLBACK(COptionsPanel,	Close);

protected:
	glgui::CLabel*					m_pNicknameLabel;
	glgui::CTextField*				m_pNickname;

	glgui::CScrollSelector<float>*	m_pSoundVolume;
	glgui::CLabel*					m_pSoundVolumeLabel;

	glgui::CScrollSelector<float>*	m_pMusicVolume;
	glgui::CLabel*					m_pMusicVolumeLabel;

	glgui::CMenu*					m_pVideoModes;

	glgui::CCheckBox*				m_pWindowed;
	glgui::CLabel*					m_pWindowedLabel;

	glgui::CCheckBox*				m_pFramebuffers;
	glgui::CLabel*					m_pFramebuffersLabel;

	glgui::CCheckBox*				m_pShaders;
	glgui::CLabel*					m_pShadersLabel;

	glgui::CCheckBox*				m_pConstrain;
	glgui::CLabel*					m_pConstrainLabel;

	glgui::CCheckBox*				m_pContextual;
	glgui::CLabel*					m_pContextualLabel;

	glgui::CCheckBox*				m_pReverseSpacebar;
	glgui::CLabel*					m_pReverseSpacebarLabel;

	glgui::CLabel*					m_pVideoChangedNotice;

	bool							m_bStandalone;

	glgui::CButton*					m_pClose;
};

class CMainMenu : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CMainMenu, glgui::CPanel);

public:
									CMainMenu();

public:
	virtual void					Delete() { delete this; };

	virtual void					Layout();
	virtual void					Think();
	virtual void					Paint(int x, int y, int w, int h);

	virtual void					SetVisible(bool bVisible);

	EVENT_CALLBACK(CMainMenu,		OpenCampaignPanel);
	EVENT_CALLBACK(CMainMenu,		OpenGamesPanel);
	EVENT_CALLBACK(CMainMenu,		OpenMultiplayerPanel);
	EVENT_CALLBACK(CMainMenu,		OpenOptionsPanel);
	EVENT_CALLBACK(CMainMenu,		Quit);
	EVENT_CALLBACK(CMainMenu,		Credits);

	CDockPanel*						GetDockPanel();

	virtual void					SetHint(const tstring &sHint);

protected:
	glgui::CButton*					m_pCampaign;
	glgui::CButton*					m_pPlay;
	glgui::CButton*					m_pMultiplayer;
	glgui::CButton*					m_pOptions;
	glgui::CButton*					m_pQuit;

	glgui::CLabel*					m_pHint;

	glgui::CButton*					m_pShowCredits;
	glgui::CLabel*					m_pCredits;

	CDockPanel*						m_pDockPanel;

	float							m_flCreditsRoll;

	glgui::CLabel*					m_pVersion;
};

#endif
