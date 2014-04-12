#ifndef DT_DIGITANKSGAME_H
#define DT_DIGITANKSGAME_H

#include <tvector.h>

#include <common.h>

#include <game/entities/game.h>

#include "units/digitank.h"
#include "terrain.h"
#include "digitanksplayer.h"
#include "dt_common.h"
#include "updates.h"
#include "instructor_entity.h"

typedef enum
{
	GAMETYPE_EMPTY = 0,
	GAMETYPE_FROM_CVAR,
	GAMETYPE_FROM_LOBBY,
	GAMETYPE_ARTILLERY,
	GAMETYPE_STANDARD,
	GAMETYPE_MENU,
	GAMETYPE_CAMPAIGN,
} gametype_t;

typedef struct
{
	size_t			iShells;
	double			flNextShell;
	Vector			vecLocation;
} airstrike_t;

typedef enum
{
	DISABLE_NOTHING     = 0,
	DISABLE_VIEW_MOVE   = (1<<0),
	DISABLE_VIEW_ROTATE = (1<<1),
	DISABLE_ENTER       = (1<<2),
	DISABLE_BUFFER      = (1<<3),
	DISABLE_PSU         = (1<<4),
	DISABLE_LOADERS     = (1<<5),
	DISABLE_SELECT      = (1<<6),
	DISABLE_HOWTOPLAY   = (1<<7),
} disable_t;

class IDigitanksGameListener
{
public:
	virtual void			GameStart()=0;
	virtual void			GameOver(bool bPlayerWon)=0;

	virtual void			NewCurrentTeam()=0;
	virtual void			NewCurrentSelection()=0;

	virtual void			OnTakeShieldDamage(class CDigitank* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bShieldOnly)=0;
	virtual void			OnTakeDamage(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled)=0;
	virtual void			OnDisabled(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor)=0;
	virtual void			OnMiss(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor)=0;
	virtual void			OnCritical(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor)=0;

	virtual void			TankSpeak(class CBaseEntity* pTank, const tstring& sSpeech)=0;

	virtual void			SetHUDActive(bool bActive)=0;
};

class CDigitanksGame : public CGame
{
	REGISTER_ENTITY_CLASS(CDigitanksGame, CGame);

public:
	CDigitanksGame();

public:
	virtual void 			Precache();
	virtual void 			Spawn();

	void					SetListener(IDigitanksGameListener* pListener) { m_pListener = pListener; };
	IDigitanksGameListener*	GetListener() { return m_pListener; };

	tstring			GetObjective() { return m_sObjective; }

	virtual void			RegisterNetworkFunctions();

	virtual void			ClientUpdate(int iClient);

	void					SetupGame(tstring sType);
	void					SetupEntities();
	NET_CALLBACK(CDigitanksGame, SetupEntities);

	static tvector<class CLevel*> GetLevels(gametype_t eGameType);
	static size_t			GetNumLevels(gametype_t eGameType);
	static class CDigitanksLevel* GetLevel(gametype_t eGameType, size_t i);
	static class CDigitanksLevel* GetLevel(tstring sFile);

	void					ReadGameScript(tstring sScript);

	void					ScatterResources();
	void					SetupProps();
	void					ScatterNeutralUnits();
	void					SetupArtillery();
	void					SetupStrategy();
	void					SetupTutorial();
	void					SetupMenuMarch();
	void					SetupCampaign(bool bReload = false);

	void					StartGame();
	virtual void			EnterGame();
	NET_CALLBACK(CDigitanksGame, EnterGame);

	void					StartNewRound();
	void					SetupArtilleryRound();
	bool					HasRounds();

	DECLARE_ENTITY_INPUT(Autosave);
	void					Autosave();

	virtual void			Think();

	void					MoveTanks();
	void					TurnTanks(Vector vecLookAt = Vector());
	void					FireTanks();

	DECLARE_ENTITY_INPUT(CancelAutoMoves);

	void					EndTurn();
	NET_CALLBACK(CDigitanksGame, EndTurn);
	void					StartTurn();
	NET_CALLBACK(CDigitanksGame, StartTurn);

	void					Bot_ExecuteTurn();

	virtual void			OnClientEnterGame(int iClient);

	bool TraceLine(const Vector& v1, const Vector& v2, Vector& vecHit, CBaseEntity** pHit, bool bTerrainOnly = false);

	virtual bool			Explode(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flRadius, float flDamage, CBaseEntity* pIgnore, const CDigitanksPlayer* pTeamIgnore);

