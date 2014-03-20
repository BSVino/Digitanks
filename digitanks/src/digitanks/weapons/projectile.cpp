#include "projectile.h"

#include <maths.h>
#include <mtrand.h>

#include <renderer/particles.h>
#include <renderer/game_renderingcontext.h>

#include <digitanksgame.h>
#include <dt_renderer.h>
#include <dt_camera.h>
#include "ui/instructor.h"
#include "ui/digitankswindow.h"
#include "ui/hud.h"

REGISTER_ENTITY(CProjectile);

NETVAR_TABLE_BEGIN(CProjectile);
	NETVAR_DEFINE(Vector, m_vecLandingSpot);
	NETVAR_DEFINE(bool, m_bFragmented);
	NETVAR_DEFINE(float, m_flDamageBonusTime);
	NETVAR_DEFINE(float, m_flDamageBonusFreeze);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CProjectile);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Vector, m_vecLandingSpot);
	SAVEDATA_DEFINE(CSaveData::DATA_OMIT, CParticleSystemInstanceHandle, m_hTrailParticles);	// Generated on load
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bFragmented);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iBounces);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bMissileDefensesNotified);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bFallSoundPlayed);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flDamageBonusTime);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flDamageBonusFreeze);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CProjectile);
INPUTS_TABLE_END();

#define _T(x) x

CProjectile::CProjectile()
{
	m_bFallSoundPlayed = false;

	m_bFragmented = false;
	m_iBounces = 0;

	m_bMissileDefensesNotified = false;

	m_flDamageBonusTime = 0;
	m_flDamageBonusFreeze = 0;
}

void CProjectile::Precache()
{
	PrecacheParticleSystem(_T("shell-trail"));
	PrecacheSound(_T("sound/bomb-drop.wav"));
	PrecacheParticleSystem(_T("dmg-boost"));
}

void CProjectile::Spawn()
{
	BaseClass::Spawn();

	m_hTrailParticles.SetSystem(CreateTrailSystem(), GetGlobalOrigin());
	m_hTrailParticles.FollowEntity(this);
}

void CProjectile::Think()
{
	BaseClass::Think();

	if (GameServer()->GetGameTime() - m_flDamageBonusTime > 1)
		m_flDamageBonusTime = 0;

	CSystemInstance* pInstance = CParticleSystemLibrary::Get()->GetInstance(m_hTrailParticles.GetInstance());
	if (pInstance)
		pInstance->SetColor(GetBonusDamageColor());

	if (MakesSounds() && BombDropNoise() && GetGlobalVelocity().z < 10.0f && !m_bFallSoundPlayed && m_flTimeExploded == 0.0)
	{
		bool bCanSeeOwner;
		if (m_hOwner != NULL && DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksPlayer(), m_hOwner->GetGlobalOrigin()) > 0)
			bCanSeeOwner = true;
		else
			bCanSeeOwner = false;

		if (DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksPlayer(), m_vecLandingSpot) > 0 || bCanSeeOwner)
		{
			EmitSound(_T("sound/bomb-drop.wav"));
			SetSoundVolume(_T("sound/bomb-drop.wav"), 0.5f);
		}

		m_bFallSoundPlayed = true;
	}

	if (Fragments() && !m_bFragmented && m_flTimeExploded == 0.0)
	{
		if (GetOwner() && GetOwner()->GetDigitanksPlayer() && !GetOwner()->GetDigitanksPlayer()->IsHumanControlled() && GameServer()->GetGameTime() - GetSpawnTime() > 2.0f)
			Fragment();
		else if (DigitanksGame()->GetGameType() != GAMETYPE_ARTILLERY && GameServer()->GetGameTime() - GetSpawnTime() > 2.0f)
			Fragment();
	}

	if (!m_bMissileDefensesNotified && !IsDeleted() && GetGlobalVelocity().z < 10.0f && m_flTimeExploded == 0.0)
	{
		// Now that we're falling and we've already fragmented (or maybe we are a fragment)
		// notify any tanks on the ground in our landing spot that we're incoming.
		// Damn don't the real missile defense designers wish that incoming projectiles did this!

		CDigitank* pClosest = NULL;
		while (pClosest = CBaseEntity::FindClosest<CDigitank>(m_vecLandingSpot, pClosest))
		{
			if (pClosest->Distance(m_vecLandingSpot) > 30)
				break;

			if (pClosest == m_hOwner)
				continue;

			if (m_hOwner != NULL && pClosest->GetPlayerOwner() == m_hOwner->GetOwner())
				continue;

			if (pClosest->CanFireMissileDefense())
			{
				pClosest->FireMissileDefense(this);
				break;
			}
		}

		m_bMissileDefensesNotified = true;
	}

	m_hTrailParticles.SetActive(m_bShouldRender && m_flTimeExploded == 0.0);

	if (!GameNetwork()->IsHost())
		return;

	if (GetGlobalOrigin().z < DigitanksGame()->GetTerrain()->GetHeight(GetGlobalOrigin().x, GetGlobalOrigin().y) - 20 || GetGlobalOrigin().z < -100)
		Delete();

	if (GameServer()->GetGameTime() - GetSpawnTime() > 20.0f)
		Delete();
}

