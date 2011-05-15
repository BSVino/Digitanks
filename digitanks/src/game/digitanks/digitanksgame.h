#ifndef DT_DIGITANKSGAME_H
#define DT_DIGITANKSGAME_H

#include <EASTL/vector.h>

#include "game.h"
#include "units/digitank.h"
#include <common.h>
#include "terrain.h"
#include "digitanksteam.h"
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
	float			flNextShell;
	Vector			vecLocation;
} airstrike_t;

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

	virtual void			TankSpeak(class CBaseEntity* pTank, const eastl::string& sSpeech)=0;

	virtual void			SetHUDActive(bool bActive)=0;
};

class CDigitanksGame : public CGame
{
	REGISTER_ENTITY_CLASS(CDigitanksGame, CGame);

public:
	virtual void 			Precache();
	virtual void 			Spawn();

	void					SetListener(IDigitanksGameListener* pListener) { m_pListener = pListener; };
	IDigitanksGameListener*	GetListener() { return m_pListener; };

	virtual void			RegisterNetworkFunctions();

	virtual void			ClientUpdate(int iClient);

	void					SetupGame(gametype_t eGameType);
	void					SetupEntities();
	NET_CALLBACK(CDigitanksGame, SetupEntities);

	static eastl::vector<class CLevel*> GetLevels(gametype_t eGameType);
	static size_t			GetNumLevels(gametype_t eGameType);
	static class CDigitanksLevel* GetLevel(gametype_t eGameType, size_t i);
	static class CDigitanksLevel* GetLevel(eastl::string16 sFile);

	void					ReadGameScript(eastl::string16 sScript);

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

	virtual void			Think();

	void					MoveTanks();
	void					TurnTanks(Vector vecLookAt = Vector());
	void					FireTanks();

	void					EndTurn();
	NET_CALLBACK(CDigitanksGame, EndTurn);
	void					StartTurn();
	NET_CALLBACK(CDigitanksGame, StartTurn);

	void					Bot_ExecuteTurn();

	virtual bool			Explode(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flRadius, float flDamage, CBaseEntity* pIgnore, CTeam* pTeamIgnore);

	virtual void			OnTakeShieldDamage(class CDigitank* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bShieldOnly);

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

	virtual void			TankSpeak(class CBaseEntity* pTank, const eastl::string& sSpeech);

	CDigitanksTeam*			GetDigitanksTeam(size_t i);

	CDigitanksTeam*			GetCurrentTeam();
	CSelectable*			GetPrimarySelection();
	CDigitank*				GetPrimarySelectionTank();
	CStructure*				GetPrimarySelectionStructure();

	controlmode_t			GetControlMode();
	void					SetControlMode(controlmode_t eMode);

	aimtype_t				GetAimType();
	void					SetAimType(aimtype_t eAimType);
	void					SetAimTypeByWeapon(weapon_t eWeapon);

	CTerrain*				GetTerrain() { if (m_hTerrain == NULL) return NULL; return m_hTerrain; };
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

	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitanksTeam, DownloadUpdate);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitanksTeam, DownloadComplete);

	NET_CALLBACK_ENTITY(CDigitanksGame, CCPU, BeginConstruction);
	NET_CALLBACK_ENTITY(CDigitanksGame, CCPU, BeginRogueProduction);

	NET_CALLBACK_ENTITY(CDigitanksGame, CStructure, BeginStructureConstruction);
	NET_CALLBACK_ENTITY(CDigitanksGame, CStructure, InstallUpdate);
	NET_CALLBACK_ENTITY(CDigitanksGame, CStructure, BeginUpgrade);

	NET_CALLBACK_ENTITY(CDigitanksGame, CSupplier, AddChild);
	NET_CALLBACK_ENTITY(CDigitanksGame, CSupplier, RemoveChild);

	NET_CALLBACK_ENTITY(CDigitanksGame, CLoader, BeginProduction);

	virtual class CDigitanksRenderer*	GetDigitanksRenderer();
	virtual class CDigitanksCamera*		GetDigitanksCamera();

	float					GetGravity();

	void					AddProjectileToWaitFor() { m_iWaitingForProjectiles++; };
	size_t					GetNumProjectilesWaitingFor() { return m_iWaitingForProjectiles; };

	void					WeaponSpecialCommand(CDigitanksTeam* pTeam = NULL);

	void					AddTankAim(Vector vecAim, float flRadius, bool bFocus);
	void					GetTankAims(eastl::vector<Vector>& avecAims, eastl::vector<float>& aflAimRadius, size_t& iFocus);
	void					ClearTankAims();

	void					SetDifficulty(size_t iDifficulty) { m_iDifficulty = iDifficulty; };
	size_t					GetDifficulty() { return m_iDifficulty; };

	void					OnDisplayTutorial(eastl::string sTutorial);

	virtual void			ClientEnterGame();

	void					SetRenderFogOfWar(bool bRenderFogOfWar);
	bool					ShouldRenderFogOfWar();

	float					GetVisibilityAtPoint(CDigitanksTeam* pViewingTeam, Vector vecPoint);

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

	CUpdateGrid*			GetUpdateGrid() { if (m_hUpdates == NULL) return NULL; return m_hUpdates; };

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

	CDigitanksTeam*			GetCurrentLocalDigitanksTeam();

	class CDigitanksLevel*	GetCurrentLevel() { return m_pLevel; };

protected:
	CNetworkedVariable<size_t> m_iCurrentTeam;

	controlmode_t			m_eControlMode;
	aimtype_t				m_eAimType;

	CNetworkedHandle<CTerrain> m_hTerrain;

	CNetworkedHandle<CInstructorEntity> m_hInstructor;

	IDigitanksGameListener*	m_pListener;

	bool					m_bWaitingForMoving;

	bool					m_bWaitingForProjectiles;
	size_t					m_iWaitingForProjectiles;

	bool					m_bTurnActive;

	size_t					m_iPowerups;

	eastl::vector<Vector>	m_avecTankAims;
	eastl::vector<float>	m_aflTankAimRadius;
	size_t					m_iTankAimFocus;

	CNetworkedVariable<size_t> m_iDifficulty;

	CNetworkedVariable<bool> m_bRenderFogOfWar;

	CNetworkedVariable<gametype_t> m_eGameType;
	CNetworkedVariable<size_t> m_iTurn;

	CNetworkedHandle<CUpdateGrid>	m_hUpdates;

	CNetworkedVariable<bool>	m_bPartyMode;
	float						m_flPartyModeStart;
	float						m_flLastFireworks;

	bool					m_bOverrideAllowLasers;

	eastl::vector<airstrike_t>	m_aAirstrikes;

	CNetworkedArray<float, MAX_UNITS> m_aflConstructionCosts;
	CNetworkedArray<float, MAX_UNITS> m_aflUpgradeCosts;

	float						m_flShowFightSign;
	float						m_flShowArtilleryTutorial;

	float						m_flLastHumanMove;

	class CDigitanksLevel*		m_pLevel;
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
