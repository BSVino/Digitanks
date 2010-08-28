#ifndef DT_LOADER_H
#define DT_LOADER_H

#include "structure.h"

typedef enum
{
	BUILDUNIT_INFANTRY,
	BUILDUNIT_TANK,
	BUILDUNIT_ARTILLERY,
} buildunit_t;

class CLoader : public CStructure
{
	REGISTER_ENTITY_CLASS(CLoader, CStructure);

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual void				StartTurn();

	virtual void				SetupMenu(menumode_t eMenuMode);

	virtual void				UpdateInfo(std::wstring& sInfo);

	void						BeginProduction();
	void						CancelProduction();
	bool						IsProducing() { return m_bProducing; };
	void						AddProduction(size_t iProduction) { m_iProductionStored += iProduction; }
	size_t						GetUnitProductionCost();

	virtual void				InstallUpdate(updatetype_t eUpdate);
	virtual void				InstallComplete();
	virtual bool				HasUpdatesAvailable();

	size_t						GetFleetPointsRequired();
	bool						HasEnoughFleetPoints();

	size_t						GetTurnsToProduce();

	void						SetBuildUnit(buildunit_t eBuildUnit);
	buildunit_t					GetBuildUnit() { return m_eBuildUnit; };

	virtual const wchar_t*		GetName();
	virtual unittype_t			GetUnitType();
	virtual size_t				ConstructionCost() const { return GetLoaderConstructionCost(); };
	virtual float				TotalHealth() const { return 70; };

	static size_t				GetUnitProductionCost(buildunit_t eBuildUnit);
	static size_t				GetLoaderConstructionCost() { return 100; };

protected:
	buildunit_t					m_eBuildUnit;

	bool						m_bProducing;
	size_t						m_iProductionStored;

	size_t						m_iTankAttack;
	size_t						m_iTankDefense;
	size_t						m_iTankMovement;
	size_t						m_iTankHealth;
	size_t						m_iTankRange;

	static size_t				s_iCancelIcon;
	static size_t				s_iInstallIcon;
	static size_t				s_iInstallAttackIcon;
	static size_t				s_iInstallDefenseIcon;
	static size_t				s_iInstallMovementIcon;
	static size_t				s_iInstallRangeIcon;
	static size_t				s_iInstallHealthIcon;
	static size_t				s_iBuildInfantryIcon;
	static size_t				s_iBuildTankIcon;
	static size_t				s_iBuildArtilleryIcon;
};

#endif