void CProjectile::SpecialCommand()
{
	if (Fragments() && !m_bFragmented)
	{
		Fragment();
		return;
	}

	if (GameServer()->GetGameTime() - m_flDamageBonusTime < DamageBonusTime())
		return;

	if (m_flTimeExploded > 0)
		return;

	m_flDamageBonusTime = GameServer()->GetGameTime();

	size_t iPulse = CParticleSystemLibrary::AddInstance(_T("dmg-boost"), GetGlobalOrigin());
	if (iPulse != ~0)
		CParticleSystemLibrary::GetInstance(iPulse)->FollowEntity(this);
}

Color CProjectile::GetBonusDamageColor() const
{
	if (m_flDamageBonusTime == 0.0f)
		return Color(255, 255, 255);

	float flRamp = RemapValClamped((float)GameServer()->GetGameTime() - m_flDamageBonusTime, 0, DamageBonusTime(), 1, 0);

	if (m_flDamageBonusFreeze > 0)
		flRamp = m_flDamageBonusFreeze;

	return LerpValue<Vector>(Vector(1, 1, 1), Vector(1, 0.2f, 0), flRamp);
}

float CProjectile::GetBonusDamage()
{
	if (m_flDamageBonusTime == 0.0f)
		return 0;

	return RemapVal((float)GameServer()->GetGameTime() - m_flDamageBonusTime, 0, DamageBonusTime(), DamageBonus(), 0);
}

void CProjectile::Fragment()
{
	if (!GameNetwork()->IsHost())
		return;

	for (size_t i = 0; i < Fragments(); i++)
	{
		CProjectile* pProjectile = GameServer()->Create<CProjectile>(GetClassName());
		pProjectile->SetOwner(m_hOwner);
		pProjectile->SetGlobalVelocity(GetGlobalVelocity() + Vector(RandomFloat(-10, 10), RandomFloat(-10, 10), RandomFloat(-10, 10)));
		pProjectile->SetGlobalGravity(Vector(0, 0, DigitanksGame()->GetGravity()));
		pProjectile->SetLandingSpot(m_vecLandingSpot);
		pProjectile->SetGlobalOrigin(GetGlobalOrigin());
		pProjectile->m_bFragmented = true;
		pProjectile->m_flDamage = m_flDamage;
		DigitanksGame()->AddProjectileToWaitFor();

		if (i == 0)
			DigitanksGame()->GetOverheadCamera()->ReplaceProjectileTarget(pProjectile);
	}

	Delete();
}

void CProjectile::OnRender(class CGameRenderingContext* pContext) const
{
	if (!m_bShouldRender)
		return;

	BaseClass::OnRender(pContext);

	if (UsesStandardShell() && m_flTimeExploded == 0.0)
	{
		if (!GameServer()->GetRenderer()->IsRenderingTransparent())
		{
			CRenderingContext c(GameServer()->GetRenderer(), true);
			c.Scale(ShellRadius(), ShellRadius(), ShellRadius());
			c.SetColor(GetBonusDamageColor());
			c.RenderSphere();
		}
	}
	else if (UsesStandardExplosion())
	{
		float flAlpha = RemapValClamped((float)(GameServer()->GetGameTime()-m_flTimeExploded), 0.2f, 1.2f, 1, 0);
		if (flAlpha > 0 && GameServer()->GetRenderer()->IsRenderingTransparent())
		{
			CRenderingContext c(GameServer()->GetRenderer(), true);
			c.UseFrameBuffer(DigitanksGame()->GetDigitanksRenderer()->GetExplosionBuffer());
			c.Scale(ExplosionRadius(), ExplosionRadius(), ExplosionRadius());
			c.SetAlpha(flAlpha);
			c.SetColor(GetBonusDamageColor());
			c.RenderSphere();
		}
	}
}

