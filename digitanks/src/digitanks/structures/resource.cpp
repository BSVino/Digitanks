#include "resource.h"

#include <sstream>

#include <renderer/game_renderer.h>
#include <renderer/game_renderingcontext.h>
#include <game/entities/game.h>
#include <renderer/particles.h>
#include <models/models.h>
#include <game/gameserver.h>

#include <digitanksgame.h>
#include "structure.h"

REGISTER_ENTITY(CResourceNode);

NETVAR_TABLE_BEGIN(CResourceNode);
	NETVAR_DEFINE(CEntityHandle<CCollector>, m_hCollector);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CResourceNode);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CCollector>, m_hCollector);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CParticleSystemInstanceHandle, m_hSparkParticles);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CResourceNode);
INPUTS_TABLE_END();

void CResourceNode::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem("electronode-spark");
	PrecacheModel("models/structures/electronode.toy");
}

void CResourceNode::Spawn()
{
	BaseClass::Spawn();

	m_bTakeDamage = false;

	SetModel("models/structures/electronode.toy");

	m_hSparkParticles.SetSystem("electronode-spark", GetGlobalOrigin());

	m_bConstructing = false;
}

void CResourceNode::Think()
{
	BaseClass::Think();

	m_hSparkParticles.SetActive(GetVisibility() > 0);

	CSystemInstance* pInstance = CParticleSystemLibrary::Get()->GetInstance(m_hSparkParticles.GetInstance());
	if (pInstance)
	{
		pInstance->SetOrigin(GetGlobalOrigin());

		if (HasCollector() && GetCollector()->GetPlayerOwner())
			pInstance->SetColor(GetCollector()->GetPlayerOwner()->GetColor());
		else
			pInstance->SetColor(Color(195,176,78));
	}
}

void CResourceNode::UpdateInfo(tstring& s)
{
	s = "ELECTRONODE\n";
	s += "Digital resource\n \n";

	s += "Team: Neutral\n \n";

	s += "Build a Capacitor or Power Supply Unit to harness this Electronode's Power resource\n";
}

void CResourceNode::ModifyContext(class CRenderingContext* pContext) const
{
	BaseClass::ModifyContext(pContext);

	if (HasCollector() && GetCollector()->GetPlayerOwner())
	{
		pContext->SetUniform("bColorSwapInAlpha", true);
		pContext->SetUniform("vecColorSwap", GetCollector()->GetPlayerOwner()->GetColor());
	}
}

void CResourceNode::PostRender() const
{
	BaseClass::PostRender();

	if (GameServer()->GetRenderer()->IsRenderingTransparent() && DigitanksGame()->GetControlMode() == MODE_BUILD)
	{
		bool bShowPreview = false;

		CDigitanksPlayer* pTeam = DigitanksGame()->GetCurrentLocalDigitanksPlayer();

		if (pTeam && pTeam->GetPrimaryCPU())
		{
			unittype_t ePreviewStructure = pTeam->GetPrimaryCPU()->GetPreviewStructure();
			if (ePreviewStructure == STRUCTURE_PSU || ePreviewStructure == STRUCTURE_BATTERY)
			{
				if (CSupplier::GetDataFlow(GetGlobalOrigin(), pTeam) > 0)
					bShowPreview = true;
			}

			if (GetCollector())
			{
				CCollector* pCollector = GetCollector();

				if (pCollector->GetPlayerOwner() != pTeam)
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
			CGameRenderingContext r(GameServer()->GetRenderer(), true);
			r.Translate(GetGlobalOrigin());

			r.SetUniform("bColorSwapInAlpha", true);
			r.SetUniform("vecColorSwap", Color(255, 255, 255));
			r.SetAlpha(0.5f);
			r.SetBlend(BLEND_ALPHA);

			size_t iModel = 0;
			switch (pTeam->GetPrimaryCPU()->GetPreviewStructure())
			{
			case STRUCTURE_BATTERY:
				iModel = CModelLibrary::Get()->FindModel("models/structures/battery.toy");
				break;

			case STRUCTURE_PSU:
				iModel = CModelLibrary::Get()->FindModel("models/structures/psu.toy");
				break;
			}

			r.RenderModel(iModel);
		}
	}
}

float CResourceNode::AvailableArea(int iArea) const
{
	return 5;
}

bool CResourceNode::IsAvailableAreaActive(int iArea) const
{
	if (DigitanksGame()->GetControlMode() != MODE_BUILD)
		return false;

	CDigitanksPlayer* pTeam = DigitanksGame()->GetCurrentLocalDigitanksPlayer();
	if (!pTeam)
		return false;

	if (!pTeam->GetPrimaryCPU())
		return false;

	unittype_t ePreviewStructure = pTeam->GetPrimaryCPU()->GetPreviewStructure();

	if (GetCollector())
	{
		CCollector* pCollector = GetCollector();

		if (pCollector->GetPlayerOwner() != pTeam)
			return false;

		if (ePreviewStructure == STRUCTURE_BATTERY)
			return false;

		if (ePreviewStructure == STRUCTURE_PSU && pCollector->GetUnitType() == STRUCTURE_PSU)
			return false;

		// Putting a PSU on a battery is okay.
	}

	if (ePreviewStructure != STRUCTURE_PSU && ePreviewStructure != STRUCTURE_BATTERY)
		return false;

	if (CSupplier::GetDataFlow(GetGlobalOrigin(), pTeam) <= 0)
		return false;

	return true;
}

CResourceNode* CResourceNode::FindClosestResource(Vector vecPoint, resource_t eResource)
{
	CResourceNode* pClosest = NULL;

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		CResourceNode* pResource = dynamic_cast<CResourceNode*>(pEntity);
		if (!pResource)
			continue;

		if (pResource->GetResource() != eResource)
			continue;

		if (!pClosest)
		{
			pClosest = pResource;
			continue;
		}

		if ((pResource->GetGlobalOrigin() - vecPoint).Length() < (pClosest->GetGlobalOrigin() - vecPoint).Length())
			pClosest = pResource;
	}

	return pClosest;
}
