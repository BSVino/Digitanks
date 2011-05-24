#include "introtank.h"

#include <tinker/application.h>
#include <renderer/renderer.h>
#include <models/models.h>

REGISTER_ENTITY(CIntroTank);

NETVAR_TABLE_BEGIN(CIntroTank);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CIntroTank);
	SAVEDATA_OMIT(m_flCurrentTurretYaw);
	SAVEDATA_OMIT(m_flGoalTurretYaw);
	SAVEDATA_OMIT(m_iTurretModel);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CIntroTank);
INPUTS_TABLE_END();

CIntroTank::CIntroTank()
{
	m_flCurrentTurretYaw = 0;
	m_flGoalTurretYaw = 0;
	m_iTurretModel = ~0;
}

void CIntroTank::ModifyContext(class CRenderingContext* pContext, bool bTransparent)
{
	BaseClass::ModifyContext(pContext, bTransparent);

	float flWidth = (float)CApplication::Get()->GetWindowWidth();
	float flHeight = (float)CApplication::Get()->GetWindowHeight();

	float flScale = 10*flWidth/flHeight;

	pContext->Scale(flScale, flScale, flScale);
}

void CIntroTank::OnRender(class CRenderingContext* pContext, bool bTransparent)
{
	BaseClass::OnRender(pContext, bTransparent);

	if (bTransparent)
		return;

	CRenderingContext r(GameServer()->GetRenderer());

	r.Translate(Vector(-0.0f, 0.810368f, 0));

	float flSpeed = RemapValClamped(fabs(AngleDifference(m_flGoalTurretYaw, m_flCurrentTurretYaw)), 30, 90, 20, 40);
	m_flCurrentTurretYaw = AngleApproach(m_flGoalTurretYaw, m_flCurrentTurretYaw, flSpeed);

	r.Rotate(-m_flCurrentTurretYaw, Vector(0, 1, 0));

	if (GetTeam())
		r.SetColorSwap(GetTeam()->GetColor());

	r.RenderModel(m_iTurretModel);
}