bool CProjectile::ShouldTouch(CBaseEntity* pOther) const
{
	if (m_flTimeExploded != 0.0)
		return false;

	if (!pOther)
		return false;

	if (pOther == m_hOwner)
		return false;

	if (pOther->GetCollisionGroup() == CG_PROP)
		return true;

	TStubbed("CProjectile::ShouldTouch");
#if 0
	if (pOther->GetCollisionGroup() == CG_ENTITY)
	{
		if (m_hOwner != NULL && pOther->GetPlayerOwner() == m_hOwner->GetPlayerOwner())
			return false;

		return true;
	}
#endif

	if (pOther->GetCollisionGroup() == CG_TERRAIN)
		return true;

	return false;
}

bool CProjectile::IsTouching(CBaseEntity* pOther, Vector& vecPoint) const
{
	switch (pOther->GetCollisionGroup())
	{
	case CG_ENTITY:
	{
		vecPoint = GetGlobalOrigin();

		CDigitank* pTank = dynamic_cast<CDigitank*>(pOther);
		float flBoundingRadius = pOther->GetBoundingRadius();
		if (pTank)
			flBoundingRadius = pTank->GetShieldBlockRadius();

		if ((pOther->GetGlobalOrigin() - GetGlobalOrigin()).LengthSqr() < flBoundingRadius*flBoundingRadius)
			return true;
		break;
	}

	case CG_TERRAIN:
		return DigitanksGame()->GetTerrain()->Collide(GetGlobalOrigin(), GetGlobalOrigin(), vecPoint);

	case CG_PROP:;
		//return pOther->Collide(GetGlobalOrigin(), GetGlobalOrigin(), vecPoint);
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

			Vector vecProjectileOrigin = GetGlobalOrigin();
			vecProjectileOrigin = DigitanksGame()->GetTerrain()->GetPointHeight(vecProjectileOrigin);
			vecProjectileOrigin.z += 0.5f;

			mReflect.SetReflection(dynamic_cast<CTerrain*>(pOther)->GetNormalAtPoint(vecProjectileOrigin));

			SetGlobalVelocity((mReflect*GetGlobalVelocity())*0.6f);

			SetGlobalOrigin(vecProjectileOrigin);

			m_iBounces++;
			return;
		}
	}

	if (dynamic_cast<CTerrain*>(pOther))
	{
		if (ShouldExplode())
			pOther->TakeDamage(m_hOwner, this, DAMAGE_EXPLOSION, m_flDamage + GetBonusDamage());
	}
	else
		pOther->TakeDamage(m_hOwner, this, DAMAGE_EXPLOSION, m_flDamage + GetBonusDamage());

	if (ShouldExplode())
		Explode(pOther);
	else
		m_flTimeExploded = 1;	// Remove immediately.

	if (MakesSounds())
	{
		StopSound(_T("sound/bomb-drop.wav"));

		bool bCanSeeOwner;
		if (m_hOwner != NULL && DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksPlayer(), m_hOwner->GetGlobalOrigin()) > 0)
			bCanSeeOwner = true;
		else
			bCanSeeOwner = false;

		if (DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksPlayer(), m_vecLandingSpot) > 0 || bCanSeeOwner)
			EmitSound(_T("sound/explosion.wav"));
	}
}

void CProjectile::OnExplode(CBaseEntity* pInstigator)
{
	BaseClass::OnExplode(pInstigator);

	if (Fragments() && !m_bFragmented)
		Fragment();

	if (MakesSounds())
		StopSound(_T("sound/bomb-drop.wav"));

	m_hTrailParticles.SetActive(false);

	if (!DigitanksGame()->GetCurrentLocalDigitanksPlayer() || DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetVisibilityAtPoint(GetGlobalOrigin()) > 0.1f)
		CreateExplosionSystem();

	if (m_flDamageBonusTime > 0)
		m_flDamageBonusFreeze = RemapValClamped((float)GameServer()->GetGameTime() - m_flDamageBonusTime, 0, DamageBonusTime(), 1, 0);

	DigitanksWindow()->GetHUD()->ClearHintWeapon();
}

