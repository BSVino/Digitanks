#ifndef DT_DIGITANKSTEAM_H
#define DT_DIGITANKSTEAM_H

#include <game/entities/player.h>

#include "units/digitank.h"
#include "updates.h"

// AI stuff
#include "structures/cpu.h"

typedef enum
{
	ACTIONTYPE_WELCOME,
	ACTIONTYPE_CONTROLS,
	ACTIONTYPE_NEWSTRUCTURE,
	ACTIONTYPE_AUTOMOVECANCELED,
	ACTIONTYPE_AUTOMOVEENEMY,
	ACTIONTYPE_UNITDAMAGED,
	ACTIONTYPE_FORTIFIEDENEMY,
	ACTIONTYPE_UNITAUTOMOVE,
	ACTIONTYPE_UNITORDERS,
	ACTIONTYPE_UPGRADE,
	ACTIONTYPE_UNITREADY,
	ACTIONTYPE_DOWNLOADUPDATES,
	ACTIONTYPE_DOWNLOADCOMPLETE,
} actiontype_t;

typedef enum
{
	TOUCHCOMMAND_NONE = 0,
	TOUCHCOMMAND_CENTERSELECT,
	TOUCHCOMMAND_DRAG,
	TOUCHCOMMAND_ROTATEZOOM,
} touchcommand_t;

typedef struct
{
	size_t			iUnit;
	actiontype_t	eActionType;
	bool			bHandled;
} actionitem_t;

class CDigitanksPlayer : public CPlayer
{
	friend class CDigitanksGame;

	REGISTER_ENTITY_CLASS(CDigitanksPlayer, CPlayer);

public:
								CDigitanksPlayer();
	virtual						~CDigitanksPlayer();

public:
	virtual void				Spawn();

	virtual void				ClientEnterGame();

	class CSelectable*			GetPrimarySelection();
	class CDigitank*			GetPrimarySelectionTank();
	class CStructure*			GetPrimarySelectionStructure();
	size_t						GetPrimarySelectionId();
	void						SetPrimarySelection(const CSelectable* pCurrent);
	bool						IsPrimarySelection(const class CSelectable* pEntity);
	void						AddToSelection(const CSelectable* pCurrent);
	bool						IsSelected(const class CSelectable* pEntity);
	size_t						GetNumSelected() { return m_aiCurrentSelection.size(); }

	class CCPU*					GetPrimaryCPU() { return m_hPrimaryCPU; }

	void						StartNewRound();

	void						StartTurn();
	void						EndTurn();

	void						CountProducers();
	void						AddPowerPerTurn(float flPower);
	float						GetPowerPerTurn() { return m_flPowerPerTurn; };
	float						GetPower() { return m_flPower; };
	void						ConsumePower(float flPower);
	void						AddPower(float flPower) { m_flPower += flPower; };

	void						CountFleetPoints();
	size_t						GetTotalFleetPoints() { return m_iTotalFleetPoints; };
	size_t						GetUsedFleetPoints() { return m_iUsedFleetPoints; };
	size_t						GetUnusedFleetPoints();

	void						CountScore();
	size_t						GetScore() const { return m_iScore; };

	void						YouLoseSirGoodDay();
	bool						HasLost() const { return m_bLost; }

	void						CountBandwidth();
	float						GetBandwidth() { return m_flBandwidth; };

	void						AppendTurnInfo(const tstring& sTurnInfo);
	tstring				GetTurnInfo();

	virtual void				OnDeleted(const class CBaseEntity* pEntity);

	size_t						GetNumTanksAlive() const;

	void						CalculateVisibility();
	void						CalculateEntityVisibility(class CBaseEntity* pEntity);
	float						GetEntityVisibility(size_t iHandle);
	float						GetVisibilityAtPoint(Vector vecPoint, bool bCloak = false) const;

	void						AddActionItem(const CSelectable* pUnit, actiontype_t eActionType);
	void						ClearActionItems();
	size_t						GetNumActionItems() const { return m_aActionItems.size(); };
	const actionitem_t*			GetActionItem(size_t i) const { return &m_aActionItems[i]; };
	void						ServerHandledActionItem(size_t i);
	void						HandledActionItem(size_t i);
	void						HandledActionItem(CSelectable* pUnit);
	void						HandledActionItem(actiontype_t eItem);

	void						DownloadUpdate(int iX, int iY, bool bCheck = true);
	void						DownloadUpdate(class CNetworkParameters* p);
	float						GetUpdateDownloaded() { return m_flUpdateDownloaded; };
	float						GetUpdateSize();
	void						DownloadComplete(bool bInformMembers = true);
	void						DownloadComplete(class CNetworkParameters* p);
	bool						HasDownloadedUpdate(int iX, int iY) const;
	bool						CanDownloadUpdate(int iX, int iY) const;
	bool						IsDownloading(int iX, int iY);
	class CUpdateItem*			GetUpdateDownloading();
	size_t						GetTurnsToDownload();
	float						GetMegabytes() { return m_flMegabytes; }

	bool						CanBuildMiniBuffers();
	bool						CanBuildBuffers();
	bool						CanBuildBatteries();
	bool						CanBuildPSUs();
	bool						CanBuildLoaders();
	bool						CanBuildInfantryLoaders();
	bool						CanBuildTankLoaders();
	bool						CanBuildArtilleryLoaders();

