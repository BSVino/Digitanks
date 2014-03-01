#ifndef DT_MENU_H
#define DT_MENU_H

#include <common.h>

#include <glgui/glgui.h>

#include <digitanksgame.h>

class CDockPanel : public glgui::CPanel
{
	DECLARE_CLASS(CDockPanel, glgui::CPanel);

public:
									CDockPanel();

	virtual void					Layout();
	virtual void					Paint(float x, float y, float w, float h);

	virtual void					SetDockedPanel(glgui::CControl<glgui::CPanel> pDock);

	void							SetBGColor(Color clrBG) { m_clrBackground = clrBG; };

protected:
	glgui::CControl<glgui::CPanel>	m_pDockedPanel;

	Color							m_clrBackground;
};

class CCampaignPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CCampaignPanel, glgui::CPanel);

public:
									CCampaignPanel();

public:
	virtual void					Layout();

	EVENT_CALLBACK(CCampaignPanel,	NewCampaign);
	EVENT_CALLBACK(CCampaignPanel,	NewCampaignHint);
	EVENT_CALLBACK(CCampaignPanel,	ContinueCampaign);
	EVENT_CALLBACK(CCampaignPanel,	ContinueCampaignHint);

protected:
	glgui::CControl<glgui::CButton>					m_pNewCampaign;
	glgui::CControl<glgui::CButton>					m_pContinueCampaign;
};

class CGamesPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CGamesPanel, glgui::CPanel);

public:
									CGamesPanel();

public:
	virtual void					Layout();

	EVENT_CALLBACK(CGamesPanel,	Artillery);
	EVENT_CALLBACK(CGamesPanel,	Strategy);
	EVENT_CALLBACK(CGamesPanel,	Load);
	EVENT_CALLBACK(CGamesPanel,	Open);

	EVENT_CALLBACK(CGamesPanel,	ArtilleryHint);
	EVENT_CALLBACK(CGamesPanel,	StrategyHint);

protected:
	glgui::CControl<glgui::CButton>					m_pArtillery;
	glgui::CControl<glgui::CButton>					m_pStrategy;

	glgui::CControl<glgui::CButton>					m_pLoad;

	glgui::CControl<CDockPanel>						m_pDockPanel;
};

class CMultiplayerPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CMultiplayerPanel, glgui::CPanel);

public:
									CMultiplayerPanel();

public:
	virtual void					Layout();

	EVENT_CALLBACK(CMultiplayerPanel,	Connect);
	EVENT_CALLBACK(CMultiplayerPanel,	CreateArtilleryLobby);
	EVENT_CALLBACK(CMultiplayerPanel,	CreateStrategyLobby);
	EVENT_CALLBACK(CMultiplayerPanel,	Load);
	EVENT_CALLBACK(CMultiplayerPanel,	Open);

	EVENT_CALLBACK(CMultiplayerPanel,	ClientHint);
	EVENT_CALLBACK(CMultiplayerPanel,	CreateArtilleryHint);
	EVENT_CALLBACK(CMultiplayerPanel,	CreateStrategyHint);

protected:
	glgui::CControl<glgui::CButton>					m_pConnect;
	glgui::CControl<glgui::CButton>					m_pCreateArtilleryLobby;
	glgui::CControl<glgui::CButton>					m_pCreateStrategyLobby;

	glgui::CControl<CDockPanel>						m_pDockPanel;
};

class CCreateLobbyPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CCreateLobbyPanel, glgui::CPanel);

public:
									CCreateLobbyPanel(gametype_t eGameType);

public:
	virtual void					Layout();

	EVENT_CALLBACK(CCreateLobbyPanel,	CreateHotseatLobby);
	EVENT_CALLBACK(CCreateLobbyPanel,	CreateOnlineLobby);
	EVENT_CALLBACK(CCreateLobbyPanel,	CreateHotseatHint);
	EVENT_CALLBACK(CCreateLobbyPanel,	CreateOnlineHint);

