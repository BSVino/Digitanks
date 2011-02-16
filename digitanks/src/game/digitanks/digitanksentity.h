#ifndef DT_DIGITANKSENTITY_H
#define DT_DIGITANKSENTITY_H

#include <baseentity.h>

#include "dt_common.h"

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
	virtual float					GetCloakConcealment() const { return 0; };
	virtual bool					HasLostConcealment() const { return false; }
	virtual float					VisibleRange() const;
	virtual bool					IsRammable() const { return true; }

	virtual float					AvailableArea() const { return 0; };
	virtual int						GetNumAvailableAreas() const { return 0; };
	virtual bool					IsAvailableAreaActive(int iArea) const { return true; };
	virtual void					RenderAvailableArea(int iArea);

	virtual void					ModifyContext(class CRenderingContext* pContext, bool bTransparent);

	virtual void					DownloadComplete(size_t x, size_t y) {};

	virtual void					UpdateInfo(eastl::string16& sInfo) {};
	virtual eastl::string16			GetName() { return L"Entity"; };
	virtual unittype_t				GetUnitType() const { return UNITTYPE_UNDEFINED; };

	virtual float					HealthRechargeRate() const { return 0.2f; };
	virtual float					BaseVisibleRange() const { return 0; };
	virtual float					TreesReduceVisibility() const { return true; };
	virtual float					TotalHealth() const { return 10; };

protected:
	eastl::vector<CEntityHandle<class CSupplyLine> >	m_ahSupplyLinesIntercepted;
};

#endif