	virtual void			OnTakeShieldDamage(class CDigitank* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bShieldOnly);
	virtual void			OnCritical(class CDigitank* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor);

	virtual void			OnTakeDamage(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled);
	virtual void			OnKilled(class CBaseEntity* pEntity);
	void					CheckWinConditions();
	void					GameOver();
	DECLARE_ENTITY_INPUT(PlayerVictory);
	DECLARE_ENTITY_INPUT(PlayerLoss);

	DECLARE_ENTITY_INPUT(TankSelectionMedal);

	virtual void			OnDisabled(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor);
	virtual void			OnMiss(CBaseEntity* pVictim, CBaseEntity* pAttacker, CBaseEntity* pInflictor);

	virtual void			OnDeleted(class CBaseEntity* pEntity);

	virtual void			TankSpeak(class CBaseEntity* pTank, const tstring& sSpeech);

	CDigitanksPlayer*		GetDigitanksPlayer(size_t i) const;

	CDigitanksPlayer*		GetCurrentPlayer() const;
	CSelectable*			GetPrimarySelection();
	CDigitank*				GetPrimarySelectionTank();
	CStructure*				GetPrimarySelectionStructure();

	Vector					GetFormationPosition(Vector vecPosition, Vector vecFacing, size_t iUnitsInFormation, size_t iPosition);

	controlmode_t			GetControlMode();
	void					SetControlMode(controlmode_t eMode);

	aimtype_t				GetAimType();
	void					SetAimType(aimtype_t eAimType);
	void					SetAimTypeByWeapon(weapon_t eWeapon);

	CTerrain*				GetTerrain() { return m_hTerrain; };
	NET_CALLBACK(CDigitanksGame, TerrainData);

	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, Move);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, Turn);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, Fire);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, FireWeapon);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, SetBonusPoints);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, TankPromoted);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, PromoteAttack);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, PromoteDefense);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, PromoteMovement);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, Speak);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, Fortify);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, Sentry);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, SetGoalMovePosition);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, CancelGoalMovePosition);

	NET_CALLBACK_ENTITY(CDigitanksGame, CUpdateGrid, UpdatesData);

	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitanksPlayer, DownloadUpdate);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitanksPlayer, DownloadComplete);

	NET_CALLBACK_ENTITY(CDigitanksGame, CCPU, BeginConstruction);
	NET_CALLBACK_ENTITY(CDigitanksGame, CCPU, BeginRogueProduction);

	NET_CALLBACK_ENTITY(CDigitanksGame, CStructure, BeginStructureConstruction);
	NET_CALLBACK_ENTITY(CDigitanksGame, CStructure, InstallUpdate);
	NET_CALLBACK_ENTITY(CDigitanksGame, CStructure, BeginUpgrade);

	NET_CALLBACK_ENTITY(CDigitanksGame, CSupplier, AddChild);
	NET_CALLBACK_ENTITY(CDigitanksGame, CSupplier, RemoveChild);

	NET_CALLBACK_ENTITY(CDigitanksGame, CLoader, BeginProduction);

	virtual class CDigitanksRenderer*	GetDigitanksRenderer();
	virtual class COverheadCamera*      GetOverheadCamera();
	void                                SetOverheadCamera(class COverheadCamera* pCamera);

	float					GetGravity();

	void					AddProjectileToWaitFor() { m_iWaitingForProjectiles++; };
	size_t					GetNumProjectilesWaitingFor() { return m_iWaitingForProjectiles; };

	void					WeaponSpecialCommand(CDigitanksPlayer* pTeam = NULL);

	void					AddTankAim(Vector vecAim, float flRadius, bool bFocus);
	void					GetTankAims(tvector<Vector>& avecAims, tvector<float>& aflAimRadius, size_t& iFocus);
	void					ClearTankAims();

	void					SetDifficulty(size_t iDifficulty) { m_iDifficulty = iDifficulty; };
	size_t					GetDifficulty() { return m_iDifficulty; };

	void					OnDisplayLesson(tstring sLesson);

	virtual void			ClientEnterGame();

	void					SetRenderFogOfWar(bool bRenderFogOfWar);
	bool					ShouldRenderFogOfWar();

	float					GetVisibilityAtPoint(CDigitanksPlayer* pViewingTeam, Vector vecPoint);

	bool					ShouldShowScores();

	float					GetConstructionCost(unittype_t eUnit);
	float					GetUpgradeCost(unittype_t eUnit);

	bool					CanBuildMiniBuffers();
	bool					CanBuildBuffers();
	bool					CanBuildBatteries();
	bool					CanBuildPSUs();
	bool					CanBuildInfantryLoaders();
	bool					CanBuildTankLoaders();
	bool					CanBuildArtilleryLoaders();

	bool					IsWeaponAllowed(weapon_t eWeapon, const CDigitank* pTank);
	bool					IsInfantryFortifyAllowed();
	void					AllowLaser();

	void					BeginAirstrike(Vector vecLocation);
	float					AirstrikeSize() const { return 50; };

	CUpdateGrid*			GetUpdateGrid() { return m_hUpdates; };

	gametype_t				GetGameType() { return m_eGameType; }
	size_t					GetTurn() { return m_iTurn; }

	size_t					GetDemoTurns() { return 50; }
	float					FogPenetrationDistance() { return 10; }
	float					LavaDamage() { return 20; }
	bool					SoftCraters();

	bool					IsPartyMode() { return m_bPartyMode; };

	static void				UpdateHUD(CNetworkedVariableBase* pVariable);
	static void				UpdateTeamMembers(CNetworkedVariableBase* pVariable);

	// CHEAT!
	void					CompleteProductions();

	CDigitanksPlayer*		GetCurrentLocalDigitanksPlayer();

	void					SetCurrentLevel(tstring sLevel);
	class CDigitanksLevel*	GetCurrentLevel() { return m_pLevel; };

	disable_t				GetDisabledFeatures();
	bool					IsFeatureDisabled(disable_t eFeature);

