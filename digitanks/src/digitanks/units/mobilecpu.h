#ifndef DT_MOBILECPU_H
#define DT_MOBILECPU_H

#include <units/digitank.h>

class CMobileCPU : public CDigitank
{
	REGISTER_ENTITY_CLASS(CMobileCPU, CDigitank);

public:
	virtual void				Precache();
	virtual void				Spawn();
	virtual void				Think();

	virtual tstring		GetEntityName() const { return _T("MCP"); };

	virtual bool				CanFortify();
	virtual void				OnFortify();

	virtual float				FindHoverHeight(Vector vecPosition) const;
	virtual Vector				GetRenderOrigin() const;

	virtual void				ModifyContext(class CRenderingContext* pContext, bool bTransparent) const;
	virtual void				OnRender(class CRenderingContext* pContext, bool bTransparent) const;

	virtual bool				IsMobileCPU() const { return true; };
	virtual float				BaseHealthRechargeRate() const { return 20.0f; };
	virtual float				GetTankSpeed() const { return 3.0f; };
	virtual float				TurnPerPower() const { return 45; };
	virtual float				GetTransitionTime() const { return 4.0f; };
	virtual float				MaxRangeRadius() const { return 30; };
	virtual float				SlowMovementFactor() const { return 0.8f; };

	virtual size_t				FleetPoints() const { return 0; };

	virtual unittype_t			GetUnitType() const { return UNIT_MOBILECPU; }

protected:
	size_t						m_iFanModel;
	float						m_flFanRotation;
};

#endif
