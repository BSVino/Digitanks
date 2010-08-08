#include "resource.h"

#include <sstream>

#include <renderer/renderer.h>
#include <game/game.h>

#include <GL/glew.h>

REGISTER_ENTITY(CResource);

void CResource::Precache()
{
	BaseClass::Precache();

	PrecacheModel(L"models/structures/electronode.obj");
}

void CResource::Spawn()
{
	BaseClass::Spawn();

	m_iProduction = 4;
	m_bTakeDamage = false;

	SetModel(L"models/structures/electronode.obj");
}

void CResource::UpdateInfo(std::string& sInfo)
{
	std::stringstream s;

	s << "ELECTRONODE\n";
	s << "Digital resource\n \n";

	s << "Yield: " << (int)(GetProduction()) << " Power\n";

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
