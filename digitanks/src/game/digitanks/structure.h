#ifndef DT_STRUCTURE_H
#define DT_STRUCTURE_H

#include "selectable.h"
#include "supplyline.h"

class CStructure : public CSelectable
{
	REGISTER_ENTITY_CLASS(CStructure, CSelectable);

public:
								CStructure();

public:
	void						BeginConstruction(size_t iTurnsToConstruct);
	size_t						GetTurnsToConstruct() { return m_iTurnsToConstruct; };
	bool						IsConstructing() { return m_bConstructing; };

	virtual void				PreStartTurn();
	virtual void				StartTurn();
	virtual void				PostStartTurn() {};

	virtual void				SetSupplier(class CSupplier* pSupplier);

	virtual void				ModifyContext(class CRenderingContext* pContext);

protected:
	bool						m_bConstructing;
	size_t						m_iTurnsToConstruct;

	CEntityHandle<CSupplier>		m_hSupplier;
	CEntityHandle<CSupplyLine>		m_hSupplyLine;
};

class CSupplier : public CStructure
{
	REGISTER_ENTITY_CLASS(CSupplier, CStructure);

public:
	static CSupplier*			FindClosestSupplier(CBaseEntity* pUnit);
	static CSupplier*			FindClosestSupplier(Vector vecPoint, class CTeam* pTeam);
};

#endif
