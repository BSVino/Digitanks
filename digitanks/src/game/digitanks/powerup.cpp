#include "powerup.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <color.h>
#include <models/models.h>
#include <renderer/renderer.h>

#include "digitank.h"
#include "digitanksgame.h"

REGISTER_ENTITY(CPowerup);

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
	int iRotate = glutGet(GLUT_ELAPSED_TIME)%3600;
	return EAngle(0, iRotate/10.0f, 0);
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
