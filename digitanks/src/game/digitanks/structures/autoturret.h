#ifndef DT_AUTOTURRET_H
#define DT_AUTOTURRET_H

#include "structure.h"

class CAutoTurret : public CStructure
{
	REGISTER_ENTITY_CLASS(CAutoTurret, CStructure);

public:
	virtual float				GetBoundingRadius() const { return 3; };

	virtual void				Precache();
	virtual void				Spawn();

	virtual void				ModifyContext(class CRenderingContext* pContext, bool bTransparent);

	virtual void				StartTurn();
	virtual void				EndTurn();

	eastl::vector<CDigitank*>	GetTargets();

	void						Fire();

	virtual void				SetupMenu(menumode_t eMenuMode);

	virtual void				UpdateInfo(eastl::string16& sInfo);

	static float				DefenseRadius() { return 50; };
	virtual float				BaseVisibleRange() const { return DefenseRadius(); };

	virtual float				AvailableArea(int iArea) const;
	virtual int					GetNumAvailableAreas() const { return 2; };
	virtual bool				IsAvailableAreaActive(int iArea) const;

	virtual size_t				InitialTurnsToConstruct() { return 2; };
	virtual float				TotalHealth() const { return 150; };

	virtual eastl::string16		GetEntityName() { return L"Firewall"; };
	virtual unittype_t			GetUnitType() const { return STRUCTURE_AUTOTURRET; };

protected:
	CNetworkedVariable<bool>	m_bHasFired;

};

#endif
