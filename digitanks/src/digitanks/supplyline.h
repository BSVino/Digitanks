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

	virtual const TVector			GetGlobalOrigin() const;
	virtual const TFloat			GetRenderRadius() const;
	virtual float					Distance(Vector vecSpot) const;

	virtual void					StartTurn();

	void							Intercept(float flIntercept);
	float							GetIntegrity() const { return m_flIntegrity; };
	void							SetIntegrity(float flIntegrity) { m_flIntegrity = flIntegrity; m_bDelayRecharge = false; };

	virtual bool					ShouldRender() const { return true; };
	virtual void					PostRender() const;

	CSupplier*						GetSupplier();
	CBaseEntity*					GetEntity();

	static float					MinimumIntegrity() { return 0.25f; }

protected:
	CNetworkedHandle<CSupplier>		m_hSupplier;
	CNetworkedHandle<CBaseEntity>	m_hEntity;

	CNetworkedVariable<float>		m_flIntegrity;
	bool							m_bDelayRecharge;

	static CMaterialHandle           s_hSupplyBeam;
};

#endif
