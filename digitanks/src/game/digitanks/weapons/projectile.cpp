#include "projectile.h"

#include <GL/glew.h>
#include <maths.h>
#include <mtrand.h>

#include <renderer/particles.h>

#include <digitanks/digitanksgame.h>
#include <digitanks/dt_renderer.h>
#include "ui/instructor.h"
#include "ui/digitankswindow.h"

REGISTER_ENTITY(CProjectile);

NETVAR_TABLE_BEGIN(CProjectile);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CProjectile);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bFallSoundPlayed);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecLandingSpot);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bMissileDefensesNotified);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bFragmented);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iBounces);
//	size_t						m_iParticleSystem;	// Generated on load
SAVEDATA_TABLE_END();

CProjectile::CProjectile()
{
	m_bFallSoundPlayed = false;

	m_bFragmented = false;
	m_iBounces = 0;

	m_bMissileDefensesNotified = false;
}

void CProjectile::Precache()
{
	PrecacheParticleSystem(L"shell-trail");
	PrecacheSound(L"sound/bomb-drop.wav");
}

void CProjectile::Think()
{
	BaseClass::Think();

	if (MakesSounds() && BombDropNoise() && GetVelocity().y < 10.0f && !m_bFallSoundPlayed && m_flTimeExploded == 0.0f)
	{
		bool bCanSeeOwner;
		if (m_hOwner != NULL && DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), m_hOwner->GetOrigin()) > 0)
			bCanSeeOwner = true;
		else
			bCanSeeOwner = false;

		if (DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), m_vecLandingSpot) > 0 || bCanSeeOwner)
		{
			EmitSound(L"sound/bomb-drop.wav");
			SetSoundVolume(L"sound/bomb-drop.wav", 0.5f);
		}

		m_bFallSoundPlayed = true;
	}

	if (Fragments() && !m_bFragmented && GameServer()->GetGameTime() - m_flTimeCreated > 2.0f && m_flTimeExploded == 0.0f)
	{
		Fragment();
	}

	if (!m_bMissileDefensesNotified && !IsDeleted() && GetVelocity().y < 10.0f && m_flTimeExploded == 0.0f)
	{
		// Now that we're falling and we've already fragmented (or maybe we are a fragment)
		// notify any tanks on the ground in our landing spot that we're incoming.
		// Damn don't the real missile defense designers wish that incoming projectiles did this!

		CDigitank* pClosest = NULL;
		while (pClosest = CBaseEntity::FindClosest<CDigitank>(m_vecLandingSpot, pClosest))
		{
			if (pClosest->Distance(m_vecLandingSpot) > 30)
				continue;

			if (pClosest == m_hOwner)
				continue;

			if (m_hOwner != NULL && pClosest->GetTeam() == m_hOwner->GetTeam())
				continue;

			if (pClosest->CanFireMissileDefense())
			{
				pClosest->FireMissileDefense(this);
				break;
			}
		}

		m_bMissileDefensesNotified = true;
	}

	if (GetOrigin().y < DigitanksGame()->GetTerrain()->GetHeight(GetOrigin().x, GetOrigin().z) - 20 || GetOrigin().y < -100)
		Delete();
}

void CProjectile::SpecialCommand()
{
	if (Fragments() && !m_bFragmented)
	{
		Fragment();
		return;
	}

	if (ShouldExplode() && m_flTimeExploded == 0.0f && !m_bFragmented)
		Explode();
}

void CProjectile::Fragment()
{
	for (size_t i = 0; i < Fragments(); i++)
	{
		CProjectile* pProjectile = GameServer()->Create<CProjectile>(GetClassName());
		pProjectile->SetOwner(m_hOwner);
		pProjectile->SetVelocity(GetVelocity() + Vector(RandomFloat(-10, 10), RandomFloat(-10, 10), RandomFloat(-10, 10)));
		pProjectile->SetGravity(GetGravity());
		pProjectile->SetLandingSpot(m_vecLandingSpot);
		pProjectile->SetOrigin(GetOrigin());
		pProjectile->m_bFragmented = true;
		DigitanksGame()->AddProjectileToWaitFor();
	}

	Delete();
}

void CProjectile::ModifyContext(class CRenderingContext* pContext, bool bTransparent)
{
	BaseClass::ModifyContext(pContext, bTransparent);
}