bool CProjectile::ShouldPlayExplosionSound()
{
	return DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksPlayer(), m_vecLandingSpot) > 0;
}

void CProjectile::OnSetOwner(CBaseEntity* pOwner)
{
	BaseClass::OnSetOwner(pOwner);

	m_hTrailParticles.SetActive(m_bShouldRender);
}

bool CProjectile::ShouldBeVisible()
{
	return DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksPlayer(), m_vecLandingSpot) > 0;
}

size_t CProjectile::CreateTrailSystem()
{
	return CParticleSystemLibrary::Get()->FindParticleSystem(_T("shell-trail"));
}

void CProjectile::CreateExplosionSystem()
{
}

void CProjectile::ClientEnterGame()
{
	BaseClass::ClientEnterGame();

	m_hTrailParticles.SetActive(m_bShouldRender);
}

REGISTER_ENTITY(CSmallShell);

NETVAR_TABLE_BEGIN(CSmallShell);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CSmallShell);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CSmallShell);
INPUTS_TABLE_END();

float CSmallShell::ExplosionRadius() const
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

INPUTS_TABLE_BEGIN(CMediumShell);
INPUTS_TABLE_END();

float CMediumShell::ExplosionRadius() const
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

INPUTS_TABLE_BEGIN(CLargeShell);
INPUTS_TABLE_END();

float CLargeShell::ExplosionRadius() const
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

INPUTS_TABLE_BEGIN(CAOEShell);
INPUTS_TABLE_END();

void CAOEShell::Precache()
{
	PrecacheParticleSystem(_T("aoe-explosion-strategy"));
	PrecacheParticleSystem(_T("aoe-explosion-artillery"));
	PrecacheParticleSystem(_T("aoe-trail"));
}

void CAOEShell::CreateExplosionSystem()
{
	if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
		CParticleSystemLibrary::AddInstance(_T("aoe-explosion-strategy"), GetGlobalOrigin());
	else
	{
		size_t iInstance = CParticleSystemLibrary::AddInstance(_T("aoe-explosion-artillery"), GetGlobalOrigin());
		CSystemInstance* pInstance = CParticleSystemLibrary::GetInstance(iInstance);
		if (pInstance)
			pInstance->SetColor(GetBonusDamageColor());
	}
}

size_t CAOEShell::CreateTrailSystem()
{
	return CParticleSystemLibrary::Get()->FindParticleSystem(_T("aoe-trail"));
}

float CAOEShell::ExplosionRadius() const
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

INPUTS_TABLE_BEGIN(CEMP);
INPUTS_TABLE_END();

void CEMP::Precache()
{
	PrecacheParticleSystem(_T("emp-explosion"));
	PrecacheParticleSystem(_T("emp-trail"));
}

void CEMP::CreateExplosionSystem()
{
	size_t iInstance = CParticleSystemLibrary::AddInstance(_T("emp-explosion"), GetGlobalOrigin());
	CSystemInstance* pInstance = CParticleSystemLibrary::GetInstance(iInstance);
	if (pInstance)
		pInstance->SetColor(GetBonusDamageColor());
}

size_t CEMP::CreateTrailSystem()
{
	return CParticleSystemLibrary::Get()->FindParticleSystem(_T("emp-trail"));
}

REGISTER_ENTITY(CICBM);

NETVAR_TABLE_BEGIN(CICBM);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CICBM);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CICBM);
INPUTS_TABLE_END();

REGISTER_ENTITY(CGrenade);

NETVAR_TABLE_BEGIN(CGrenade);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CGrenade);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, EAngle, m_angAngle);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, EAngle, m_angRotation);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CGrenade);
INPUTS_TABLE_END();

void CGrenade::Precache()
{
	PrecacheModel(_T("models/weapons/grenade.toy"));
}

void CGrenade::Spawn()
{
	BaseClass::Spawn();

	SetModel(_T("models/weapons/grenade.toy"));

	m_angAngle = EAngle(RandomFloat(-90, 90), RandomFloat(0, 360), RandomFloat(-90, 90));
	m_angRotation = EAngle(RandomFloat(-5, 5), RandomFloat(-5, 5), RandomFloat(-5, 5));
}

EAngle CGrenade::GetRenderAngles() const
{
	return m_angAngle + m_angRotation * (GameServer()->GetGameTime() * 50);
}

void CGrenade::SpecialCommand()
{
	if (ShouldExplode() && m_flTimeExploded == 0.0 && !m_bFragmented)
		Explode();
}