	// AI stuff
	void						Bot_DownloadUpdates();
	bool						Bot_BuildFirstPriority();
	void						Bot_AssignDefenders();
	void						Bot_ExecuteTurn();
	void						Bot_ExecuteTurnArtillery();
	void						Bot_ExecuteTurnCampaign();
	void						Bot_AddBuildPriority(unittype_t eUnit, CDigitanksEntity* pTarget = NULL);
	CSupplier*					Bot_FindUnusedSupplier(size_t iDependents = ~0, bool bNoSuppliers = true);
	bool						Bot_BuildCollector(class CResourceNode* pResource);
	void						Bot_UseArtilleryAI() { m_bUseArtilleryAI = true; }

	size_t						GetNumTanks() const { return m_ahTanks.size(); };
	class CDigitank*			GetTank(size_t i) const;

	size_t						GetNumUnits() const { return m_ahUnits.size(); };
	CDigitanksEntity*			GetUnit(size_t i) const;

	void						AddUnit(CDigitanksEntity* pEntity);
	virtual void				OnAddUnit(CDigitanksEntity* pEntity);

	void						RemoveUnit(CDigitanksEntity* pEntity);
	virtual void				OnRemoveUnit(CDigitanksEntity* pEntity);

	virtual void				MouseMotion(int dx, int dy);
	virtual bool				MouseInput(int iButton, tinker_mouse_state_t iState);
	void                        MouseDoubleClick();
	virtual void				MouseWheel(int iState);
	virtual void				KeyPress(int c);
	virtual void				KeyRelease(int c);
	virtual void				CharPress(int c);
	virtual void				CharRelease(int c);
	virtual void TouchMotion(int iFinger, float x, float y, float dx, float dy);
	virtual bool TouchInput(int iFinger, tinker_mouse_state_t iState, float x, float y);

	bool GetBoxSelection(size_t& iX, size_t& iY, size_t& iX2, size_t& iY2);
	bool IsMouseDragging();

	int  GetMouseCurrentX() { return m_iMouseCurrentX; };
	int  GetMouseCurrentY() { return m_iMouseCurrentY; };

	void						SetLoseCondition(losecondition_t eLose) { m_eLoseCondition = eLose; }
	losecondition_t				GetLoseCondition() { return m_eLoseCondition; }

	void						DontIncludeInScoreboard() { m_bIncludeInScoreboard = false; }
	bool						ShouldIncludeInScoreboard() const { return m_bIncludeInScoreboard; }

	touchcommand_t GetTouchCommand() const { return m_eTouchCommand; }

protected:
	CNetworkedSTLVector<CEntityHandle<CDigitank> >        m_ahTanks;
	CNetworkedSTLVector<CEntityHandle<CDigitanksEntity> > m_ahUnits;

	tvector<size_t>		m_aiCurrentSelection;

	tmap<size_t, float>	m_aflVisibilities;

	CNetworkedVariable<float>	m_flPowerPerTurn;
	CNetworkedVariable<float>	m_flPower;

	CNetworkedVariable<size_t>	m_iTotalFleetPoints;
	CNetworkedVariable<size_t>	m_iUsedFleetPoints;

	CNetworkedVariable<size_t>	m_iScore;

	CNetworkedVariable<bool>	m_bLost;

	CNetworkedSTLVector<actionitem_t> m_aActionItems;

	// AI stuff
	CNetworkedHandle<CCPU>		m_hPrimaryCPU;
	CEntityHandle<CLoader>		m_hInfantryLoader;
	CEntityHandle<CLoader>		m_hTankLoader;
	CEntityHandle<CLoader>		m_hArtilleryLoader;

	typedef struct
	{
		unittype_t				m_eUnit;
		CEntityHandle<CDigitanksEntity> m_hTarget;
	} builditem_t;
	tvector<builditem_t>		m_aeBuildPriorities;
	size_t                      m_iBuildPrioritiesHead;
	bool						m_bCanUpgrade;

	Vector						m_vecExplore;
	bool						m_bUseArtilleryAI;

	bool						m_bLKV;
	Vector						m_vecLKV;

	size_t						m_iFleetPointAttackQuota;
	tvector<CEntityHandle<CDigitank> >	m_ahAttackTeam;

	size_t						m_iLastScoutBuilt;

	CNetworkedVariable<int>		m_iCurrentUpdateX;
	CNetworkedVariable<int>		m_iCurrentUpdateY;
	CNetworkedArray<bool, UPDATE_GRID_SIZE*UPDATE_GRID_SIZE> m_abUpdates;
	CNetworkedVariable<float>	m_flUpdateDownloaded;
	CNetworkedVariable<float>	m_flMegabytes;
	CNetworkedVariable<float>	m_flBandwidth;

	CNetworkedString			m_sTurnInfo;

	CNetworkedVariable<bool>	m_bCanBuildBuffers;
	CNetworkedVariable<bool>	m_bCanBuildPSUs;
	CNetworkedVariable<bool>	m_bCanBuildInfantryLoaders;
	CNetworkedVariable<bool>	m_bCanBuildTankLoaders;
	CNetworkedVariable<bool>	m_bCanBuildArtilleryLoaders;

	CNetworkedVariable<losecondition_t> m_eLoseCondition;

	CNetworkedVariable<bool>	m_bIncludeInScoreboard;

	int							m_iMouseMoved;
	bool						m_bBoxSelect;

	int							m_iMouseInitialX;
	int							m_iMouseInitialY;
	int							m_iMouseCurrentX;
	int							m_iMouseCurrentY;

	touchcommand_t m_eTouchCommand;
	float m_flTouchStartX;
	float m_flTouchStartY;
	float m_flTouchMoved;
};

inline CDigitanksPlayer* ToDigitanksPlayer(CBaseEntity* pPlayer)
{
	return dynamic_cast<CDigitanksPlayer*>(pPlayer);
}

#endif