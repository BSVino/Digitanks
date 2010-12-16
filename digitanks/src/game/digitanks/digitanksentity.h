#ifndef DT_DIGITANKSENTITY_H
#define DT_DIGITANKSENTITY_H

#include <baseentity.h>

typedef enum
{
	UNITTYPE_UNDEFINED = 0,
	STRUCTURE_CPU,
	STRUCTURE_MINIBUFFER,
	STRUCTURE_BUFFER,
	STRUCTURE_BATTERY,
	STRUCTURE_PSU,
	STRUCTURE_INFANTRYLOADER,
	STRUCTURE_TANKLOADER,
	STRUCTURE_ARTILLERYLOADER,
	STRUCTURE_ELECTRONODE,
	UNIT_INFANTRY,
	UNIT_TANK,
	UNIT_ARTILLERY,
	UNIT_SCOUT,
	UNIT_MOBILECPU,
	MAX_UNITS,
} unittype_t;

class CDigitanksEntity : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CDigitanksEntity, CBaseEntity);

public:
	virtual void					Spawn();

	virtual void					Think();

	virtual void					StartTurn();
	virtual void					EndTurn();

	virtual void					InterceptSupplyLines();

	class CDigitanksTeam*			GetDigitanksTeam() const;

	virtual bool					ShouldRender() const;
	virtual void					RenderVisibleArea();
	virtual float					GetVisibility(CDigitanksTeam* pTeam) const;
	virtual float					GetVisibility() const;
	virtual bool					GetsConcealmentBonus() const { return true; };

	virtual void					ModifyContext(class CRenderingContext* pContext, bool bTransparent);

	virtual void					DownloadComplete(size_t x, size_t y) {};

	virtual void					UpdateInfo(eastl::string16& sInfo) {};
	virtual eastl::string16			GetName() { return L"Entity"; };
	virtual unittype_t				GetUnitType() const { return UNITTYPE_UNDEFINED; };

	virtual float					HealthRechargeRate() const { return 0.2f; };
	virtual float					VisibleRange() const { return 0; };
	virtual float					TotalHealth() const { return 10; };

protected:
	eastl::vector<CEntityHandle<class CSupplyLine> >	m_ahSupplyLinesIntercepted;
};

#endif