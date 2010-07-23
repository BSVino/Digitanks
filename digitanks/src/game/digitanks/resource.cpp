#include "resource.h"

#include <sstream>

#include <renderer/renderer.h>
#include <game/game.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

REGISTER_ENTITY(CResource);

void CResource::Spawn()
{
	BaseClass::Spawn();

	m_iProduction = 4;
	m_bTakeDamage = false;
}

void CResource::OnRender()
{
	BaseClass::OnRender();

	if (GetVisibility() == 0)
		return;

	CRenderingContext c(Game()->GetRenderer());
	c.Scale(1, 3, 1);

	glutSolidCube(3);
}

void CResource::UpdateInfo(std::string& sInfo)
{
	std::stringstream s;

	s << "ELECTRONODE\n";
	s << "Digital resource\n \n";

	s << "Yield: " << (int)(GetProduction()) << "%\n";

	sInfo = s.str();
}

CResource* CResource::FindClosestResource(Vector vecPoint, resource_t eResource)
{
	CResource* pClosest = NULL;

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (!pEntity)
			continue;

		CResource* pResource = dynamic_cast<CResource*>(pEntity);
		if (!pResource)
			continue;

		if (pResource->GetResource() != eResource)
			continue;

		if (!pClosest)
		{
			pClosest = pResource;
			continue;
		}

		if ((pResource->GetOrigin() - vecPoint).Length() < (pClosest->GetOrigin() - vecPoint).Length())
			pClosest = pResource;
	}

	return pClosest;
}
