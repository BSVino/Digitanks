#include "digitanksentity.h"

#include <GL/glew.h>

#include <maths.h>
#include <mtrand.h>
#include <geometry.h>

#include <renderer/renderer.h>
#include <renderer/dissolver.h>

#include "digitanksgame.h"

NETVAR_TABLE_BEGIN(CDigitanksEntity);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CDigitanksEntity);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, CEntityHandle<class CSupplyLine>, m_ahSupplyLinesIntercepted);
SAVEDATA_TABLE_END();

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

	if (!IsAlive() && GameServer()->GetGameTime() > m_flTimeKilled + 1.0f)
	{
		GameServer()->Delete(this);

		if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY)
		{
			switch (RandomInt(0, 5))
			{
			case 0:
			{
				bool bColorSwap = GetTeam() && (dynamic_cast<CStructure*>(this) || dynamic_cast<CDigitank*>(this));
				CModelDissolver::AddModel(this, bColorSwap?&GetTeam()->GetColor():NULL);
				break;
			}

			case 1:
			{
				CProjectile* pProjectile = GameServer()->Create<CLargeShell>("CLargeShell");
				pProjectile->SetOwner(NULL);
				pProjectile->SetOrigin(GetOrigin());
				pProjectile->Explode();
				break;
			}

			case 2:
			{
				CProjectile* pProjectile = GameServer()->Create<CAOEShell>("CAOEShell");
				pProjectile->SetOwner(NULL);
				pProjectile->SetOrigin(GetOrigin());
				pProjectile->Explode();
				break;
			}

			case 3:
			{
				CProjectile* pProjectile = GameServer()->Create<CClusterBomb>("CClusterBomb");
				pProjectile->SetOwner(NULL);
				pProjectile->SetOrigin(GetOrigin());
				pProjectile->Explode();
				break;
			}

			case 4:
			{
				CProjectile* pProjectile = GameServer()->Create<CEarthshaker>("CEarthshaker");
				pProjectile->SetOwner(NULL);
				pProjectile->SetOrigin(GetOrigin());
				pProjectile->Explode();
				break;
			}

			case 5:
			{
				CProjectile* pProjectile = GameServer()->Create<CTractorBomb>("CTractorBomb");
				pProjectile->SetOwner(NULL);
				pProjectile->SetOrigin(GetOrigin());
				pProjectile->Explode();
				break;
			}
			}
		}
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

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
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
	return GetVisibility(DigitanksGame()->GetCurrentLocalDigitanksTeam()) > 0;
}

void CDigitanksEntity::RenderVisibleArea()
{
	if (VisibleRange() == 0)
		return;

	CRenderingContext c(GameServer()->GetRenderer());
	c.Translate(GetOrigin());
	c.Scale(VisibleRange(), VisibleRange(), VisibleRange());
	c.RenderSphere();
}

float CDigitanksEntity::GetVisibility(CDigitanksTeam* pTeam) const
{
	float flConceal = 0.0f;
	if (GetsConcealmentBonus())
	{
		if (DigitanksGame()->GetTerrain()->GetBit(CTerrain::WorldToArraySpace(m_vecOrigin.Get().x), CTerrain::WorldToArraySpace(m_vecOrigin.Get().z), TB_TREE))
			flConceal = 0.7f;
	}

	if (!DigitanksGame()->ShouldRenderFogOfWar())
		return 1 - flConceal;

	if (!pTeam)
		return 0;

	if (GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam())
		return 1 - flConceal;

	float flVisibility = pTeam->GetEntityVisibility(GetHandle()) - flConceal;

	if (flVisibility < 0)
		return 0;

	return flVisibility;
}

float CDigitanksEntity::GetVisibility() const
{
	if (!DigitanksGame())
		return 0;

	return GetVisibility(DigitanksGame()->GetCurrentLocalDigitanksTeam());
}

float CDigitanksEntity::VisibleRange() const
{
	if (TreesReduceVisibility() && DigitanksGame()->GetTerrain()->IsPointInTrees(GetOrigin()))
		return BaseVisibleRange()/2;

	return BaseVisibleRange();
}

void CDigitanksEntity::ModifyContext(CRenderingContext* pContext, bool bTransparent)
{
	BaseClass::ModifyContext(pContext, bTransparent);

	CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentTeam();

	float flVisibility = GetVisibility();
	if (flVisibility < 1)
	{
		pContext->SetAlpha(flVisibility);
		pContext->SetBlend(BLEND_ALPHA);
	}
}
