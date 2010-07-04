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
	virtual void				Spawn();

	virtual void				StartTurn();

	virtual void				SetupMenu(menumode_t eMenuMode);

	virtual void				OnRender();

	void						BeginConstruction();
	void						CancelConstruction();

	void						SetBuildUnit(buildunit_t eBuildUnit) { m_eBuildUnit = eBuildUnit; };
	buildunit_t					GetBuildUnit() { return m_eBuildUnit; };

protected:
	buildunit_t					m_eBuildUnit;

	bool						m_bProducing;
	size_t						m_iProductionStored;
};

#endif
