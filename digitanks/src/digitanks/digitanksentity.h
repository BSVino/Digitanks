#ifndef DT_DIGITANKSENTITY_H
#define DT_DIGITANKSENTITY_H

#include <game/baseentity.h>
#include <renderer/particles.h>

#include "dt_common.h"

class CDigitanksEntity : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CDigitanksEntity, CBaseEntity);

public:
	virtual void					Precache();
	virtual void					Spawn();

	virtual void					ClientSpawn();

	virtual void					Think();

	class CWreckage*				CreateWreckage();

	virtual void					StartTurn();
	virtual void					EndTurn();

	virtual void					InterceptSupplyLines();

	class CDigitanksTeam*			GetDigitanksTeam() const;

	virtual bool					ShouldRender() const;
	virtual void					RenderVisibleArea();

	virtual void					OnSetOrigin(const Vector& vecOrigin);

	virtual float					GetVisibility(CDigitanksTeam* pTeam) const;
	virtual float					GetVisibility() const;
	virtual float					GetVisibility();
	virtual void					CalculateVisibility();
	virtual void					DirtyVisibility() { m_bVisibilityDirty = true; };
	virtual void					DirtyArea() { m_flNextDirtyArea = GameServer()->GetGameTime(); };
	DECLARE_ENTITY_OUTPUT(OnBecomeVisible);
	DECLARE_ENTITY_OUTPUT(OnBecomeFullyVisible);

	virtual bool					GetsConcealmentBonus() const { return true; };
	virtual float					GetCloakConcealment() const { return 0; };
	virtual bool					HasLostConcealment() const { return false; }
	virtual float					VisibleRange() const;
	virtual bool					IsRammable() const { return true; }

	virtual float					AvailableArea(int iArea) const { return 0; };
	virtual int						GetNumAvailableAreas() const { return 0; };
	virtual bool					IsAvailableAreaActive(int iArea) const { return true; };
	virtual void					RenderAvailableArea(int iArea);

	virtual bool					IsTouching(CBaseEntity* pOther, Vector& vecPoint) const;

	void							Imprison();
	bool							IsImprisoned() const { return m_bImprisoned; }
	void							Rescue(CDigitanksEntity* pOther);
	DECLARE_ENTITY_OUTPUT(OnRescue);

	virtual void					ModifyContext(class CRenderingContext* pContext, bool bTransparent) const;
	virtual void					OnRender(class CRenderingContext* pContext, bool bTransparent) const;

	virtual void					DownloadComplete(size_t x, size_t y) {};

	virtual void					UpdateInfo(tstring& sInfo) {};
	virtual tstring					GetEntityName() const { return _T("Entity"); };
	virtual unittype_t				GetUnitType() const { return UNITTYPE_UNDEFINED; };

	void							SetObjective(bool bObjective) { m_bObjective = bObjective; }
	bool							IsObjective() { return m_bObjective; }
	DECLARE_ENTITY_INPUT(MakeObjective);
	DECLARE_ENTITY_INPUT(ClearObjective);

	virtual float					HealthRechargeRate() const { return 0.2f; };
	virtual float					BaseVisibleRange() const { return 0; };
	virtual float					TreesReduceVisibility() const { return true; };
	virtual float					TotalHealth() const { return 100; };

protected:
	eastl::vector<CEntityHandle<class CSupplyLine> >	m_ahSupplyLinesIntercepted;

	bool							m_bVisibilityDirty;
	float							m_flVisibility;				// Only for local team!

	float							m_flNextDirtyArea;
	float							m_flNextDirtyOrigin;

	bool							m_bImprisoned;

	bool							m_bObjective;

	size_t							m_iCageModel;
	CParticleSystemInstanceHandle	m_hCageParticles;
};

#endif
