#ifndef DT_RESOURCE_H
#define DT_RESOURCE_H

#include "structure.h"
#include "collector.h"
#include <renderer/particles.h>

class CResourceNode : public CStructure
{
	REGISTER_ENTITY_CLASS(CResourceNode, CStructure);

public:
	virtual const TFloat		GetBoundingRadius() const { return 2; };

	virtual void				Precache();
	virtual void				Spawn();
	virtual void				Think();

	virtual bool				GetsConcealmentBonus() const { return false; };

	virtual void				UpdateInfo(tstring& sInfo);

	virtual void				ModifyContext(class CRenderingContext* pContext) const;
	virtual void				PostRender() const;

	resource_t					GetResource() { return RESOURCE_ELECTRONODE; };
	virtual bool				ShowHealthBar() { return false; }

	bool						HasCollector() const { return m_hCollector != NULL; }
	class CCollector*			GetCollector() const { return m_hCollector; }
	void						SetCollector(class CCollector* pCollector) { m_hCollector = pCollector; }

	virtual tstring				GetEntityName() const { return "Electronode"; };
	virtual unittype_t			GetUnitType() const { return STRUCTURE_ELECTRONODE; };
	virtual bool				IsRammable() const { return false; }

	virtual float				AvailableArea(int iArea) const;
	virtual int					GetNumAvailableAreas() const { return 1; };
	virtual bool				IsAvailableAreaActive(int iArea) const;

	static CResourceNode*		FindClosestResource(Vector vecPoint, resource_t eResource);

protected:
	CNetworkedHandle<CCollector> m_hCollector;

	CParticleSystemInstanceHandle m_hSparkParticles;
};

#endif
