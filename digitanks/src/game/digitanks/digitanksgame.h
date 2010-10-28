#ifndef DT_DIGITANKSGAME_H
#define DT_DIGITANKSGAME_H

#include <vector>

#include "game.h"
#include "digitank.h"
#include <common.h>
#include "terrain.h"
#include "digitanksteam.h"
#include "dt_common.h"
#include "updates.h"

typedef enum
{
	GAMETYPE_EMPTY,
	GAMETYPE_TUTORIAL,
	GAMETYPE_ARTILLERY,
	GAMETYPE_STANDARD,
	GAMETYPE_MENU,
} gametype_t;

typedef enum
{
	ACTIONTYPE_WELCOME,
	ACTIONTYPE_NEWUNIT,
	ACTIONTYPE_NEWSTRUCTURE,
	ACTIONTYPE_AUTOMOVECANCELED,
	ACTIONTYPE_AUTOMOVEENEMY,
	ACTIONTYPE_UNITDAMAGED,
	ACTIONTYPE_FORTIFIEDENEMY,
	ACTIONTYPE_UNITAUTOMOVE,
	ACTIONTYPE_UNITORDERS,
	ACTIONTYPE_CONSTRUCTION,
	ACTIONTYPE_INSTALLATION,
	ACTIONTYPE_UPGRADE,
	ACTIONTYPE_UNITREADY,
} actiontype_t;

typedef struct
{
	size_t			iUnit;
	actiontype_t	eActionType;
	bool			bHandled;
} actionitem_t;

class IDigitanksGameListener
{
public:
	virtual void			GameStart()=0;
	virtual void			GameOver(bool bPlayerWon)=0;

	virtual void			NewCurrentTeam()=0;
	virtual void			NewCurrentSelection()=0;

	virtual void			OnTakeShieldDamage(class CDigitank* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bShieldOnly)=0;
	virtual void			OnTakeDamage(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled)=0;

	virtual void			TankSpeak(class CBaseEntity* pTank, const std::string& sSpeech)=0;

	virtual void			ClearTurnInfo()=0;
	virtual void			AppendTurnInfo(const wchar_t* pszInfo)=0;

	virtual void			SetHUDActive(bool bActive)=0;
};

class CDigitanksGame : public CGame
{
	REGISTER_ENTITY_CLASS(CDigitanksGame, CGame);

public:
	virtual void 			Spawn();

	void					SetListener(IDigitanksGameListener* pListener) { m_pListener = pListener; };
	IDigitanksGameListener*	GetListener() { return m_pListener; };

	virtual void			RegisterNetworkFunctions();

	virtual void			OnClientConnect(CNetworkParameters* p);
	virtual void			OnClientDisconnect(CNetworkParameters* p);

	void					SetupGame(gametype_t eGameType);
	void					SetupEntities();
	NET_CALLBACK(CDigitanksGame, SetupEntities);

	void					SetPlayers(int iPlayers) { m_iPlayers = iPlayers; };
	void					SetTanks(int iTanks) { m_iTanks = iTanks; };

	void					ScatterResources();
	void					ScatterProps();
	void					SetupArtillery();
	void					SetupStandard();
	void					SetupTutorial();
	void					SetupMenuMarch();

	void					StartGame();
	NET_CALLBACK(CDigitanksGame, EnterGame);

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

	virtual void			OnDeleted(class CBaseEntity* pEntity);

	virtual void			TankSpeak(class CBaseEntity* pTank, const std::string& sSpeech);

	CDigitanksTeam*			GetDigitanksTeam(size_t i);

	CDigitanksTeam*			GetCurrentTeam();
	CSelectable*			GetPrimarySelection();
	CDigitank*				GetPrimarySelectionTank();
	CStructure*				GetPrimarySelectionStructure();

	controlmode_t			GetControlMode();
	void					SetControlMode(controlmode_t eMode);

