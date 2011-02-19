#include "powerup.h"

#include <color.h>
#include <mtrand.h>

#include <models/models.h>
#include <renderer/renderer.h>

#include "units/digitank.h"
#include "digitanksgame.h"

REGISTER_ENTITY(CPowerup);

NETVAR_TABLE_BEGIN(CPowerup);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CPowerup);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, powerup_type_t, m_ePowerupType);
SAVEDATA_TABLE_END();

void CPowerup::Precache()
{
	PrecacheModel(L"models/powerup.obj", false);
	PrecacheModel(L"models/powerup-airstrike.obj", false);
	PrecacheModel(L"models/powerup-tank.obj", false);
	PrecacheModel(L"models/powerup-missiledefense.obj", false);
}

void CPowerup::Spawn()
{
	BaseClass::Spawn();

	SetCollisionGroup(CG_POWERUP);
	if (RandomInt(0, 2) == 0)
	{
		switch (RandomInt(0, 4))
		{
		case 0:
		case 1:
			m_ePowerupType = POWERUP_AIRSTRIKE;
			break;

		case 4:
			m_ePowerupType = POWERUP_TANK;
			break;
		}
	}
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

	case POWERUP_TANK:
		SetModel(L"models/powerup-tank.obj");
		break;

	case POWERUP_MISSILEDEFENSE:
		SetModel(L"models/powerup-missiledefense.obj");
		break;
	}
}

eastl::string16 CPowerup::GetName()
{
	switch (m_ePowerupType)
	{
	default:
	case POWERUP_BONUS:
		return L"Promotion Powerup";

	case POWERUP_AIRSTRIKE:
		return L"Airstrike Powerup";

	case POWERUP_TANK:
		return L"New Unit Powerup";

	case POWERUP_MISSILEDEFENSE:
		return L"Missile Defense Powerup";
	}
}

EAngle CPowerup::GetRenderAngles() const
{
	float flRotate = fmod(GameServer()->GetGameTime(), 3.6f)*100.0f;
	return EAngle(0, flRotate, 0);
}

Vector CPowerup::GetRenderOrigin() const
{
	Vector vecOrigin = BaseClass::GetRenderOrigin();
	vecOrigin.y += RemapValClamped(GameServer()->GetGameTime() - GetSpawnTime(), 0, 3, 100, 0) + RemapVal(Lerp(Oscillate(GameServer()->GetGameTime() + GetSpawnTime(), 3), 0.8f), 0, 1, 3, 4);
	return vecOrigin;
}

void CPowerup::PreRender(bool bTransparent)
{
	if (!bTransparent)
		return;

	CModel* pModel = CModelLibrary::Get()->GetModel(GetModel());

	Vector clrPowerup(1, 1, 1);
	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
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

		if (pTank->GetPreviewMovePower() <= pTank->GetRemainingMovementEnergy() && (pTank->GetPreviewMove() - GetOrigin()).LengthSqr() < GetBoundingRadius()*GetBoundingRadius())
		{
			clrPowerup = Vector(0, 1, 0);
			break;
		}
	}
}

void CPowerup::ModifyContext(class CRenderingContext* pContext, bool bTransparent)
{
	BaseClass::ModifyContext(pContext, bTransparent);

	pContext->SetBlend(BLEND_ADDITIVE);
	pContext->SetDepthMask(false);

	pContext->SetAlpha(RemapValClamped(GameServer()->GetGameTime() - GetSpawnTime(), 0, 3, 0, 1));
}