void CGrenade::OnExplode(CBaseEntity* pInstigator)
{
	BaseClass::OnExplode(pInstigator);

	SetModel(_T(""));
}

REGISTER_ENTITY(CDaisyChain);

NETVAR_TABLE_BEGIN(CDaisyChain);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CDaisyChain);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flExplosionRadius);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CDaisyChain);
INPUTS_TABLE_END();

void CDaisyChain::Spawn()
{
	BaseClass::Spawn();

	m_flExplosionRadius = 20;
}

void CDaisyChain::SpecialCommand()
{
	if (ShouldExplode() && m_flTimeExploded == 0.0 && !m_bFragmented)
		Explode();
}

void CDaisyChain::OnExplode(CBaseEntity* pInstigator)
{
	BaseClass::OnExplode(pInstigator);

	// 20, 16, 12
	// The third will be the last.
	if (m_flExplosionRadius < 13)
		return;

	if (!GameNetwork()->IsHost())
		return;

	CDaisyChain* pProjectile = GameServer()->Create<CDaisyChain>(GetClassName());
	pProjectile->SetOwner(m_hOwner);
	pProjectile->SetGlobalVelocity(GetGlobalVelocity());
	pProjectile->SetGlobalGravity(GetGlobalGravity());
	pProjectile->SetLandingSpot(m_vecLandingSpot);
	pProjectile->SetGlobalOrigin(GetGlobalOrigin());
	pProjectile->m_flExplosionRadius = m_flExplosionRadius - 4;
	pProjectile->m_flDamage = m_flDamage - 1.0f;
	DigitanksGame()->AddProjectileToWaitFor();

	DigitanksGame()->GetOverheadCamera()->ReplaceProjectileTarget(pProjectile);
}

REGISTER_ENTITY(CClusterBomb);

NETVAR_TABLE_BEGIN(CClusterBomb);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CClusterBomb);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flExplosionRadius);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CClusterBomb);
INPUTS_TABLE_END();

void CClusterBomb::Spawn()
{
	BaseClass::Spawn();

	m_flExplosionRadius = 20;
}

void CClusterBomb::SpecialCommand()
{
	if (m_flExplosionRadius < 15)
		return;

	if (m_flTimeExploded == 0.0)
		Explode();
}

void CClusterBomb::OnExplode(CBaseEntity* pInstigator)
{
	BaseClass::OnExplode(pInstigator);

	if (m_flExplosionRadius < 15)
		return;

	if (!GameNetwork()->IsHost())
		return;

	for (size_t i = 0; i < 6; i++)
	{
		CClusterBomb* pProjectile = GameServer()->Create<CClusterBomb>(GetClassName());
		pProjectile->SetOwner(m_hOwner);
		pProjectile->SetGlobalVelocity(Vector(RandomFloat(-10, 10), RandomFloat(10, 30), RandomFloat(-10, 10)));
		pProjectile->SetGlobalGravity(Vector(0, 0, DigitanksGame()->GetGravity()));
		pProjectile->SetLandingSpot(m_vecLandingSpot);
		pProjectile->SetGlobalOrigin(GetGlobalOrigin());
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

INPUTS_TABLE_BEGIN(CEarthshaker);
INPUTS_TABLE_END();

REGISTER_ENTITY(CSploogeShell);

NETVAR_TABLE_BEGIN(CSploogeShell);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CSploogeShell);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CSploogeShell);
INPUTS_TABLE_END();

void CSploogeShell::Precache()
{
	PrecacheModel(_T("models/weapons/bolt.toy"));
	PrecacheParticleSystem(_T("bolt-trail"));
	PrecacheParticleSystem(_T("bolt-explosion"));
}

void CSploogeShell::Spawn()
{
	BaseClass::Spawn();

	SetModel(_T("models/weapons/bolt.toy"));
}

EAngle CSploogeShell::GetRenderAngles() const
{
	if (GetGlobalVelocity().LengthSqr() == 0)
		return EAngle(-90, 0, 0);

	return VectorAngles(GetGlobalVelocity());
}

size_t CSploogeShell::CreateTrailSystem()
{
	return CParticleSystemLibrary::Get()->FindParticleSystem(_T("bolt-trail"));
}

void CSploogeShell::CreateExplosionSystem()
{
	CParticleSystemLibrary::AddInstance(_T("bolt-explosion"), GetGlobalOrigin());
}