	CTerrain*				GetTerrain() { if (m_hTerrain == NULL) return NULL; return m_hTerrain; };
	NET_CALLBACK(CDigitanksGame, TerrainData);

	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, ManageSupplyLine);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, SetAttackPower);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, Move);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, Turn);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, Fire);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, FireProjectile);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, SetBonusPoints);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, TankPromoted);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, PromoteAttack);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, PromoteDefense);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, PromoteMovement);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, Speak);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, Fortify);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, SetGoalMovePosition);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, CancelGoalMovePosition);

	NET_CALLBACK_ENTITY(CDigitanksGame, CUpdateGrid, UpdatesData);

	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitanksTeam, TeamUpdatesData);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitanksTeam, DownloadUpdate);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitanksTeam, DownloadComplete);

	NET_CALLBACK_ENTITY(CDigitanksGame, CCPU, BeginConstruction);
	NET_CALLBACK_ENTITY(CDigitanksGame, CCPU, CancelConstruction);
	NET_CALLBACK_ENTITY(CDigitanksGame, CCPU, BeginRogueProduction);
	NET_CALLBACK_ENTITY(CDigitanksGame, CCPU, CancelRogueProduction);

	NET_CALLBACK_ENTITY(CDigitanksGame, CStructure, BeginStructureConstruction);
	NET_CALLBACK_ENTITY(CDigitanksGame, CStructure, InstallUpdate);
	NET_CALLBACK_ENTITY(CDigitanksGame, CStructure, CancelInstall);
	NET_CALLBACK_ENTITY(CDigitanksGame, CStructure, BeginUpgrade);
	NET_CALLBACK_ENTITY(CDigitanksGame, CStructure, CancelUpgrade);
	NET_CALLBACK_ENTITY(CDigitanksGame, CStructure, AddStructureUpdate);

	NET_CALLBACK_ENTITY(CDigitanksGame, CSupplier, AddChild);
	NET_CALLBACK_ENTITY(CDigitanksGame, CSupplier, RemoveChild);

	NET_CALLBACK_ENTITY(CDigitanksGame, CLoader, BeginProduction);
	NET_CALLBACK_ENTITY(CDigitanksGame, CLoader, CancelProduction);

	virtual CRenderer*		CreateRenderer();
	virtual class CDigitanksRenderer*	GetDigitanksRenderer();

	virtual CCamera*		CreateCamera();
	virtual class CDigitanksCamera*	GetDigitanksCamera();

	float					GetGravity();

	void					AddProjectileToWaitFor() { m_iWaitingForProjectiles++; };

	void					AddTankAim(Vector vecAim, float flRadius, bool bFocus);
	void					GetTankAims(std::vector<Vector>& avecAims, std::vector<float>& aflAimRadius, size_t& iFocus);
	void					ClearTankAims();

	void					SetDifficulty(size_t iDifficulty) { m_iDifficulty = iDifficulty; };
	size_t					GetDifficulty() { return m_iDifficulty; };

	void					AppendTurnInfo(const wchar_t* pszTurnInfo);

	void					OnDisplayTutorial(size_t iTutorial);

	virtual void			ClientEnterGame();

	void					SetRenderFogOfWar(bool bRenderFogOfWar) { m_bRenderFogOfWar = bRenderFogOfWar; };
	bool					ShouldRenderFogOfWar();

	bool					ShouldShowScores();

	bool					CanBuildMiniBuffers();
	bool					CanBuildBuffers();
	bool					CanBuildBatteries();
	bool					CanBuildPSUs();
	bool					CanBuildInfantryLoaders();
	bool					CanBuildTankLoaders();
	bool					CanBuildArtilleryLoaders();

	void					AddActionItem(CSelectable* pUnit, actiontype_t eActionType);
	std::vector<actionitem_t>&	GetActionItems() { return m_aActionItems; }
	void					AllowActionItems(bool bAllow) { m_bAllowActionItems = bAllow; };

	CUpdateGrid*			GetUpdateGrid() { if (m_hUpdates == NULL) return NULL; return m_hUpdates; };

	gametype_t				GetGameType() { return m_eGameType; }
	size_t					GetTurn() { return m_iTurn; }

	float					FogPenetrationDistance() { return 10; }

	bool					IsPartyMode() { return m_bPartyMode; };

	static void				UpdateHUD(CNetworkedVariableBase* pVariable);

	// CHEAT!
	void					CompleteProductions();

	CDigitanksTeam*			GetLocalDigitanksTeam();

protected:
	CNetworkedVariable<size_t> m_iCurrentTeam;

	controlmode_t			m_eControlMode;

	CNetworkedHandle<CTerrain> m_hTerrain;

	IDigitanksGameListener*	m_pListener;

	bool					m_bWaitingForMoving;

	bool					m_bWaitingForProjectiles;
	size_t					m_iWaitingForProjectiles;

	bool					m_bTurnActive;

	size_t					m_iPowerups;

	std::vector<Vector>		m_avecTankAims;
	std::vector<float>		m_aflTankAimRadius;
	size_t					m_iTankAimFocus;

	CNetworkedVariable<size_t> m_iDifficulty;

	CNetworkedVariable<bool> m_bRenderFogOfWar;

	int							m_iPlayers;
	int							m_iTanks;

	CNetworkedVariable<gametype_t> m_eGameType;
	CNetworkedVariable<size_t> m_iTurn;

	CNetworkedHandle<CUpdateGrid>	m_hUpdates;

	std::vector<actionitem_t>	m_aActionItems;
	bool					m_bAllowActionItems;

	CNetworkedVariable<bool>	m_bPartyMode;
	float						m_flLastFireworks;
};

inline class CDigitanksGame* DigitanksGame()
{
	if (!Game())
		return NULL;

	return dynamic_cast<CDigitanksGame*>(Game());
}

enum
{
	CG_ENTITY = 1,
	CG_TERRAIN,
	CG_PROJECTILE,
	CG_POWERUP,
	CG_PROP,
};

#endif
