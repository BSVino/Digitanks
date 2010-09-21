#ifndef DT_SUPPLYLINE_H
#define DT_SUPPLYLINE_H

#include "digitanksentity.h"

class CSupplyLine : public CDigitanksEntity
{
	REGISTER_ENTITY_CLASS(CSupplyLine, CDigitanksEntity);

public:
	void							Precache();
	void							Spawn();

	void							SetEntities(class CSupplier* pSupplier, CBaseEntity* pEntity);

	virtual Vector					GetOrigin() const;

	virtual void					StartTurn();

	virtual bool					ShouldRender() const { return true; };
	virtual void					PostRender();

protected:
	CEntityHandle<CSupplier>		m_hSupplier;
	CEntityHandle<CBaseEntity>		m_hEntity;

	bool							m_bIntercepted;

	static size_t					s_iSupplyBeam;
};

#endif