protected:
	gametype_t						m_eGameType;

	glgui::CControl<glgui::CButton>					m_pCreateHotseatLobby;
	glgui::CControl<glgui::CButton>					m_pCreateOnlineLobby;
};

class CConnectPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CConnectPanel, glgui::CPanel);

public:
									CConnectPanel();

public:
	virtual void					Layout();

	EVENT_CALLBACK(CConnectPanel,	Connect);

protected:
	glgui::CControl<glgui::CLabel>					m_pHostnameLabel;
	glgui::CControl<glgui::CTextField>				m_pHostname;

	glgui::CControl<glgui::CButton>					m_pConnect;
};

class CArtilleryGamePanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CArtilleryGamePanel, glgui::CPanel);

public:
									CArtilleryGamePanel(bool bMultiplayer = false);
	virtual 						~CArtilleryGamePanel();

public:
	virtual void					Layout();
	virtual void					Paint(float x, float y, float w, float h);

	EVENT_CALLBACK(CArtilleryGamePanel,	BeginGame);
	EVENT_CALLBACK(CArtilleryGamePanel,	UpdateLayout);
	EVENT_CALLBACK(CArtilleryGamePanel,	LevelChosen);
	EVENT_CALLBACK(CArtilleryGamePanel,	LevelPreview);
	EVENT_CALLBACK(CArtilleryGamePanel,	LevelRevertPreview);
	EVENT_CALLBACK(CArtilleryGamePanel,	TanksSelected);
	EVENT_CALLBACK(CArtilleryGamePanel,	TerrainSelected);

	void							PreviewLevel(size_t iLevel);

protected:
	glgui::CControl<glgui::CTree>					m_pLevels;
	size_t							m_iLevelSelected;

	glgui::CControl<glgui::CLabel>					m_pLevelDescription;

	CMaterialHandle							m_hLevelPreview;

	glgui::CControl<glgui::CScrollSelector<int>>	m_pDifficulty;
	glgui::CControl<glgui::CLabel>					m_pDifficultyLabel;

	glgui::CControl<glgui::CScrollSelector<int>>	m_pBotPlayers;
	glgui::CControl<glgui::CLabel>					m_pBotPlayersLabel;

	glgui::CControl<glgui::CScrollSelector<int>>	m_pTanks;
	glgui::CControl<glgui::CLabel>					m_pTanksLabel;

	glgui::CControl<glgui::CScrollSelector<float>>	m_pTerrain;
	glgui::CControl<glgui::CLabel>					m_pTerrainLabel;

	glgui::CControl<glgui::CButton>					m_pBeginGame;
};

class CStrategyGamePanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CStrategyGamePanel, glgui::CPanel);

public:
									CStrategyGamePanel(bool bMultiplayer = false);
									~CStrategyGamePanel();

public:
	virtual void					Layout();
	virtual void					Paint(float x, float y, float w, float h);

	EVENT_CALLBACK(CStrategyGamePanel,	BeginGame);
	EVENT_CALLBACK(CStrategyGamePanel,	UpdateLayout);
	EVENT_CALLBACK(CStrategyGamePanel,	LevelChosen);
	EVENT_CALLBACK(CStrategyGamePanel,	LevelPreview);
	EVENT_CALLBACK(CStrategyGamePanel,	LevelRevertPreview);

	void							PreviewLevel(size_t iLevel);

protected:
	glgui::CControl<glgui::CTree>					m_pLevels;
	size_t							m_iLevelSelected;

	glgui::CControl<glgui::CLabel>					m_pLevelDescription;

	CMaterialHandle							m_hLevelPreview;

	glgui::CControl<glgui::CScrollSelector<int>>	m_pDifficulty;
	glgui::CControl<glgui::CLabel>					m_pDifficultyLabel;

	glgui::CControl<glgui::CScrollSelector<int>>	m_pBotPlayers;
	glgui::CControl<glgui::CLabel>					m_pBotPlayersLabel;

	glgui::CControl<glgui::CButton>					m_pBeginGame;
};

class COptionsPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(COptionsPanel, glgui::CPanel);

public:
									COptionsPanel();

public:
	virtual void					Layout();
	virtual void					Paint(float x, float y, float w, float h);

	void							SetStandalone(bool bStandalone) { m_bStandalone = bStandalone; }

	EVENT_CALLBACK(COptionsPanel,	NewNickname);
	EVENT_CALLBACK(COptionsPanel,	SoundVolumeChanged);
	EVENT_CALLBACK(COptionsPanel,	MusicVolumeChanged);
	EVENT_CALLBACK(COptionsPanel,	VideoModeChosen);
	EVENT_CALLBACK(COptionsPanel,	WindowedChanged);
	EVENT_CALLBACK(COptionsPanel,	ConstrainChanged);
	EVENT_CALLBACK(COptionsPanel,	ContextualChanged);
	EVENT_CALLBACK(COptionsPanel,	ReverseSpacebarChanged);
	EVENT_CALLBACK(COptionsPanel,	Close);

protected:
	glgui::CControl<glgui::CLabel>					m_pNicknameLabel;
	glgui::CControl<glgui::CTextField>				m_pNickname;

	glgui::CControl<glgui::CScrollSelector<float>>	m_pSoundVolume;
	glgui::CControl<glgui::CLabel>					m_pSoundVolumeLabel;

	glgui::CControl<glgui::CScrollSelector<float>>	m_pMusicVolume;
	glgui::CControl<glgui::CLabel>					m_pMusicVolumeLabel;

	glgui::CControl<glgui::CMenu>					m_pVideoModes;

	glgui::CControl<glgui::CCheckBox>				m_pWindowed;
	glgui::CControl<glgui::CLabel>					m_pWindowedLabel;

	glgui::CControl<glgui::CCheckBox>				m_pConstrain;
	glgui::CControl<glgui::CLabel>					m_pConstrainLabel;

	glgui::CControl<glgui::CCheckBox>				m_pContextual;
	glgui::CControl<glgui::CLabel>					m_pContextualLabel;

	glgui::CControl<glgui::CCheckBox>				m_pReverseSpacebar;
	glgui::CControl<glgui::CLabel>					m_pReverseSpacebarLabel;

	glgui::CControl<glgui::CLabel>					m_pVideoChangedNotice;

	bool							m_bStandalone;

	glgui::CControl<glgui::CButton>					m_pClose;
};

class CMainMenu : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CMainMenu, glgui::CPanel);

public:
									CMainMenu();

public:
	virtual void					Layout();
	virtual void					Think();
	virtual void					Paint(float x, float y, float w, float h);

	virtual void					SetVisible(bool bVisible);

	EVENT_CALLBACK(CMainMenu,		OpenCampaignPanel);
	EVENT_CALLBACK(CMainMenu,		OpenGamesPanel);
	EVENT_CALLBACK(CMainMenu,		OpenMultiplayerPanel);
	EVENT_CALLBACK(CMainMenu,		OpenOptionsPanel);
	EVENT_CALLBACK(CMainMenu,		Quit);
	EVENT_CALLBACK(CMainMenu,		Credits);

	glgui::CControl<CDockPanel>						GetDockPanel();

	virtual void					SetHint(const tstring &sHint);

protected:
	glgui::CControl<glgui::CButton>					m_pCampaign;
	glgui::CControl<glgui::CButton>					m_pPlay;
	glgui::CControl<glgui::CButton>					m_pMultiplayer;
	glgui::CControl<glgui::CButton>					m_pOptions;
	glgui::CControl<glgui::CButton>					m_pQuit;

	glgui::CControl<glgui::CLabel>					m_pHint;

	glgui::CControl<glgui::CButton>					m_pShowCredits;
	glgui::CControl<glgui::CLabel>					m_pCredits;

	glgui::CControl<CDockPanel>						m_pDockPanel;

	float							m_flCreditsRoll;

	glgui::CControl<glgui::CLabel>					m_pVersion;
};

#endif