void CProjectile::OnRender(class CRenderingContext* pContext, bool bTransparent)
{
	if (!m_bShouldRender)
		return;

	BaseClass::OnRender(pContext, bTransparent);

	if (m_flTimeExploded == 0.0f)
	{
		if (!bTransparent)
		{
			CRenderingContext c(GameServer()->GetRenderer());
			c.Scale(ShellRadius(), ShellRadius(), ShellRadius());
			c.SetColor(Color(255, 255, 255));
			c.RenderSphere();
		}
	}
	else if (UsesStandardExplosion())
	{
		float flAlpha = RemapValClamped(GameServer()->GetGameTime()-m_flTimeExploded, 0.2f, 1.2f, 1, 0);
		if (flAlpha > 0 && bTransparent)
		{
			CRenderingContext c(GameServer()->GetRenderer());
			if (DigitanksGame()->GetDigitanksRenderer()->ShouldUseFramebuffers())
				c.UseFrameBuffer(DigitanksGame()->GetDigitanksRenderer()->GetExplosionBuffer());
			else
				c.SetBlend(BLEND_ADDITIVE);
			c.Scale(ExplosionRadius(), ExplosionRadius(), ExplosionRadius());
			c.SetColor(Color(255, 255, 255, (int)(flAlpha*255)));
			c.RenderSphere();
		}
	}
}

void CProjectile::OnDeleted()
{
	if (m_iParticleSystem != ~0)
		CParticleSystemLibrary::StopInstance(m_iParticleSystem);
}

bool CProjectile::ShouldTouch(CBaseEntity* pOther) const
{
	if (m_flTimeExploded != 0)
		return false;

	if (!pOther)
		return false;

	if (pOther == m_hOwner)
		return false;

	if (pOther->GetCollisionGroup() == CG_PROP)
		return true;

	if (pOther->GetCollisionGroup() == CG_ENTITY)
	{
		if (m_hOwner != NULL && pOther->GetTeam() == m_hOwner->GetTeam())
			return false;

		return true;
	}

	if (pOther->GetCollisionGroup() == CG_TERRAIN)
		return true;

	return false;
}

bool CProjectile::IsTouching(CBaseEntity* pOther, Vector& vecPoint) const
{
	switch (pOther->GetCollisionGroup())
	{
	case CG_ENTITY:
		vecPoint = GetOrigin();
		if ((pOther->GetOrigin() - GetOrigin()).LengthSqr() < pOther->GetBoundingRadius()*pOther->GetBoundingRadius())
			return true;
		break;

	case CG_TERRAIN:
		return DigitanksGame()->GetTerrain()->Collide(GetLastOrigin(), GetOrigin(), vecPoint);

	case CG_PROP:
		return pOther->Collide(GetLastOrigin(), GetOrigin(), vecPoint);
	}

	return false;
};

void CProjectile::Touching(CBaseEntity* pOther)
{
	if (m_iBounces < Bounces())
	{
		// If we hit the terrain then bounce otherwise blow up.
		if (dynamic_cast<CTerrain*>(pOther))
		{
			Matrix4x4 mReflect;

			Vector vecProjectileOrigin = GetOrigin();
			DigitanksGame()->GetTerrain()->SetPointHeight(vecProjectileOrigin);
			vecProjectileOrigin.y += 0.5f;

			mReflect.SetReflection(dynamic_cast<CTerrain*>(pOther)->GetNormalAtPoint(vecProjectileOrigin));

			SetVelocity((mReflect*GetVelocity())*0.6f);

			SetOrigin(vecProjectileOrigin);

			m_iBounces++;
			return;
		}
	}

	if (dynamic_cast<CTerrain*>(pOther))
	{
		if (ShouldExplode())
			pOther->TakeDamage(m_hOwner, this, DAMAGE_EXPLOSION, m_flDamage);
	}
	else
		pOther->TakeDamage(m_hOwner, this, DAMAGE_EXPLOSION, m_flDamage);

	if (ShouldExplode())
		Explode(pOther);
	else
		m_flTimeExploded = 1;	// Remove immediately.

	if (MakesSounds())
	{
		StopSound(L"sound/bomb-drop.wav");

		bool bCanSeeOwner;
		if (m_hOwner != NULL && DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), m_hOwner->GetOrigin()) > 0)
			bCanSeeOwner = true;
		else
			bCanSeeOwner = false;

		if (DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), m_vecLandingSpot) > 0 || bCanSeeOwner)
			EmitSound(L"sound/explosion.wav");
	}
}

void CProjectile::OnExplode(CBaseEntity* pInstigator)
{
	BaseClass::OnExplode(pInstigator);

	if (MakesSounds())
		StopSound(L"sound/bomb-drop.wav");

	if (m_iParticleSystem != ~0)
	{
		CParticleSystemLibrary::StopInstance(m_iParticleSystem);
		m_iParticleSystem = 0;
	}

	if (!DigitanksGame()->GetCurrentLocalDigitanksTeam() || DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetVisibilityAtPoint(GetOrigin()) > 0.1f)
		CreateExplosionSystem();
}

