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
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CParticleSystemInstanceHandle, m_hSparkParticles);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CResource);
INPUTS_TABLE_END();

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

	m_hSparkParticles.SetSystem(L"electronode-spark", GetOrigin());

	m_bConstructing = false;
}

void CResource::Think()
{
	BaseClass::Think();

	m_hSparkParticles.SetActive(GetVisibility() > 0);

	CSystemInstance* pInstance = CParticleSystemLibrary::Get()->GetInstance(m_hSparkParticles.GetInstance());
	if (pInstance)
	{
		pInstance->SetOrigin(GetOrigin());

		if (HasCollector() && GetCollector()->GetTeam())
			pInstance->SetColor(GetCollector()->GetTeam()->GetColor());
		else
			pInstance->SetColor(Color(195,176,78));
	}
}

void CResource::UpdateInfo(eastl::string16& s)
{
	s = L"ELECTRONODE\n";
	s += L"Digital resource\n \n";

	s += L"Team: Neutral\n \n";

	s += L"Build a Capacitor or Power Supply Unit to harness this Electronode's Power resource\n";
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

			if (GetCollector())
			{
				CCollector* pCollector = GetCollector();

				if (pCollector->GetTeam() != pTeam)
					bShowPreview = false;

				if (ePreviewStructure == STRUCTURE_BATTERY)
					bShowPreview = false;

				if (ePreviewStructure == STRUCTURE_PSU && pCollector->GetUnitType() == STRUCTURE_PSU)
					bShowPreview = false;

				// Putting a PSU on a battery is okay.
			}
		}

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

float CResource::AvailableArea(int iArea) const
{
	return 5;
}

bool CResource::IsAvailableAreaActive(int iArea) const
{
	if (DigitanksGame()->GetControlMode() != MODE_BUILD)
		return false;

	CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();
	if (!pTeam)
		return false;

	if (!pTeam->GetPrimaryCPU())
		return false;

	unittype_t ePreviewStructure = pTeam->GetPrimaryCPU()->GetPreviewStructure();

	if (GetCollector())
	{
		CCollector* pCollector = GetCollector();

		if (pCollector->GetTeam() != pTeam)
			return false;

		if (ePreviewStructure == STRUCTURE_BATTERY)
			return false;

		if (ePreviewStructure == STRUCTURE_PSU && pCollector->GetUnitType() == STRUCTURE_PSU)
			return false;

		// Putting a PSU on a battery is okay.
	}

	if (ePreviewStructure != STRUCTURE_PSU && ePreviewStructure != STRUCTURE_BATTERY)
		return false;

	if (CSupplier::GetDataFlow(GetOrigin(), pTeam) <= 0)
		return false;

	return true;
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
