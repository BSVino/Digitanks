#include "digitanksentity.h"

#include <GL/glew.h>

#include <maths.h>

#include <renderer/renderer.h>
#include <renderer/dissolver.h>

#include "digitanksgame.h"

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

void CDigitanksEntity::StartTurn()
{
	float flHealth = m_flHealth;
	m_flHealth = Approach(m_flTotalHealth, m_flHealth, HealthRechargeRate());

	if (flHealth - m_flHealth < 0)
		DigitanksGame()->OnTakeDamage(this, NULL, NULL, flHealth - m_flHealth, true, false);

	m_ahSupplyLinesIntercepted.clear();
}

void CDigitanksEntity::EndTurn()
{
	InterceptSupplyLines();
}

void CDigitanksEntity::InterceptSupplyLines()
{
	// Haha... no.
	if (dynamic_cast<CSupplyLine*>(this))
		return;

	if (!GetTeam())
		return;

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (!pEntity)
			continue;

		CSupplyLine* pSupplyLine = dynamic_cast<CSupplyLine*>(pEntity);
		if (!pSupplyLine)
			continue;

		if (pSupplyLine->GetTeam() == GetTeam())
			continue;

		if (!pSupplyLine->GetTeam())
			continue;

		if (!pSupplyLine->GetSupplier() || !pSupplyLine->GetEntity())
			continue;

		Vector vecEntity = GetOrigin();
		vecEntity.y = 0;

		Vector vecSupplier = pSupplyLine->GetSupplier()->GetOrigin();
		vecSupplier.y = 0;

		Vector vecUnit = pSupplyLine->GetEntity()->GetOrigin();
		vecUnit.y = 0;

		if (DistanceToLineSegment(vecEntity, vecSupplier, vecUnit) > GetBoundingRadius()+4)
			continue;

		bool bFound = false;
		for (size_t j = 0; j < m_ahSupplyLinesIntercepted.size(); j++)
		{
			if (pSupplyLine == m_ahSupplyLinesIntercepted[j])
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			pSupplyLine->Intercept(0.2f);
			m_ahSupplyLinesIntercepted.push_back(pSupplyLine);
		}
	}
}

CDigitanksTeam* CDigitanksEntity::GetDigitanksTeam() const
{
	return dynamic_cast<CDigitanksTeam*>(BaseClass::GetTeam());
}

bool CDigitanksEntity::ShouldRender() const
{
	return GetVisibility(DigitanksGame()->GetLocalDigitanksTeam()) > 0;
}

void CDigitanksEntity::RenderVisibleArea()
{
	if (VisibleRange() == 0)
		return;

	CRenderingContext c(Game()->GetRenderer());
	c.Translate(GetOrigin());
	c.Scale(VisibleRange(), VisibleRange(), VisibleRange());
	c.RenderSphere();
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
	//glutSolidSphere(GetBoundingRadius(), 8, 4);
#endif
}

