#include "projectile.h"

#include <GL/glew.h>
#include <maths.h>
#include <mtrand.h>

#include <renderer/particles.h>

#include <digitanks/digitanksgame.h>
#include <digitanks/dt_camera.h>
#include <digitanks/dt_renderer.h>
#include "ui/instructor.h"
#include "ui/digitankswindow.h"

static float g_aflWeaponEnergies[WEAPON_MAX] =
{
	0.0f,

	2.0f,	// small
	5.0f,	// medium
	8.0f,	// large
	6.0f,	// AoE
	6.0f,	// emp
	4.0f,	// tractor bomb
	3.0f,	// splooge
	6.0f,	// icbm
	4.0f,	// grenade
	4.0f,	// earthshaker

	6.0f,	// machine gun
	3.0f,	// torpedo
	8.0f,	// artillery

	0.0f,	// airstrike
	0.0f,	// fireworks
};

static float g_aflWeaponDamages[WEAPON_MAX] =
{
	0.0f,

	2.0f,	// small
	5.0f,	// medium
	8.0f,	// large
	4.0f,	// AoE
	6.0f,	// emp
	1.0f,	// tractor bomb
	7.0f,	// splooge
	7.0f,	// icbm
	8.0f,	// grenade
	1.0f,	// earthshaker

	0.12f,	// machine gun
	0.0f,	// torpedo
	1.3f,	// artillery

	5.0f,	// airstrike
	0.0f,	// fireworks
};

static size_t g_aiWeaponShells[WEAPON_MAX] =
{
	0,

	1,	// small
	1,	// medium
	1,	// large
	1,	// AoE
	1,	// emp
	1,	// tractor bomb
	20,	// splooge
	1,	// icbm
	1,	// grenade
	1,	// earthshaker

	20,	// machine gun
	1,	// torpedo
	3,	// artillery

	1,	// airstrike
	1,	// fireworks
};

static float g_aflWeaponFireInterval[WEAPON_MAX] =
{
	0,

	0,		// small
	0,		// medium
	0,		// large
	0,		// AoE
	0,		// emp
	0,		// tractor bomb
	0.01f,	// splooge
	0,		// icbm
	0,		// grenade
	0,		// earthshaker

	0.1f,	// machine gun
	0,		// torpedo
	0.25f,	// artillery

	0,		// airstrike
	0,		// fireworks
};

static char16_t* g_apszWeaponNames[WEAPON_MAX] =
{
	L"None",

	L"Little Boy",
	L"Fat Man",
	L"Big Mama",
	L"Plasma Charge",
	L"Electro-Magnetic Pulse",
	L"Tractor Bomb",
	L"Grapeshot",
	L"ICBM",
	L"Grenade",
	L"Earthshaker",

	L"Flak Cannon",
	L"Torpedo",
	L"Artillery Shell",

	L"Airstrike",
	L"Fireworks",
};

static char16_t* g_apszWeaponDescriptions[WEAPON_MAX] =
{
	L"None",

	L"This light projectile bomb does just enough damage to keep the enemy from regenerating his shields next turn.",
	L"This medium projectile bomb is a good tradeoff between firepower and defense.",
	L"This heavy projectile bomb packs a mean punch at the cost of your defense for the next turn.",
	L"This large area of effect projectile bomb is good for attacking a group of tanks.",
	L"This medium projectile bomb sends out an electonic pulse that does extra damage against shields but relatively little damage against tank hulls.",
	L"This light projectile bomb does very little damage, but can knock tanks around a great deal.",
	L"This light attack fires a stream of small projectiles at your enemy. It can be deadly at close range, but loses effectiveness with distance.",
	L"This heavy projectile breaks into multiple fragments before it falls down onto its target.",
	L"This heavy projectile bounces three times before it explodes. Chunk it into holes to find out-of-reach targets.",
	L"This projectile bomb does very little damage but is effective at creating a rather large hole in the ground.",

	L"The infantry's light mounted gun is its main firepower.",
	L"This special attack targets supply lines. It does no damage but it can sever structures from the enemy network and force them to become neutral.",
	L"The artillery fires a salvo of shells which do double damage against shields but half damage against structures.",

	L"Rain fire and brimstone upon your enemies.",
	L"You won! Fireworks are in order.",
};

