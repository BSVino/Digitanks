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

	virtual void				UpdateInfo(std::string& sInfo);

	void						BeginProduction();
	void						CancelProduction();
	bool						IsProducing() { return m_bProducing; };
	void						AddProduction(size_t iProduction) { m_iProductionStored += iProduction; }

	size_t						GetFleetPointsRequired();
	bool						HasEnoughFleetPoints();

	size_t						GetTurnsToProduce();

	void						SetBuildUnit(buildunit_t eBuildUnit);
	buildunit_t					GetBuildUnit() { return m_eBuildUnit; };

	virtual const char*			GetName();
	virtual size_t				ConstructionCost() const { return 70; };
	virtual float				TotalHealth() const { return 70; };

protected:
	buildunit_t					m_eBuildUnit;

	bool						m_bProducing;
	size_t						m_iProductionStored;
};

#endif