REGISTER_ENTITY(CTractorBomb);

NETVAR_TABLE_BEGIN(CTractorBomb);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTractorBomb);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CTractorBomb);
INPUTS_TABLE_END();

void CTractorBomb::Precache()
{
	PrecacheParticleSystem(_T("tractor-bomb-explosion"));
	PrecacheParticleSystem(_T("tractor-bomb-trail"));
}

void CTractorBomb::SpecialCommand()
{
	if (ShouldExplode() && m_flTimeExploded == 0.0 && !m_bFragmented)
		Explode();
}

size_t CTractorBomb::CreateTrailSystem()
{
	return CParticleSystemLibrary::Get()->FindParticleSystem(_T("tractor-bomb-trail"));
}

void CTractorBomb::CreateExplosionSystem()
{
	CParticleSystemLibrary::AddInstance(_T("tractor-bomb-explosion"), GetGlobalOrigin());
}

REGISTER_ENTITY(CArtilleryShell);

NETVAR_TABLE_BEGIN(CArtilleryShell);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CArtilleryShell);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CArtilleryShell);
INPUTS_TABLE_END();

void CArtilleryShell::Precache()
{
	PrecacheParticleSystem(_T("emp-explosion"));
	PrecacheParticleSystem(_T("emp-trail"));
}

void CArtilleryShell::CreateExplosionSystem()
{
	CParticleSystemLibrary::AddInstance(_T("emp-explosion"), GetGlobalOrigin());
}

size_t CArtilleryShell::CreateTrailSystem()
{
	return CParticleSystemLibrary::Get()->FindParticleSystem(_T("emp-trail"));
}

REGISTER_ENTITY(CArtilleryAoE);

NETVAR_TABLE_BEGIN(CArtilleryAoE);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CArtilleryAoE);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CArtilleryAoE);
INPUTS_TABLE_END();

void CArtilleryAoE::Precache()
{
	PrecacheParticleSystem(_T("aoe-explosion-strategy"));
	PrecacheParticleSystem(_T("aoe-trail"));
}

void CArtilleryAoE::CreateExplosionSystem()
{
	CParticleSystemLibrary::AddInstance(_T("aoe-explosion-strategy"), GetGlobalOrigin());
}

size_t CArtilleryAoE::CreateTrailSystem()
{
	return CParticleSystemLibrary::Get()->FindParticleSystem(_T("aoe-trail"));
}

REGISTER_ENTITY(CArtilleryICBM);

NETVAR_TABLE_BEGIN(CArtilleryICBM);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CArtilleryICBM);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CArtilleryICBM);
INPUTS_TABLE_END();

REGISTER_ENTITY(CDevastator);

NETVAR_TABLE_BEGIN(CDevastator);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CDevastator);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CDevastator);
INPUTS_TABLE_END();

size_t CInfantryFlak::s_iTrailSystem = ~0;

REGISTER_ENTITY(CInfantryFlak);

NETVAR_TABLE_BEGIN(CInfantryFlak);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CInfantryFlak);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CInfantryFlak);
INPUTS_TABLE_END();

void CInfantryFlak::Precache()
{
	PrecacheModel(_T("models/weapons/bolt.toy"));
	PrecacheParticleSystem(_T("bolt-trail"));
	PrecacheParticleSystem(_T("bolt-explosion"));

	s_iTrailSystem = CParticleSystemLibrary::Get()->FindParticleSystem(_T("bolt-trail"));
}

void CInfantryFlak::Spawn()
{
	BaseClass::Spawn();

	SetModel(_T("models/weapons/bolt.toy"));
}

EAngle CInfantryFlak::GetRenderAngles() const
{
	if (GetGlobalVelocity().LengthSqr() == 0)
		return EAngle(-90, 0, 0);

	return VectorAngles(GetGlobalVelocity());
}

size_t CInfantryFlak::CreateTrailSystem()
{
	return s_iTrailSystem;
}

void CInfantryFlak::CreateExplosionSystem()
{
	CParticleSystemLibrary::AddInstance(_T("bolt-explosion"), GetGlobalOrigin());
}

REGISTER_ENTITY(CTorpedo);

NETVAR_TABLE_BEGIN(CTorpedo);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTorpedo);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bBurrowing);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CTorpedo);
INPUTS_TABLE_END();

