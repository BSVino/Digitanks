#include "digitanksentity.h"

#include <maths.h>
#include <mtrand.h>
#include <geometry.h>

#include <models/models.h>
#include <renderer/shaders.h>
#include <renderer/game_renderer.h>
#include <renderer/game_renderingcontext.h>
#include <game/gameserver.h>

#include "dissolver.h"
#include <ui/digitankswindow.h>
#include <ui/hud.h>
#include "digitanksgame.h"
#include "wreckage.h"
#include "dt_camera.h"

REGISTER_ENTITY(CDigitanksEntity);

NETVAR_TABLE_BEGIN(CDigitanksEntity);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CDigitanksEntity);
	SAVEDATA_DEFINE_OUTPUT(OnBecomeVisible);
	SAVEDATA_DEFINE_OUTPUT(OnBecomeFullyVisible);
	SAVEDATA_DEFINE_OUTPUT(OnRescue);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYVECTOR, CEntityHandle<class CSupplyLine>, m_ahSupplyLinesIntercepted);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bVisibilityDirty);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flVisibility);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flNextDirtyArea);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flNextDirtyOrigin);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bImprisoned);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bObjective);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, size_t, m_iCageModel);	// Set in Spawn()
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CParticleSystemInstanceHandle, m_hCageParticles);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CDigitanksEntity);
	INPUT_DEFINE(MakeObjective);
	INPUT_DEFINE(ClearObjective);
INPUTS_TABLE_END();

void CDigitanksEntity::Precache()
{
	PrecacheModel("models/cage.toy");
	PrecacheParticleSystem("cage-aura");
}

void CDigitanksEntity::Spawn()
{
	BaseClass::Spawn();

	m_bTakeDamage = true;
	m_flTotalHealth = TotalHealth();
	m_flHealth = m_flTotalHealth;

	m_flVisibility = 0;
	m_bVisibilityDirty = true;
	m_flNextDirtyArea = 0;
	m_flNextDirtyOrigin = 0;

	m_bImprisoned = false;
	m_bObjective = false;

	m_iCageModel = CModelLibrary::Get()->FindModel("models/cage.toy");
	m_hCageParticles.SetSystem("cage-aura", GetGlobalOrigin());
	m_hCageParticles.FollowEntity(this);
}

void CDigitanksEntity::ClientSpawn()
{
	CalculateVisibility();
}

