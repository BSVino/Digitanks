#ifndef DT_STRUCTURE_H
#define DT_STRUCTURE_H

#include "selectable.h"

class CStructure : public CSelectable
{
	REGISTER_ENTITY_CLASS(CStructure, CSelectable);

public:
								CStructure();

public:
	void						BeginConstruction(size_t iTurnsToConstruct);
	size_t						GetTurnsToConstruct() { return m_iTurnsToConstruct; };
	bool						IsConstructing() { return m_bConstructing; };

	virtual void				StartTurn();
	virtual void				PostStartTurn() {};

	virtual void				ModifyContext(class CRenderingContext* pContext);

protected:
	bool						m_bConstructing;
	size_t						m_iTurnsToConstruct;
};

#endif
