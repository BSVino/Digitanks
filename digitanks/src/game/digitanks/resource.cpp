#include "resource.h"

#include <sstream>

#include <renderer/renderer.h>
#include <game/game.h>
#include <renderer/particles.h>

#include <GL/glew.h>

NETVAR_TABLE_BEGIN(CResource);
NETVAR_TABLE_END();

void CResource::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem(L"electronode-spark");
	PrecacheModel(L"models/structures/electronode.obj", true);
}

void CResource::Spawn()
{
	BaseClass::Spawn();

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

			if (HasCollector() && GetCollector()->GetTeam())
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

			if (HasCollector() && GetCollector()->GetTeam())
				CParticleSystemLibrary::Get()->GetInstance(m_iSpark)->SetColor(GetCollector()->GetTeam()->GetColor());
			else
				CParticleSystemLibrary::Get()->GetInstance(m_iSpark)->SetColor(Color(255,255,255));
		}
	}
}

void CResource::UpdateInfo(std::wstring& sInfo)
{
	std::wstringstream s;

	s << L"ELECTRONODE\n";
	s << L"Digital resource\n \n";

	s << L"Build a Battery or Power Supply Unit to harness this Electronode's Power resource\n";

	sInfo = s.str();
}

void CResource::ModifyContext(class CRenderingContext* pContext)
{
	BaseClass::ModifyContext(pContext);

	if (HasCollector() && GetCollector()->GetTeam())
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
