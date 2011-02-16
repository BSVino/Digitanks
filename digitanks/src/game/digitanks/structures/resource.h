#ifndef DT_RESOURCE_H
#define DT_RESOURCE_H

#include "structure.h"
#include "collector.h"

class CResource : public CStructure
{
	REGISTER_ENTITY_CLASS(CResource, CStructure);

public:
	virtual float				GetBoundingRadius() const { return 2; };

	virtual void				Precache();
	virtual void				Spawn();
	virtual void				Think();

	virtual bool				GetsConcealmentBonus() const { return false; };

	virtual void				UpdateInfo(eastl::string16& sInfo);

	virtual void				ModifyContext(class CRenderingContext* pContext, bool bTransparent);
	virtual void				PostRender(bool bTransparent);

	resource_t					GetResource() { return RESOURCE_ELECTRONODE; };
	virtual bool				ShowHealthBar() { return false; }

	bool						HasCollector() { return m_hCollector != NULL; }
	class CCollector*			GetCollector() const { return m_hCollector; }
	void						SetCollector(class CCollector* pCollector) { m_hCollector = pCollector; }

	virtual eastl::string16		GetName() { return L"Electronode"; };
	virtual unittype_t			GetUnitType() const { return STRUCTURE_ELECTRONODE; };
	virtual bool				IsRammable() const { return false; }

	virtual float				AvailableArea() const;
	virtual int					GetNumAvailableAreas() const { return 1; };
	virtual bool				IsAvailableAreaActive(int iArea) const;

	static CResource*			FindClosestResource(Vector vecPoint, resource_t eResource);

protected:
	CNetworkedHandle<CCollector> m_hCollector;

	size_t						m_iSpark;
};

#endif