bool CProjectile::ShouldPlayExplosionSound()
{
	return DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), m_vecLandingSpot) > 0;
}

void CProjectile::OnSetOwner(CDigitank* pOwner)
{
	BaseClass::OnSetOwner(pOwner);

	SetSimulated(true);
	SetCollisionGroup(CG_PROJECTILE);

	if (m_bShouldRender)
	{
		m_iParticleSystem = CreateTrailSystem();
		if (m_iParticleSystem != ~0)
			CParticleSystemLibrary::GetInstance(m_iParticleSystem)->FollowEntity(this);
	}
}

bool CProjectile::ShouldBeVisible()
{
	return DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), m_vecLandingSpot) > 0;
}

size_t CProjectile::CreateTrailSystem()
{
	return CParticleSystemLibrary::AddInstance(L"shell-trail", GetOrigin());
}

void CProjectile::CreateExplosionSystem()
{
}

void CProjectile::ClientEnterGame()
{
	BaseClass::ClientEnterGame();

	if (m_bShouldRender)
	{
		m_iParticleSystem = CreateTrailSystem();
		if (m_iParticleSystem != ~0)
			CParticleSystemLibrary::GetInstance(m_iParticleSystem)->FollowEntity(this);
	}
}

REGISTER_ENTITY(CSmallShell);

NETVAR_TABLE_BEGIN(CSmallShell);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CSmallShell);
SAVEDATA_TABLE_END();

float CSmallShell::ExplosionRadius()
{
	if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
		return 4.0f;
	else
		return 6.0f;
}

REGISTER_ENTITY(CMediumShell);

NETVAR_TABLE_BEGIN(CMediumShell);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CMediumShell);
SAVEDATA_TABLE_END();

float CMediumShell::ExplosionRadius()
{
	if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
		return 6.0f;
	else
		return 12.0f;
}

REGISTER_ENTITY(CLargeShell);

NETVAR_TABLE_BEGIN(CLargeShell);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CLargeShell);
SAVEDATA_TABLE_END();

float CLargeShell::ExplosionRadius()
{
	if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
		return 8.0f;
	else
		return 18.0f;
}

REGISTER_ENTITY(CAOEShell);

NETVAR_TABLE_BEGIN(CAOEShell);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CAOEShell);
SAVEDATA_TABLE_END();

void CAOEShell::Precache()
{
	PrecacheParticleSystem(L"aoe-explosion-strategy");
	PrecacheParticleSystem(L"aoe-explosion-artillery");
	PrecacheParticleSystem(L"aoe-trail");
}

void CAOEShell::CreateExplosionSystem()
{
	if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
		CParticleSystemLibrary::AddInstance(L"aoe-explosion-strategy", GetOrigin());
	else
		CParticleSystemLibrary::AddInstance(L"aoe-explosion-artillery", GetOrigin());
}

size_t CAOEShell::CreateTrailSystem()
{
	return CParticleSystemLibrary::AddInstance(L"aoe-trail", GetOrigin());
}

float CAOEShell::ExplosionRadius()
{
	if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
		return 12.0f;
	else
		return 30.0f;
}

REGISTER_ENTITY(CEMP);

NETVAR_TABLE_BEGIN(CEMP);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CEMP);
SAVEDATA_TABLE_END();

void CEMP::Precache()
{
	PrecacheParticleSystem(L"emp-explosion");
	PrecacheParticleSystem(L"emp-trail");
}

void CEMP::CreateExplosionSystem()
{
	CParticleSystemLibrary::AddInstance(L"emp-explosion", GetOrigin());
}

size_t CEMP::CreateTrailSystem()
{
	return CParticleSystemLibrary::AddInstance(L"emp-trail", GetOrigin());
}

REGISTER_ENTITY(CICBM);

NETVAR_TABLE_BEGIN(CICBM);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CICBM);
SAVEDATA_TABLE_END();

REGISTER_ENTITY(CGrenade);

NETVAR_TABLE_BEGIN(CGrenade);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CGrenade);
SAVEDATA_TABLE_END();

REGISTER_ENTITY(CDaisyChain);

NETVAR_TABLE_BEGIN(CDaisyChain);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CDaisyChain);
SAVEDATA_TABLE_END();

void CDaisyChain::Spawn()
{
	BaseClass::Spawn();

	m_flExplosionRadius = 20;
}

