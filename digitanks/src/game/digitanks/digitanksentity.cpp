#include "digitanksentity.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <renderer/renderer.h>
#include <renderer/dissolver.h>

#include "digitanksgame.h"

REGISTER_ENTITY(CDigitanksEntity);

void CDigitanksEntity::Spawn()
{
	BaseClass::Spawn();

	m_bTakeDamage = true;
	m_flTotalHealth = TotalHealth();
	m_flHealth = m_flTotalHealth;
}

void CDigitanksEntity::Think()
{
	BaseClass::Think();

	if (!IsAlive() && Game()->GetGameTime() > m_flTimeKilled + 1.0f)
	{
		CModelDissolver::AddModel(this);
		Game()->Delete(this);
	}
}

CDigitanksTeam* CDigitanksEntity::GetDigitanksTeam() const
{
	return dynamic_cast<CDigitanksTeam*>(BaseClass::GetTeam());
}

void CDigitanksEntity::RenderVisibleArea()
{
	if (VisibleRange() == 0)
		return;

	CRenderingContext c(Game()->GetRenderer());
	c.Translate(GetOrigin());
	glutSolidSphere(VisibleRange(), 20, 10);
}

float CDigitanksEntity::GetVisibility(CDigitanksTeam* pTeam) const
{
	if (!DigitanksGame()->ShouldRenderFogOfWar())
		return 1;

	return pTeam->GetEntityVisibility(GetHandle());
}

float CDigitanksEntity::GetVisibility() const
{
	return GetVisibility(CDigitanksGame::GetLocalDigitanksTeam());
}

void CDigitanksEntity::ModifyContext(CRenderingContext* pContext)
{
	BaseClass::ModifyContext(pContext);

	CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentTeam();

	float flVisibility = GetVisibility();
	if (flVisibility < 1)
	{
		pContext->SetAlpha(flVisibility);
		pContext->SetBlend(BLEND_ALPHA);
	}
}

void CDigitanksEntity::OnRender()
{
	BaseClass::OnRender();

#if defined(_DEBUG) && defined(SHOW_BOUNDING_SPHERES)
	CRenderingContext r(Game()->GetRenderer());
	r.SetBlend(BLEND_ALPHA);
	r.SetColor(Color(255, 255, 255, 100));
	r.SetAlpha(0.2f);
	glutSolidSphere(GetBoundingRadius(), 8, 4);
#endif
}
