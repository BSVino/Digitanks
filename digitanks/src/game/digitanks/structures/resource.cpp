#include "resource.h"

#include <sstream>

#include <renderer/renderer.h>
#include <game/game.h>
#include <renderer/particles.h>
#include <models/models.h>

#include <GL/glew.h>

#include <game/digitanks/digitanksgame.h>
#include "structure.h"

REGISTER_ENTITY(CResource);

NETVAR_TABLE_BEGIN(CResource);
	NETVAR_DEFINE(CEntityHandle<CCollector>, m_hCollector);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CResource);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CCollector>, m_hCollector);
SAVEDATA_TABLE_END();

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

	m_bConstructing = false;
}

void CResource::Think()
{
	BaseClass::Think();

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

void CResource::UpdateInfo(eastl::string16& s)
{
	s = L"ELECTRONODE\n";
	s += L"Digital resource\n \n";

	s += L"Build a Battery or Power Supply Unit to harness this Electronode's Power resource\n";
}

void CResource::ModifyContext(class CRenderingContext* pContext, bool bTransparent)
{
	BaseClass::ModifyContext(pContext, bTransparent);

	if (HasCollector() && GetCollector()->GetTeam())
		pContext->SetColorSwap(GetCollector()->GetTeam()->GetColor());
}

void CResource::PostRender(bool bTransparent)
{
	BaseClass::PostRender(bTransparent);

	if (bTransparent && DigitanksGame()->GetControlMode() == MODE_BUILD)
	{
		bool bShowPreview = false;

		CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();

		if (pTeam && pTeam->GetPrimaryCPU())
		{
			unittype_t ePreviewStructure = pTeam->GetPrimaryCPU()->GetPreviewStructure();
			if (ePreviewStructure == STRUCTURE_PSU || ePreviewStructure == STRUCTURE_BATTERY)
			{
				if (CSupplier::GetDataFlow(GetOrigin(), pTeam) > 0)
					bShowPreview = true;
			}
		}

		if (GetCollector())
			bShowPreview = false;

		if (bShowPreview)
		{
			CRenderingContext r(GameServer()->GetRenderer());
			r.Translate(GetOrigin());

			r.SetColorSwap(Color(255, 255, 255));
			r.SetAlpha(0.5f);
			r.SetBlend(BLEND_ALPHA);

			size_t iModel = 0;
			switch (pTeam->GetPrimaryCPU()->GetPreviewStructure())
			{
			case STRUCTURE_BATTERY:
				iModel = CModelLibrary::Get()->FindModel(L"models/structures/battery.obj");
				break;

			case STRUCTURE_PSU:
				iModel = CModelLibrary::Get()->FindModel(L"models/structures/psu.obj");
				break;
			}

			r.RenderModel(iModel);
		}
	}
}

float CResource::BuildableArea() const
{
	if (DigitanksGame()->GetControlMode() != MODE_BUILD)
		return 0;

	if (GetCollector())
		return 0;

	CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();

	if (!pTeam)
		return 0;

	if (!pTeam->GetPrimaryCPU())
		return 0;

	unittype_t ePreviewStructure = pTeam->GetPrimaryCPU()->GetPreviewStructure();
	if (ePreviewStructure != STRUCTURE_PSU && ePreviewStructure != STRUCTURE_BATTERY)
		return 0;

	if (CSupplier::GetDataFlow(GetOrigin(), pTeam) <= 0)
		return 0;

	return 5;
}

CResource* CResource::FindClosestResource(Vector vecPoint, resource_t eResource)
{
	CResource* pClosest = NULL;

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
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