void CDaisyChain::OnExplode(CBaseEntity* pInstigator)
{
	BaseClass::OnExplode(pInstigator);

	// 20, 16, 12
	// The third will be the last.
	if (m_flExplosionRadius < 13)
		return;

	CDaisyChain* pProjectile = GameServer()->Create<CDaisyChain>(GetClassName());
	pProjectile->SetOwner(m_hOwner);
	pProjectile->SetVelocity(GetVelocity());
	pProjectile->SetGravity(GetGravity());
	pProjectile->SetLandingSpot(m_vecLandingSpot);
	pProjectile->SetOrigin(GetOrigin());
	pProjectile->m_flExplosionRadius = m_flExplosionRadius - 4;
	pProjectile->m_flDamage = m_flDamage - 1;
	DigitanksGame()->AddProjectileToWaitFor();
}

REGISTER_ENTITY(CClusterBomb);

NETVAR_TABLE_BEGIN(CClusterBomb);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CClusterBomb);
SAVEDATA_TABLE_END();

void CClusterBomb::Spawn()
{
	BaseClass::Spawn();

	m_flExplosionRadius = 20;
}

void CClusterBomb::SpecialCommand()
{
	if (m_flExplosionRadius < 15)
		return;

	if (m_flTimeExploded == 0.0f)
		Explode();
}

void CClusterBomb::OnExplode(CBaseEntity* pInstigator)
{
	BaseClass::OnExplode(pInstigator);

	if (m_flExplosionRadius < 15)
		return;

	for (size_t i = 0; i < 6; i++)
	{
		CClusterBomb* pProjectile = GameServer()->Create<CClusterBomb>(GetClassName());
		pProjectile->SetOwner(m_hOwner);
		pProjectile->SetVelocity(Vector(RandomFloat(-10, 10), RandomFloat(10, 30), RandomFloat(-10, 10)));
		pProjectile->SetGravity(Vector(0, DigitanksGame()->GetGravity(), 0));
		pProjectile->SetLandingSpot(m_vecLandingSpot);
		pProjectile->SetOrigin(GetOrigin());
		pProjectile->m_flExplosionRadius = 10;
		pProjectile->m_flDamage = m_flDamage/2;
		DigitanksGame()->AddProjectileToWaitFor();
	}
}

REGISTER_ENTITY(CEarthshaker);

NETVAR_TABLE_BEGIN(CEarthshaker);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CEarthshaker);
SAVEDATA_TABLE_END();

REGISTER_ENTITY(CSploogeShell);

NETVAR_TABLE_BEGIN(CSploogeShell);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CSploogeShell);
SAVEDATA_TABLE_END();

REGISTER_ENTITY(CTractorBomb);

NETVAR_TABLE_BEGIN(CTractorBomb);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTractorBomb);
SAVEDATA_TABLE_END();

void CTractorBomb::Precache()
{
	PrecacheParticleSystem(L"tractor-bomb-explosion");
	PrecacheParticleSystem(L"tractor-bomb-trail");
}

size_t CTractorBomb::CreateTrailSystem()
{
	return CParticleSystemLibrary::AddInstance(L"tractor-bomb-trail", GetOrigin());
}

void CTractorBomb::CreateExplosionSystem()
{
	CParticleSystemLibrary::AddInstance(L"tractor-bomb-explosion", GetOrigin());
}

REGISTER_ENTITY(CArtilleryShell);

NETVAR_TABLE_BEGIN(CArtilleryShell);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CArtilleryShell);
SAVEDATA_TABLE_END();

void CArtilleryShell::Precache()
{
	PrecacheParticleSystem(L"emp-explosion");
	PrecacheParticleSystem(L"emp-trail");
}

void CArtilleryShell::CreateExplosionSystem()
{
	CParticleSystemLibrary::AddInstance(L"emp-explosion", GetOrigin());
}

size_t CArtilleryShell::CreateTrailSystem()
{
	return CParticleSystemLibrary::AddInstance(L"emp-trail", GetOrigin());
}

REGISTER_ENTITY(CArtilleryAoE);

NETVAR_TABLE_BEGIN(CArtilleryAoE);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CArtilleryAoE);
SAVEDATA_TABLE_END();

void CArtilleryAoE::Precache()
{
	PrecacheParticleSystem(L"aoe-explosion-strategy");
	PrecacheParticleSystem(L"aoe-trail");
}

void CArtilleryAoE::CreateExplosionSystem()
{
	CParticleSystemLibrary::AddInstance(L"aoe-explosion-strategy", GetOrigin());
}

