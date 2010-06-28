#ifndef DT_SUPPLYLINE_H
#define DT_SUPPLYLINE_H

#include <baseentity.h>

class CSupplyLine : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CSupplyLine, CBaseEntity);

public:
	void							SetEntities(class CSupplier* pSupplier, CBaseEntity* pEntity);

	virtual Vector					GetOrigin() const;

	virtual void					OnRender();

protected:
	CEntityHandle<CSupplier>		m_hSupplier;
	CEntityHandle<CBaseEntity>		m_hEntity;
};

#endif