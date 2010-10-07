#include "powerup.h"

#include <color.h>
#include <models/models.h>
#include <renderer/renderer.h>

#include "digitank.h"
#include "digitanksgame.h"

NETVAR_TABLE_BEGIN(CPowerup);
NETVAR_TABLE_END();

CPowerup::CPowerup()
{
	SetCollisionGroup(CG_POWERUP);

	SetModel(L"models/powerup.obj");
}

void CPowerup::Precache()
{
	PrecacheModel(L"models/powerup.obj", false);
}

EAngle CPowerup::GetRenderAngles() const
{
	float flRotate = fmod(Game()->GetGameTime(), 3.6f)*100.0f;
	return EAngle(0, flRotate, 0);
}

void CPowerup::PreRender()
{
	CModel* pModel = CModelLibrary::Get()->GetModel(GetModel());

	Vector clrPowerup(255, 255, 255);
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

		if ((pTank->GetDesiredMove() - GetOrigin()).LengthSqr() < GetBoundingRadius()*GetBoundingRadius())
			clrPowerup = Vector(0, 255, 0);

		if (pTank->GetPreviewMovePower() <= pTank->GetTotalMovementPower() && (pTank->GetPreviewMove() - GetOrigin()).LengthSqr() < GetBoundingRadius()*GetBoundingRadius())
			clrPowerup = Vector(0, 255, 0);
	}

	pModel->m_pScene->GetMaterial(pModel->m_pScene->FindMaterial(L"Powerup"))->m_vecDiffuse = clrPowerup;
}

void CPowerup::ModifyContext(class CRenderingContext* pContext)
{
	BaseClass::ModifyContext(pContext);

	pContext->SetBlend(BLEND_ADDITIVE);
}
