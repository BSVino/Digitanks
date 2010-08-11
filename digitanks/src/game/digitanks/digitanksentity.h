#ifndef DT_DIGITANKSENTITY_H
#define DT_DIGITANKSENTITY_H

#include <baseentity.h>

typedef enum
{
	UNITTYPE_UNDEFINED = 0,
	STRUCTURE_CPU,
	STRUCTURE_BUFFER,
	STRUCTURE_PSU,
	STRUCTURE_INFANTRYLOADER,
	STRUCTURE_TANKLOADER,
	STRUCTURE_ARTILLERYLOADER,
	STRUCTURE_ELECTRONODE,
	UNIT_INFANTRY,
	UNIT_TANK,
	UNIT_ARTILLERY,
} unittype_t;

class CDigitanksEntity : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CDigitanksEntity, CBaseEntity);

public:
	virtual void					Spawn();

	virtual void					Think();

	virtual void					StartTurn();

	class CDigitanksTeam*			GetDigitanksTeam() const;

	virtual void					RenderVisibleArea();
	virtual float					GetVisibility(CDigitanksTeam* pTeam) const;
	virtual float					GetVisibility() const;

	virtual void					ModifyContext(class CRenderingContext* pContext);
	virtual void					OnRender();

	virtual void					DownloadComplete(class CUpdateItem* pItem) {};

	virtual void					UpdateInfo(std::string& sInfo) {};
	virtual const char*				GetName() { return "Entity"; };
	virtual unittype_t				GetUnitType() { return UNITTYPE_UNDEFINED; };

	virtual float					HealthRechargeRate() const { return 0.2f; };
	virtual float					VisibleRange() const { return 0; };
	virtual float					TotalHealth() const { return 10; };
};

#endif