protected:
	CNetworkedVariable<size_t> m_iCurrentPlayer;

	controlmode_t			m_eControlMode;
	aimtype_t				m_eAimType;

	CNetworkedString		m_sObjective;

	CNetworkedHandle<CTerrain> m_hTerrain;
	CNetworkedHandle<COverheadCamera> m_hOverheadCamera;

	CNetworkedHandle<CInstructorEntity> m_hInstructor;

	IDigitanksGameListener*	m_pListener;

	bool					m_bWaitingForMoving;

	bool					m_bWaitingForProjectiles;
	size_t					m_iWaitingForProjectiles;

	bool					m_bTurnActive;

	size_t					m_iPowerups;

	tvector<Vector>			m_avecTankAims;
	tvector<float>			m_aflTankAimRadius;
	size_t					m_iTankAimFocus;

	CNetworkedVariable<size_t> m_iDifficulty;

	CNetworkedVariable<bool> m_bRenderFogOfWar;

	CNetworkedVariable<gametype_t> m_eGameType;
	CNetworkedVariable<size_t> m_iTurn;

	CNetworkedHandle<CUpdateGrid>	m_hUpdates;

	CNetworkedVariable<bool>	m_bPartyMode;
	double						m_flPartyModeStart;
	double						m_flLastFireworks;

	bool					m_bOverrideAllowLasers;

	tvector<airstrike_t>	m_aAirstrikes;

	CNetworkedArray<float, MAX_UNITS> m_aflConstructionCosts;
	CNetworkedArray<float, MAX_UNITS> m_aflUpgradeCosts;

	double						m_flShowFightSign;
	double						m_flShowArtilleryTutorial;

	double						m_flLastHumanMove;

	class CDigitanksLevel*		m_pLevel;

	CNetworkedVariable<bool>	m_bLevelAllowsBuffers;
	CNetworkedVariable<bool>	m_bLevelAllowsPSUs;
	CNetworkedVariable<bool>	m_bLevelAllowsTankLoaders;
	CNetworkedVariable<bool>	m_bLevelAllowsArtilleryLoaders;
	CNetworkedVariable<bool>	m_bLevelAllowsInfantryLasers;
	CNetworkedVariable<bool>	m_bLevelAllowsInfantryTreeCutters;
	CNetworkedVariable<bool>	m_bLevelAllowsInfantryFortify;
};

inline class CDigitanksGame* DigitanksGame()
{
	CGame* pGame = Game();
	if (!pGame)
		return NULL;

	return static_cast<CDigitanksGame*>(pGame);
}

enum
{
	CG_ENTITY		= (1<<0),
	CG_TERRAIN		= (1<<1),
	CG_PROJECTILE	= (1<<2),
	CG_POWERUP		= (1<<3),
	CG_PROP			= (1<<4),
};

#endif
