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
	GAMETYPE_TUTORIAL,
	GAMETYPE_ARTILLERY,
	GAMETYPE_STANDARD,
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

	virtual void			TankSpeak(class CDigitank* pTank, const std::string& sSpeech)=0;

	virtual void			ClearTurnInfo()=0;
	virtual void			AppendTurnInfo(const wchar_t* pszInfo)=0;

	virtual void			SetHUDActive(bool bActive)=0;
};

class CDigitanksGame : public CGame
{
	DECLARE_CLASS(CDigitanksGame, CGame);

public:
							CDigitanksGame();
							~CDigitanksGame();

public:
	void					SetListener(IDigitanksGameListener* pListener) { m_pListener = pListener; };
	IDigitanksGameListener*	GetListener() { return m_pListener; };

	virtual void			RegisterNetworkFunctions();

	virtual void			OnClientConnect(CNetworkParameters* p);
	virtual void			OnClientUpdate(CNetworkParameters* p);
	virtual void			OnClientDisconnect(CNetworkParameters* p);

	void					SetupGame(gametype_t eGameType);
	void					SetupEntities();
	NET_CALLBACK(CDigitanksGame, SetupEntities);

	void					ScatterResources();
	void					ScatterProps();
	void					SetupArtillery();
	void					SetupStandard();
	void					SetupTutorial();

	void					StartGame();
	NET_CALLBACK(CDigitanksGame, EnterGame);

	virtual void			Think();

	void					SetDesiredMove();
	void					SetDesiredTurn(Vector vecLookAt = Vector());
	void					SetDesiredAim();

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

	virtual void			OnDeleted(class CBaseEntity* pEntity);

	virtual void			TankSpeak(class CDigitank* pTank, const std::string& sSpeech);

	CDigitanksTeam*			GetDigitanksTeam(size_t i);

	CDigitanksTeam*			GetCurrentTeam();
	CSelectable*			GetPrimarySelection();
	CDigitank*				GetPrimarySelectionTank();
	CStructure*				GetPrimarySelectionStructure();

	controlmode_t			GetControlMode();
	void					SetControlMode(controlmode_t eMode);

	CTerrain*				GetTerrain() { if (m_hTerrain == NULL) return NULL; return m_hTerrain; };
	NET_CALLBACK(CDigitanksGame, SetTerrain);
	NET_CALLBACK(CDigitanksGame, TerrainData);

	NET_CALLBACK(CDigitanksGame, SetCurrentTeam);

	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, SetDesiredMove);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, CancelDesiredMove);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, SetDesiredTurn);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, CancelDesiredTurn);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, SetDesiredAim);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, CancelDesiredAim);
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

	virtual void			CreateRenderer();
	virtual class CDigitanksRenderer*	GetDigitanksRenderer();

	virtual void			CreateCamera();
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

	// CHEAT!
	void					CompleteProductions();

	static CDigitanksTeam*	GetLocalDigitanksTeam();

protected:
	size_t					m_iCurrentTeam;

	controlmode_t			m_eControlMode;

	CEntityHandle<CTerrain>	m_hTerrain;

	IDigitanksGameListener*	m_pListener;

	bool					m_bWaitingForMoving;

	bool					m_bWaitingForProjectiles;
	size_t					m_iWaitingForProjectiles;

	bool					m_bTurnActive;

	size_t					m_iPowerups;

	std::vector<Vector>		m_avecTankAims;
	std::vector<float>		m_aflTankAimRadius;
	size_t					m_iTankAimFocus;

	size_t					m_iDifficulty;

	bool					m_bRenderFogOfWar;

	gametype_t				m_eGameType;
	size_t					m_iTurn;

	CEntityHandle<CUpdateGrid>	m_hUpdates;

	std::vector<actionitem_t>	m_aActionItems;
	bool					m_bAllowActionItems;
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
};

#endif