NETVAR_TABLE_BEGIN(CProjectile);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CProjectile);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flTimeCreated);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flTimeExploded);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bFallSoundPlayed);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CDigitank>, m_hOwner);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flDamage);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecLandingSpot);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bShouldRender);
//	size_t						m_iParticleSystem;	// Generated on load
SAVEDATA_TABLE_END();

CProjectile::CProjectile()
{
	m_flTimeCreated = GameServer()?GameServer()->GetGameTime():0;
	m_flTimeExploded = 0;

	m_bFallSoundPlayed = false;

	m_bShouldRender = true;

	m_bFragmented = false;
	m_iBounces = 0;
}

void CProjectile::Precache()
{
	PrecacheParticleSystem(L"shell-trail");
	PrecacheSound(L"sound/bomb-drop.wav");
	PrecacheSound(L"sound/explosion.wav");
}

void CProjectile::Think()
{
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

	if (Fragments() && !m_bFragmented && GetVelocity().y < 10.0f && m_flTimeExploded == 0.0f)
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

	if (GetOrigin().y < DigitanksGame()->GetTerrain()->GetHeight(GetOrigin().x, GetOrigin().z) - 20 || GetOrigin().y < -100)
		Delete();

	else if (m_flTimeExploded != 0.0f && GameServer()->GetGameTime() - m_flTimeExploded > 2.0f)
		Delete();
}

void CProjectile::ModifyContext(class CRenderingContext* pContext)
{
	BaseClass::ModifyContext(pContext);
}

void CProjectile::OnRender()
{
	if (!m_bShouldRender)
		return;

	BaseClass::OnRender();

	if (m_flTimeExploded == 0.0f)
	{
		CRenderingContext c(GameServer()->GetRenderer());
		c.Scale(ShellRadius(), ShellRadius(), ShellRadius());
		c.SetColor(Color(255, 255, 255));
		c.RenderSphere();
	}
	else
	{
		float flAlpha = RemapValClamped(GameServer()->GetGameTime()-m_flTimeExploded, 0.2f, 1.2f, 1, 0);
		if (flAlpha > 0)
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
			pOther->TakeDamage(m_hOwner, this, m_flDamage);
	}
	else
		pOther->TakeDamage(m_hOwner, this, m_flDamage);

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

void CProjectile::Explode(CBaseEntity* pInstigator)
{
	SetVelocity(Vector());
	SetGravity(Vector());

	bool bHit = false;
	if (m_flDamage > 0)
		bHit = DigitanksGame()->Explode(m_hOwner, this, ExplosionRadius(), m_flDamage, pInstigator, (m_hOwner == NULL)?NULL:m_hOwner->GetTeam());

	m_flTimeExploded = GameServer()->GetGameTime();

	if (m_bShouldRender)
		DigitanksGame()->GetDigitanksCamera()->Shake(GetOrigin(), 3);

	bool bCanSeeOwner;
	if (m_hOwner != NULL && DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), m_hOwner->GetOrigin()) > 0)
		bCanSeeOwner = true;
	else
		bCanSeeOwner = false;

	if (MakesSounds() && DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), m_vecLandingSpot) > 0 || bCanSeeOwner)
		EmitSound(L"sound/explosion.wav");

	if (m_hOwner != NULL && dynamic_cast<CTerrain*>(pInstigator) && !bHit)
		m_hOwner->Speak(TANKSPEECH_MISSED);

	if (m_iParticleSystem != ~0)
	{
		CParticleSystemLibrary::StopInstance(m_iParticleSystem);
		m_iParticleSystem = 0;
	}

	if (DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), GetOrigin()) > 0.5f)
		DigitanksGame()->GetDigitanksRenderer()->BloomPulse();
}