void CDigitanksEntity::Think()
{
	BaseClass::Think();

	if (m_flNextDirtyOrigin > 0 && GameServer()->GetGameTime() > m_flNextDirtyOrigin)
	{
		DirtyVisibility();

		m_flNextDirtyOrigin = 0;
	}

	if (m_flNextDirtyArea > 0 && GameServer()->GetGameTime() > m_flNextDirtyArea)
	{
		CDigitanksEntity* pOther = this;

		while (true)
		{
			pOther = CBaseEntity::FindClosest<CDigitanksEntity>(GetGlobalOrigin(), pOther);

			if (!pOther)
				break;

			if (pOther == this)
				continue;

			if (pOther->Distance(GetGlobalOrigin()) > VisibleRange() + DigitanksGame()->FogPenetrationDistance())
				break;

			pOther->DirtyVisibility();
		}

		m_flNextDirtyArea = 0;
	}

	if (GameNetwork()->IsHost() && !IsAlive() && GameServer()->GetGameTime() > m_flTimeKilled + 1.0f)
	{
		GameServer()->Delete(this);

		if (DigitanksGame()->GetTerrain()->IsPointOverHole(GetGlobalOrigin()))
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
				for (size_t i = 0; i < 8; i++)
				{
					CDebris* pDebris = GameServer()->Create<CDebris>("CDebris");
					pDebris->SetGlobalOrigin(GetGlobalOrigin());
				}

				CWreckage* pWreckage = CreateWreckage();
				pWreckage->SetScale(2);

				DigitanksGame()->GetOverheadCamera()->Shake(GetGlobalOrigin(), 3);
				break;
			}

			case 1:
			{
				CProjectile* pProjectile = GameServer()->Create<CLargeShell>("CLargeShell");
				pProjectile->SetOwner(NULL);
				pProjectile->SetGlobalOrigin(GetGlobalOrigin());
				pProjectile->Explode();
				break;
			}

			case 2:
			{
				CProjectile* pProjectile = GameServer()->Create<CAOEShell>("CAOEShell");
				pProjectile->SetOwner(NULL);
				pProjectile->SetGlobalOrigin(GetGlobalOrigin());
				pProjectile->Explode();
				break;
			}

			case 3:
			{
				CProjectile* pProjectile = GameServer()->Create<CClusterBomb>("CClusterBomb");
				pProjectile->SetOwner(NULL);
				pProjectile->SetGlobalOrigin(GetGlobalOrigin());
				pProjectile->Explode();
				break;
			}

			case 4:
			{
				CProjectile* pProjectile = GameServer()->Create<CEarthshaker>("CEarthshaker");
				pProjectile->SetOwner(NULL);
				pProjectile->SetGlobalOrigin(GetGlobalOrigin());
				pProjectile->Explode();
				break;
			}

			case 5:
			{
				CProjectile* pProjectile = GameServer()->Create<CTractorBomb>("CTractorBomb");
				pProjectile->SetOwner(NULL);
				pProjectile->SetGlobalOrigin(GetGlobalOrigin());
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

	m_hCageParticles.SetActive(IsImprisoned() && GetVisibility() > 0.1f);
}

CWreckage* CDigitanksEntity::CreateWreckage()
{
	// Figure out what to do about structures later.
	if (dynamic_cast<CDigitank*>(this) == NULL)
		return NULL;

	CWreckage* pWreckage = GameServer()->Create<CWreckage>("CWreckage");
	pWreckage->SetGlobalOrigin(GetRenderOrigin());
	pWreckage->SetGlobalAngles(GetRenderAngles());
	pWreckage->SetModel(GetModelID());
	pWreckage->SetGlobalGravity(Vector(0, 0, DigitanksGame()->GetGravity()));
	pWreckage->SetOldPlayer(GetDigitanksPlayer());

	pWreckage->CalculateVisibility();

	CDigitank* pTank = dynamic_cast<CDigitank*>(this);
	if (pTank)
		pWreckage->SetTurretModel(pTank->GetTurretModel());

	bool bColorSwap = GetPlayerOwner() && (dynamic_cast<CDigitank*>(this));
	if (bColorSwap)
		pWreckage->SetColorSwap(GetPlayerOwner()->GetColor());

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

	if (!GetPlayerOwner())
		return;

	for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
		if (!pEntity)
			continue;

		CSupplyLine* pSupplyLine = dynamic_cast<CSupplyLine*>(pEntity);
		if (!pSupplyLine)
			continue;

		if (pSupplyLine->GetPlayerOwner() == GetPlayerOwner())
			continue;

		if (!pSupplyLine->GetPlayerOwner())
			continue;

		if (!pSupplyLine->GetSupplier() || !pSupplyLine->GetEntity())
			continue;

		Vector vecEntity = GetGlobalOrigin();
		vecEntity.z = 0;

		Vector vecSupplier = pSupplyLine->GetSupplier()->GetGlobalOrigin();
		vecSupplier.z = 0;

		Vector vecUnit = pSupplyLine->GetEntity()->GetGlobalOrigin();
		vecUnit.z = 0;

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

CDigitanksPlayer* CDigitanksEntity::GetDigitanksPlayer() const
{
	CBaseEntity* pOwner = GetOwner();
	if (!pOwner)
		return nullptr;

	TAssert(dynamic_cast<CDigitanksPlayer*>(pOwner));
	return static_cast<CDigitanksPlayer*>(pOwner);
}

bool CDigitanksEntity::ShouldRender() const
{
	return GetVisibility(DigitanksGame()->GetCurrentLocalDigitanksPlayer()) > 0;
}

void CDigitanksEntity::RenderVisibleArea()
{
	if (VisibleRange() == 0)
		return;

	CGameRenderingContext c(GameServer()->GetRenderer(), true);
	c.Translate(GetGlobalOrigin());
	c.Scale(VisibleRange(), VisibleRange(), VisibleRange());
	c.RenderSphere();
}

void CDigitanksEntity::OnSetLocalTransform(TMatrix& m)
{
	BaseClass::OnSetLocalTransform(m);

	// Don't do this constantly.
	if (GameServer()->GetGameTime() > m_flNextDirtyOrigin)
	{
		DirtyVisibility();
		m_flNextDirtyOrigin = GameServer()->GetGameTime() + RandomFloat(0.5, 0.8f);
	}

	// Don't do this constantly.
	if (GameServer()->GetGameTime() > m_flNextDirtyArea)
		m_flNextDirtyArea = GameServer()->GetGameTime() + 1.0f;
}

float CDigitanksEntity::GetVisibility(CDigitanksPlayer* pPlayer) const
{
	CDigitanksGame* pGame = DigitanksGame();

	CTerrain* pTerrain = pGame->GetTerrain();

	float flConceal = 0.0f;
	if (GetsConcealmentBonus() && pTerrain)
	{
		if (pTerrain->GetBit(CTerrain::WorldToArraySpace(GetGlobalOrigin().x), CTerrain::WorldToArraySpace(GetGlobalOrigin().y), TB_TREE))
			flConceal = 0.7f;
	}

	float flCloak = GetCloakConcealment();
	if (flCloak > flConceal)
		flConceal = flCloak;

	if (HasLostConcealment())
		flConceal = 0;

	if (pPlayer && pPlayer == GetDigitanksPlayer())
		return 1 - flConceal/2;

	if (!pGame->ShouldRenderFogOfWar())
	{
		if (pPlayer && pPlayer->IsHumanControlled())
			return 1 - flConceal;
	}

	if (!pPlayer)
		return 0;

	float flVisibility = pPlayer->GetEntityVisibility(GetHandle()) - flConceal;

	if (flVisibility < 0)
		return 0;

	return flVisibility;
}

void CDigitanksEntity::CalculateVisibility()
{
	if (!DigitanksGame())
		return;

	for (size_t i = 0; i < DigitanksGame()->GetNumPlayers(); i++)
	{
		if (!DigitanksGame()->GetDigitanksPlayer(i))
			continue;

		DigitanksGame()->GetDigitanksPlayer(i)->CalculateEntityVisibility(this);
	}

	m_flVisibility = GetVisibility(DigitanksGame()->GetCurrentLocalDigitanksPlayer());
	m_bVisibilityDirty = false;
}

float CDigitanksEntity::GetVisibility() const
{
	CDigitanksGame* pGame = DigitanksGame();
	if (!pGame)
		return 0;

	if (!m_bVisibilityDirty)
		return m_flVisibility;

	return GetVisibility(pGame->GetCurrentLocalDigitanksPlayer());
}

float CDigitanksEntity::GetVisibility()
{
	CDigitanksGame* pGame = DigitanksGame();
	if (!pGame)
		return 0;

	if (!m_bVisibilityDirty)
		return m_flVisibility;

	float flOldVisibility = m_flVisibility;

	CDigitanksPlayer* pLocalPlayer = pGame->GetCurrentLocalDigitanksPlayer();
	m_flVisibility = GetVisibility(pLocalPlayer);

	Vector vecOrigin = GetGlobalOrigin();

	// Find the nearest entity in the local team. If he's close enough, reduce this unit's concealment.
	CDigitanksEntity* pOther = this;
	while (true)
	{
		pOther = CBaseEntity::FindClosest<CDigitanksEntity>(vecOrigin, pOther);

		if (!pOther)
			break;

		if (pOther->Distance(vecOrigin) > 20)
			break;

		if (pOther == this)
			continue;

		if (pOther->GetDigitanksPlayer() != pLocalPlayer)
			continue;

		m_flVisibility = 1 - ((1-m_flVisibility)/2);
		break;
	}

	bool bBecameVisible = (flOldVisibility < 0.25f && m_flVisibility >= 0.25f);
	bool bBecameFullyVisible = (flOldVisibility < 1.0f && m_flVisibility >= 1.0f);

	if (bBecameFullyVisible)
		CallOutput("OnBecomeFullyVisible");
	if (bBecameVisible)
		CallOutput("OnBecomeVisible");

	m_bVisibilityDirty = false;
	return m_flVisibility;
}

float CDigitanksEntity::VisibleRange() const
{
	// Don't use GetGlobalOrigin because CDigitank::GetGlobalOrigin() can be expensive and we really don't want what it does.
	if (TreesReduceVisibility() && DigitanksGame()->GetTerrain()->IsPointInTrees(GetGlobalOrigin()))
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

	CGameRenderingContext c(GameServer()->GetRenderer(), true);
	c.Translate(GetGlobalOrigin());
	c.Scale(flAvailableArea, flAvailableArea*flScoutScale, flAvailableArea);
	c.RenderSphere();
}

bool CDigitanksEntity::IsTouching(CBaseEntity* pOther, Vector& vecPoint) const
{
	if (IsImprisoned())
	{
		CDigitank* pOtherTank = dynamic_cast<CDigitank*>(pOther);

		if (!pOtherTank)
			return false;

		if (!pOtherTank->GetPlayerOwner())
			return false;

		if (Distance(pOtherTank->GetRealOrigin()) > 20)
			return false;

		return true;
	}

	TUnimplemented();

	return false;
}

void CDigitanksEntity::Imprison()
{
	m_bImprisoned = true;
	m_bTakeDamage = false;

	m_hCageParticles.SetActive(true);
}

void CDigitanksEntity::Rescue(CDigitanksEntity* pOther)
{
	if (!m_bImprisoned)
		return;

	if (!pOther)
		return;

	if (!pOther->GetPlayerOwner())
		return;

	if (!pOther->GetPlayerOwner()->IsHumanControlled())
		return;

	// Only suppliers can free structures other than CPUs. Otherwise they'd just go bunk again immediately.
	if (GetUnitType() != STRUCTURE_CPU && dynamic_cast<CStructure*>(this) && !dynamic_cast<CSupplier*>(pOther))
		return;

	if (GetUnitType() == STRUCTURE_CPU)
		pOther->GetDigitanksPlayer()->AddPower(5);

	pOther->GetDigitanksPlayer()->AddUnit(this);

	DigitanksWindow()->GetHUD()->AddPowerupNotification(this, POWERUP_TANK);

	m_bImprisoned = false;
	m_bTakeDamage = true;

	StartTurn();

	CallOutput("OnRescue");

	m_hCageParticles.SetActive(false);

	m_flNextDirtyArea = GameServer()->GetGameTime();
}

bool CDigitanksEntity::ShouldRenderTransparent() const
{
	float flVisibility = GetVisibility();
	if (flVisibility < 1)
		return true;

	return BaseClass::ShouldRenderTransparent();
}

void CDigitanksEntity::ModifyContext(CRenderingContext* pContext) const
{
	BaseClass::ModifyContext(pContext);

	CDigitanksPlayer* pTeam = DigitanksGame()->GetCurrentPlayer();

	float flVisibility = GetVisibility();
	if (flVisibility < 1)
	{
		pContext->SetAlpha(flVisibility);
		pContext->SetBlend(BLEND_ALPHA);
	}
}

void CDigitanksEntity::OnRender(CGameRenderingContext* pContext) const
{
	BaseClass::OnRender(pContext);

	if (GameServer()->GetRenderer()->IsRenderingTransparent() && IsImprisoned())
	{
		CGameRenderingContext c(GameServer()->GetRenderer(), true);
		c.SetBackCulling(false);
		c.SetBlend(BLEND_ADDITIVE);
		c.Scale(GetBoundingRadius(), GetBoundingRadius(), GetBoundingRadius());
		c.Rotate((float)GameServer()->GetGameTime() * 90, Vector(0, 0, 1));

		float flVisibility = GetVisibility() * 0.6f;
		c.UseProgram("scroll");
		c.SetUniform("iTexture", 0);
		c.SetUniform("flAlpha", flVisibility);
		c.SetUniform("flTime", (float)-GameServer()->GetGameTime());
		c.SetUniform("flSpeed", 1.0f);

		c.RenderModel(m_iCageModel, this);
	}

	if (GameServer()->GetRenderer()->IsRenderingTransparent() && IsImprisoned())
	{
		CGameRenderingContext c(GameServer()->GetRenderer(), true);
		c.SetBackCulling(false);
		c.SetBlend(BLEND_ADDITIVE);
		c.Scale(GetBoundingRadius()+1, GetBoundingRadius()+1, GetBoundingRadius()+1);
		c.Rotate(-(float)GameServer()->GetGameTime() * 90, Vector(0, 0, 1));

		float flVisibility = GetVisibility() * 0.6f;
		c.UseProgram("scroll");
		c.SetUniform("iTexture", 0);
		c.SetUniform("flAlpha", flVisibility);
		c.SetUniform("flTime", (float)-GameServer()->GetGameTime());
		c.SetUniform("flSpeed", 1.0f);

		c.RenderModel(m_iCageModel);
	}

	if (!GameServer()->GetRenderer()->IsRenderingTransparent())
		return;

	if (!DigitanksGame()->GetTerrain()->GetBit(CTerrain::WorldToArraySpace(GetGlobalOrigin().x), CTerrain::WorldToArraySpace(GetGlobalOrigin().y), TB_TREE))
		return;

	if (GetVisibility() < 0.6f)
		return;

	CGameRenderingContext c(GameServer()->GetRenderer(), true);
	c.SetBlend(BLEND_NONE);
	c.SetAlpha(1);
	c.BindTexture(0);

	TUnimplemented();

	/*
	// Draw outlines of objects in trees.
	glPushAttrib( GL_ALL_ATTRIB_BITS );

	glDisable( GL_TEXTURE_2D );
	glEnable( GL_POLYGON_OFFSET_FILL );
	glPolygonOffset( -3.5f, -3.5f );

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(true);
	glEnable(GL_DEPTH_TEST);
	pContext->RenderModel(GetModel());

	glDisable( GL_POLYGON_OFFSET_FILL );
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(false);
	glEnable(GL_DEPTH_TEST);
	glLineWidth( 2.0f );
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glColor3ubv( Color(0, 0, 0) );
	pContext->RenderModel(GetModel());

	glPopAttrib();
	*/
}

void CDigitanksEntity::MakeObjective(const tvector<tstring>& sArgs)
{
	m_bObjective = true;
}

void CDigitanksEntity::ClearObjective(const tvector<tstring>& sArgs)
{
	m_bObjective = false;
}

void CDigitanksEntity::SetOwner(CDigitanksPlayer* pOwner)
{
	m_hOwner = pOwner;
}

CDigitanksPlayer* CDigitanksEntity::GetPlayerOwner() const
{
	CBaseEntity* pOwner = GetOwner();
	if (!pOwner)
		return nullptr;

	TAssert(dynamic_cast<CDigitanksPlayer*>(pOwner));
	return static_cast<CDigitanksPlayer*>(pOwner);
}
