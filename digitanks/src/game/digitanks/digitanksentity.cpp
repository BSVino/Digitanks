#include "digitanksentity.h"

#include <GL/glew.h>

#include <maths.h>
#include <mtrand.h>
#include <geometry.h>

#include <renderer/renderer.h>
#include <renderer/dissolver.h>

#include "digitanksgame.h"
#include "wreckage.h"

REGISTER_ENTITY(CDigitanksEntity);

NETVAR_TABLE_BEGIN(CDigitanksEntity);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CDigitanksEntity);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, CEntityHandle<class CSupplyLine>, m_ahSupplyLinesIntercepted);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bVisibilityDirty);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flVisibility);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flNextDirtyArea);
SAVEDATA_TABLE_END();

void CDigitanksEntity::Spawn()
{
	BaseClass::Spawn();

	m_bTakeDamage = true;
	m_flTotalHealth = TotalHealth();
	m_flHealth = m_flTotalHealth;

	m_bVisibilityDirty = true;
	m_flNextDirtyArea = 0;

	CalculateVisibility();
}

void CDigitanksEntity::Think()
{
	BaseClass::Think();

	if (m_flNextDirtyArea > 0 && GameServer()->GetGameTime() > m_flNextDirtyArea)
	{
		CDigitanksEntity* pOther = this;

		while (true)
		{
			pOther = CBaseEntity::FindClosest<CDigitanksEntity>(GetOrigin(), pOther);

			if (!pOther)
				break;

			if (pOther == this)
				continue;

			if (pOther->Distance(GetOrigin()) > VisibleRange() + DigitanksGame()->FogPenetrationDistance())
				break;

			pOther->DirtyVisibility();
		}

		m_flNextDirtyArea = 0;
	}

	if (CNetwork::IsHost() && !IsAlive() && GameServer()->GetGameTime() > m_flTimeKilled + 1.0f)
	{
		GameServer()->Delete(this);

		if (DigitanksGame()->GetTerrain()->IsPointOverHole(GetOrigin()))
		{
			CWreckage* pWreckage = CreateWreckage();

			if (pWreckage)
			{
				pWreckage->FellIntoHole();
				if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY)
					pWreckage->SetScale(2);
			}
		}
		else if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY)
		{
			switch (RandomInt(0, 8))
			{
			case 0:
			case 6:
			case 7:
			case 8:
			default:
			{
				CWreckage* pWreckage = CreateWreckage();
				pWreckage->SetScale(2);
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
		else
		{
			// Strategy mode
			CreateWreckage();
		}
	}
}

CWreckage* CDigitanksEntity::CreateWreckage()
{
	// Figure out what to do about structures later.
	if (dynamic_cast<CDigitank*>(this) == NULL)
		return NULL;

	CWreckage* pWreckage = GameServer()->Create<CWreckage>("CWreckage");
	pWreckage->SetOrigin(GetRenderOrigin());
	pWreckage->SetAngles(GetRenderAngles());
	pWreckage->SetModel(GetModel());
	pWreckage->SetGravity(Vector(0, DigitanksGame()->GetGravity(), 0));
	pWreckage->SetOldTeam(GetDigitanksTeam());

	pWreckage->CalculateVisibility();

	CDigitank* pTank = dynamic_cast<CDigitank*>(this);
	if (pTank)
		pWreckage->SetTurretModel(pTank->GetTurretModel());

	bool bColorSwap = GetTeam() && (dynamic_cast<CDigitank*>(this));
	if (bColorSwap)
		pWreckage->SetColorSwap(GetTeam()->GetColor());

	return pWreckage;
}

void CDigitanksEntity::StartTurn()
{
	// Recache it and make sure it's not dirty.
	DirtyVisibility();
	GetVisibility();

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
	return static_cast<CDigitanksTeam*>(BaseClass::GetTeam());
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

void CDigitanksEntity::OnSetOrigin(const Vector& vecOrigin)
{
	BaseClass::OnSetOrigin(vecOrigin);

	DirtyVisibility();

	// Don't do this constantly.
	if (GameServer()->GetGameTime() > m_flNextDirtyArea)
		m_flNextDirtyArea = GameServer()->GetGameTime() + 1.0f;
}

float CDigitanksEntity::GetVisibility(CDigitanksTeam* pTeam) const
{
	CDigitanksGame* pGame = DigitanksGame();

	CTerrain* pTerrain = pGame->GetTerrain();

	float flConceal = 0.0f;
	if (GetsConcealmentBonus() && pTerrain)
	{
		if (pTerrain->GetBit(CTerrain::WorldToArraySpace(m_vecOrigin.Get().x), CTerrain::WorldToArraySpace(m_vecOrigin.Get().z), TB_TREE))
			flConceal = 0.7f;
	}

	float flCloak = GetCloakConcealment();
	if (flCloak > flConceal)
		flConceal = flCloak;

	if (HasLostConcealment())
		flConceal = 0;

	if (!pGame->ShouldRenderFogOfWar())
		return 1 - flConceal;

	if (!pTeam)
		return 0;

	if (pTeam == GetDigitanksTeam())
		return 1 - flConceal/2;

	float flVisibility = pTeam->GetEntityVisibility(GetHandle()) - flConceal;

	if (flVisibility < 0)
		return 0;

	return flVisibility;
}

void CDigitanksEntity::CalculateVisibility()
{
	for (size_t i = 0; i < DigitanksGame()->GetNumTeams(); i++)
		DigitanksGame()->GetDigitanksTeam(i)->CalculateEntityVisibility(this);

	m_flVisibility = GetVisibility(DigitanksGame()->GetCurrentLocalDigitanksTeam());
	m_bVisibilityDirty = false;
}

float CDigitanksEntity::GetVisibility() const
{
	CDigitanksGame* pGame = DigitanksGame();
	if (!pGame)
		return 0;

	if (!m_bVisibilityDirty)
		return m_flVisibility;

	return GetVisibility(pGame->GetCurrentLocalDigitanksTeam());
}

float CDigitanksEntity::GetVisibility()
{
	CDigitanksGame* pGame = DigitanksGame();
	if (!pGame)
		return 0;

	if (!m_bVisibilityDirty)
		return m_flVisibility;

	CDigitanksTeam* pLocalTeam = pGame->GetCurrentLocalDigitanksTeam();
	m_flVisibility = GetVisibility(pLocalTeam);

	// Find the nearest entity in the local team. If he's close enough, reduce this unit's concealment.
	CDigitanksEntity* pOther = this;
	while (true)
	{
		pOther = CBaseEntity::FindClosest<CDigitanksEntity>(GetOrigin(), pOther);

		if (!pOther)
			break;

		if (pOther == this)
			continue;

		if (pOther->GetDigitanksTeam() != pLocalTeam)
			continue;

		if (pOther->Distance(GetOrigin()) > 15)
			break;

		m_flVisibility = 1 - ((1-m_flVisibility)/2);
		break;
	}

	m_bVisibilityDirty = false;
	return m_flVisibility;
}

float CDigitanksEntity::VisibleRange() const
{
	// Don't use GetOrigin because CDigitank::GetOrigin() can be expensive and we really don't want what it does.
	if (TreesReduceVisibility() && DigitanksGame()->GetTerrain()->IsPointInTrees(m_vecOrigin))
		return BaseVisibleRange()/2;

	return BaseVisibleRange();
}

void CDigitanksEntity::RenderAvailableArea(int iArea)
{
	float flAvailableArea = AvailableArea(iArea);

	if (flAvailableArea == 0)
		return;

	float flScoutScale = 1.0f;

	// Scouts have very tall ones so we can see them underneath on the ground.
	if (GetUnitType() == UNIT_SCOUT)
		flScoutScale = 10;

	if (dynamic_cast<CStructure*>(this) && DigitanksGame()->GetControlMode() == MODE_AIM)
		flScoutScale = 10;

	CRenderingContext c(GameServer()->GetRenderer());
	c.Translate(GetOrigin());
	c.Scale(flAvailableArea, flAvailableArea*flScoutScale, flAvailableArea);
	c.RenderSphere();
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