void CProjectile::SetOwner(CDigitank* pOwner)
{
	m_hOwner = pOwner;
	if (pOwner)
		SetOrigin(pOwner->GetOrigin() + Vector(0, 1, 0));
	SetSimulated(true);
	SetCollisionGroup(CG_POWERUP);

	if (DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), m_vecLandingSpot) > 0 || DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), m_hOwner->GetOrigin()) > 0)
		m_bShouldRender = true;
	else
		m_bShouldRender = false;

	if (m_bShouldRender)
	{
		m_iParticleSystem = CreateParticleSystem();
		if (m_iParticleSystem != ~0)
			CParticleSystemLibrary::GetInstance(m_iParticleSystem)->FollowEntity(this);
	}

	m_flDamage = GetWeaponDamage(GetWeaponType())/GetWeaponShells(GetWeaponType());
}

size_t CProjectile::CreateParticleSystem()
{
	return CParticleSystemLibrary::AddInstance(L"shell-trail", GetOrigin());
}

void CProjectile::ClientEnterGame()
{
	BaseClass::ClientEnterGame();

	if (m_bShouldRender)
	{
		m_iParticleSystem = CreateParticleSystem();
		if (m_iParticleSystem != ~0)
			CParticleSystemLibrary::GetInstance(m_iParticleSystem)->FollowEntity(this);
	}
}

float CProjectile::GetWeaponEnergy(weapon_t eProjectile)
{
	return g_aflWeaponEnergies[eProjectile];
}

float CProjectile::GetWeaponDamage(weapon_t eProjectile)
{
	return g_aflWeaponDamages[eProjectile];
}

size_t CProjectile::GetWeaponShells(weapon_t eProjectile)
{
	return g_aiWeaponShells[eProjectile];
}

float CProjectile::GetWeaponFireInterval(weapon_t eProjectile)
{
	return g_aflWeaponFireInterval[eProjectile];
}

char16_t* CProjectile::GetWeaponName(weapon_t eProjectile)
{
	return g_apszWeaponNames[eProjectile];
}

char16_t* CProjectile::GetWeaponDescription(weapon_t eProjectile)
{
	return g_apszWeaponDescriptions[eProjectile];
}

NETVAR_TABLE_BEGIN(CSmallShell);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CSmallShell);
SAVEDATA_TABLE_END();

NETVAR_TABLE_BEGIN(CMediumShell);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CMediumShell);
SAVEDATA_TABLE_END();

NETVAR_TABLE_BEGIN(CLargeShell);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CLargeShell);
SAVEDATA_TABLE_END();

NETVAR_TABLE_BEGIN(CAOEShell);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CAOEShell);
SAVEDATA_TABLE_END();

NETVAR_TABLE_BEGIN(CEMP);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CEMP);
SAVEDATA_TABLE_END();

NETVAR_TABLE_BEGIN(CICBM);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CICBM);
SAVEDATA_TABLE_END();

NETVAR_TABLE_BEGIN(CGrenade);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CGrenade);
SAVEDATA_TABLE_END();

NETVAR_TABLE_BEGIN(CEarthshaker);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CEarthshaker);
SAVEDATA_TABLE_END();

NETVAR_TABLE_BEGIN(CSploogeShell);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CSploogeShell);
SAVEDATA_TABLE_END();

NETVAR_TABLE_BEGIN(CTractorBomb);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTractorBomb);
SAVEDATA_TABLE_END();

NETVAR_TABLE_BEGIN(CArtilleryShell);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CArtilleryShell);
SAVEDATA_TABLE_END();

NETVAR_TABLE_BEGIN(CInfantryFlak);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CInfantryFlak);
SAVEDATA_TABLE_END();

size_t CInfantryFlak::CreateParticleSystem()
{
	return ~0;
}

NETVAR_TABLE_BEGIN(CTorpedo);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTorpedo);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bBurrowing);
SAVEDATA_TABLE_END();

CTorpedo::CTorpedo()
{
	m_bBurrowing = false;

	m_flDamage = 0;
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

	BaseClass::Explode(pInstigator);

	if (DigitanksGame()->GetVisibilityAtPoint(DigitanksGame()->GetCurrentLocalDigitanksTeam(), GetOrigin()) > 0.5f)
		DigitanksGame()->GetDigitanksRenderer()->BloomPulse();
}
