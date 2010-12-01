#include "powerup.h"

#include <color.h>
#include <mtrand.h>

#include <models/models.h>
#include <renderer/renderer.h>

#include "units/digitank.h"
#include "digitanksgame.h"

NETVAR_TABLE_BEGIN(CPowerup);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CPowerup);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, powerup_type_t, m_ePowerupType);
SAVEDATA_TABLE_END();

void CPowerup::Precache()
{
	PrecacheModel(L"models/powerup.obj", false);
	PrecacheModel(L"models/powerup-airstrike.obj", false);
}

void CPowerup::Spawn()
{
	BaseClass::Spawn();

	SetCollisionGroup(CG_POWERUP);
	if (RandomInt(0, 3) == 0)
		m_ePowerupType = POWERUP_AIRSTRIKE;
	else
		m_ePowerupType = POWERUP_BONUS;

	switch (m_ePowerupType)
	{
	default:
	case POWERUP_BONUS:
		SetModel(L"models/powerup.obj");
		break;

	case POWERUP_AIRSTRIKE:
		SetModel(L"models/powerup-airstrike.obj");
		break;
	}
}

EAngle CPowerup::GetRenderAngles() const
{
	float flRotate = fmod(GameServer()->GetGameTime(), 3.6f)*100.0f;
	return EAngle(0, flRotate, 0);
}

void CPowerup::PreRender()
{
	CModel* pModel = CModelLibrary::Get()->GetModel(GetModel());

	Vector clrPowerup(1, 1, 1);
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (pEntity == NULL)
			continue;

		CDigitank* pTank = dynamic_cast<CDigitank*>(pEntity);
		if (!pTank)
			continue;

		if (!pTank->CanGetPowerups())
			continue;

		if ((pTank->GetOrigin() - GetOrigin()).LengthSqr() < GetBoundingRadius()*GetBoundingRadius())
		{
			clrPowerup = Vector(0, 1, 0);
			break;
		}

		if (pTank->GetPreviewMovePower() <= pTank->GetTotalMovementPower() && (pTank->GetPreviewMove() - GetOrigin()).LengthSqr() < GetBoundingRadius()*GetBoundingRadius())
		{
			clrPowerup = Vector(0, 1, 0);
			break;
		}
	}

	pModel->m_pScene->GetMaterial(pModel->m_pScene->FindMaterial(L"Powerup"))->m_vecDiffuse = clrPowerup;
}

void CPowerup::ModifyContext(class CRenderingContext* pContext)
{
	BaseClass::ModifyContext(pContext);

	pContext->SetBlend(BLEND_ADDITIVE);
	pContext->SetDepthMask(false);
}
