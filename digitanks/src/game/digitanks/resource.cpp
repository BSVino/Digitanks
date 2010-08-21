#include "resource.h"

#include <sstream>

#include <renderer/renderer.h>
#include <game/game.h>
#include <renderer/particles.h>

#include <GL/glew.h>

void CResource::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem(L"electronode-spark");
	PrecacheModel(L"models/structures/electronode.obj");
}

void CResource::Spawn()
{
	BaseClass::Spawn();

	m_iProduction = 4;
	m_bTakeDamage = false;

	SetModel(L"models/structures/electronode.obj");

	m_iSpark = ~0;
}

void CResource::Think()
{
	if (m_iSpark != ~0)
	{
		CSystemInstance* pInstance = CParticleSystemLibrary::Get()->GetInstance(m_iSpark);

		if (!pInstance)
			m_iSpark = ~0;
		else
		{
			pInstance->SetOrigin(GetOrigin());

			if (HasCollector())
				pInstance->SetColor(GetCollector()->GetTeam()->GetColor());
			else
				pInstance->SetColor(Color(255,255,255));

			if (GetVisibility() <= 0)
			{
				CParticleSystemLibrary::Get()->StopInstance(m_iSpark);
				m_iSpark = ~0;
			}
		}
	}
	else
	{
		if (GetVisibility() > 0)
		{
			m_iSpark = CParticleSystemLibrary::AddInstance(L"electronode-spark", GetOrigin());

			if (HasCollector())
				CParticleSystemLibrary::Get()->GetInstance(m_iSpark)->SetColor(GetCollector()->GetTeam()->GetColor());
			else
				CParticleSystemLibrary::Get()->GetInstance(m_iSpark)->SetColor(Color(255,255,255));
		}
	}
}

void CResource::UpdateInfo(std::string& sInfo)
{
	std::stringstream s;

	s << "ELECTRONODE\n";
	s << "Digital resource\n \n";

	s << "Yield: " << (int)(GetProduction()) << " Power\n";

	sInfo = s.str();
}

void CResource::ModifyContext(class CRenderingContext* pContext)
{
	BaseClass::ModifyContext(pContext);

	if (HasCollector())
		pContext->SetColorSwap(GetCollector()->GetTeam()->GetColor());
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