CTorpedo::CTorpedo()
{
	m_flTimeExploded = 0;
	m_bBurrowing = false;
	m_flDamage = 0;
}

void CTorpedo::Precache()
{
	PrecacheParticleSystem(_T("torpedo-trail"));
	PrecacheParticleSystem(_T("torpedo-explosion"));
}

void CTorpedo::Spawn()
{
	BaseClass::Spawn();

	m_bBurrowing = false;
	m_flDamage = 0;
}

size_t CTorpedo::CreateTrailSystem()
{
	return CParticleSystemLibrary::Get()->FindParticleSystem(_T("torpedo-trail"));
}

void CTorpedo::Think()
{
	if (!GameNetwork()->IsHost())
	{
		if (m_bBurrowing)
		{
			float flDistance = GameServer()->GetFrameTime() * 10;

			Vector vecDirection = m_vecLandingSpot - GetGlobalOrigin();
			vecDirection.z = 0;

			if (vecDirection.LengthSqr() < flDistance*flDistance*2)
			{
				m_bBurrowing = false;
				Explode();
			}
		}

		return;
	}

	if (m_bBurrowing)
	{
		float flDistance = GameServer()->GetFrameTime() * 10;

		bool bSpeedTorpedo = false;

		if (DigitanksGame()->GetCurrentLocalDigitanksPlayer()->GetVisibilityAtPoint(GetGlobalOrigin()) < 0.3f)
			bSpeedTorpedo = true;

		if (GetOwner() && GetOwner()->GetPlayerOwner() && GetOwner()->GetPlayerOwner()->IsHumanControlled())
			bSpeedTorpedo = false;

		if (bSpeedTorpedo)
			flDistance *= 2;

		Vector vecDirection = m_vecLandingSpot - GetGlobalOrigin();
		vecDirection.z = 0;

		if (vecDirection.LengthSqr() < flDistance*flDistance*2)
		{
			m_bBurrowing = false;
			Explode();
		}

		// Insert Jaws theme here.
		Vector vecPosition = GetGlobalOrigin() + vecDirection.Normalized() * flDistance;
		SetGlobalOrigin(DigitanksGame()->GetTerrain()->GetPointHeight(vecPosition));
	}
	else
	{
		// Sometimes the collide raytrace fails. Fuck that jazz.
		if (GetGlobalOrigin().z < DigitanksGame()->GetTerrain()->GetHeight(GetGlobalOrigin().x, GetGlobalOrigin().y))
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
		SetGlobalVelocity(Vector());
		SetGlobalGravity(Vector());
	}

	// Don't call superclass!
}

void CTorpedo::Explode(CBaseEntity* pInstigator)
{
	CSupplyLine* pClosest = NULL;
	while (true)
	{
		pClosest = CBaseEntity::FindClosest<CSupplyLine>(GetGlobalOrigin(), pClosest);

		if (!pClosest)
			break;

		if (pClosest->GetPlayerOwner() == GetOwner()->GetPlayerOwner())
			continue;

		if (!pClosest->GetSupplier() || !pClosest->GetEntity())
			continue;

		if (pClosest->Distance(GetGlobalOrigin()) > ExplosionRadius())
			break;

		pClosest->Intercept(0.5f);

//		DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_TORPEDO);
	}

	// Explode before we disable. Disabling removes shields and torpedos only damage tanks with no shields,
	// so we want the first shot to take down shields and not damage.
	BaseClass::Explode(pInstigator);

	CDigitank* pClosestTank = NULL;
	while (true)
	{
		pClosestTank = CBaseEntity::FindClosest<CDigitank>(GetGlobalOrigin(), pClosestTank);

		if (!pClosestTank)
			break;

		if (pClosestTank->GetPlayerOwner() == GetOwner()->GetPlayerOwner())
			continue;

		if (pClosestTank->Distance(GetGlobalOrigin()) > ExplosionRadius())
			break;

		pClosestTank->Disable(1);
	}

	if (DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksPlayer(), GetGlobalOrigin()) > 0.5f)
	{
		DigitanksGame()->GetDigitanksRenderer()->BloomPulse();
		CParticleSystemLibrary::AddInstance(_T("torpedo-explosion"), GetGlobalOrigin());
	}
}

REGISTER_ENTITY(CTreeCutter);

NETVAR_TABLE_BEGIN(CTreeCutter);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTreeCutter);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CTreeCutter);
INPUTS_TABLE_END();