size_t CArtilleryAoE::CreateTrailSystem()
{
	return CParticleSystemLibrary::AddInstance(L"aoe-trail", GetOrigin());
}

REGISTER_ENTITY(CArtilleryICBM);

NETVAR_TABLE_BEGIN(CArtilleryICBM);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CArtilleryICBM);
SAVEDATA_TABLE_END();

REGISTER_ENTITY(CDevastator);

NETVAR_TABLE_BEGIN(CDevastator);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CDevastator);
SAVEDATA_TABLE_END();

REGISTER_ENTITY(CInfantryFlak);

NETVAR_TABLE_BEGIN(CInfantryFlak);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CInfantryFlak);
SAVEDATA_TABLE_END();

size_t CInfantryFlak::CreateTrailSystem()
{
	return ~0;
}

REGISTER_ENTITY(CTorpedo);

NETVAR_TABLE_BEGIN(CTorpedo);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTorpedo);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bBurrowing);
SAVEDATA_TABLE_END();

CTorpedo::CTorpedo()
{
	m_flTimeExploded = 0;
	m_bBurrowing = false;
	m_flDamage = 0;
}

void CTorpedo::Precache()
{
	PrecacheParticleSystem(L"torpedo-trail");
	PrecacheParticleSystem(L"torpedo-explosion");
}

void CTorpedo::Spawn()
{
	m_bBurrowing = false;
	m_flDamage = 0;
}

size_t CTorpedo::CreateTrailSystem()
{
	return CParticleSystemLibrary::AddInstance(L"torpedo-trail", GetOrigin());
}

void CTorpedo::Think()
{
	if (m_bBurrowing)
	{
		float flDistance = GameServer()->GetFrameTime() * 10;
		Vector vecDirection = m_vecLandingSpot - GetOrigin();
		vecDirection.y = 0;

		if (vecDirection.LengthSqr() < flDistance*flDistance*2)
		{
			m_bBurrowing = false;
			Explode();
		}

		// Insert Jaws theme here.
		Vector vecPosition = GetOrigin() + vecDirection.Normalized() * flDistance;
		SetOrigin(DigitanksGame()->GetTerrain()->SetPointHeight(vecPosition));
	}
	else
	{
		// Sometimes the collide raytrace fails. Fuck that jazz.
		if (GetOrigin().y < DigitanksGame()->GetTerrain()->GetHeight(GetOrigin().x, GetOrigin().z))
			Touching(DigitanksGame()->GetTerrain());
	}

	if (!m_bBurrowing)
		BaseClass::Think();
}

bool CTorpedo::ShouldTouch(CBaseEntity* pOther) const
{
	if (m_bBurrowing && pOther && pOther->GetCollisionGroup() == CG_TERRAIN)
		return false;

	return BaseClass::ShouldTouch(pOther);
}

void CTorpedo::Touching(CBaseEntity* pOther)
{
	if (pOther->GetCollisionGroup() == CG_TERRAIN)
	{
		m_bBurrowing = true;
		SetVelocity(Vector());
		SetGravity(Vector());
	}

	// Don't call superclass!
}

void CTorpedo::Explode(CBaseEntity* pInstigator)
{
	CSupplyLine* pClosest = NULL;
	while (true)
	{
		pClosest = CBaseEntity::FindClosest<CSupplyLine>(GetOrigin(), pClosest);

		if (!pClosest)
			break;

		if (pClosest->GetTeam() == GetTeam())
			continue;

		if (!pClosest->GetSupplier() || !pClosest->GetEntity())
			continue;

		if (pClosest->Distance(GetOrigin()) > ExplosionRadius())
			break;

		pClosest->Intercept(0.5f);

		DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_TORPEDO);
	}

	CDigitank* pClosestTank = NULL;
	while (true)
	{
		pClosestTank = CBaseEntity::FindClosest<CDigitank>(GetOrigin(), pClosestTank);

		if (!pClosestTank)
			break;

		if (pClosestTank->GetTeam() == GetTeam())
			continue;

		if (pClosestTank->Distance(GetOrigin()) > ExplosionRadius())
			break;

		pClosestTank->Disable(1);
	}

	BaseClass::Explode(pInstigator);

	if (DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), GetOrigin()) > 0.5f)
		DigitanksGame()->GetDigitanksRenderer()->BloomPulse();

	CParticleSystemLibrary::AddInstance(L"torpedo-explosion", GetOrigin());
}

REGISTER_ENTITY(CTreeCutter);

NETVAR_TABLE_BEGIN(CTreeCutter);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTreeCutter);
SAVEDATA_TABLE_END();
