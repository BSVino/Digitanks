#include "projectile.h"

#include <GL/glew.h>
#include <maths.h>

#include <renderer/particles.h>

#include <digitanks/digitanksgame.h>
#include <digitanks/dt_camera.h>
#include <digitanks/dt_renderer.h>
#include "ui/instructor.h"
#include "ui/digitankswindow.h"

static float g_aflProjectileEnergies[PROJECTILE_MAX] =
{
	2.0f,	// small
	5.0f,	// medium
	8.0f,	// large
	6.0f,	// AoE

	6.0f,	// machine gun
	3.0f,	// torpedo
	8.0f,	// artillery

	0.0f,	// fireworks
};

static float g_aflProjectileDamages[PROJECTILE_MAX] =
{
	2.0f,	// small
	5.0f,	// medium
	8.0f,	// large
	4.0f,	// AoE

	0.12f,	// machine gun
	0.0f,	// torpedo
	1.3f,	// artillery

	0.0f,	// fireworks
};

static char16_t* g_apszProjectileNames[PROJECTILE_MAX] =
{
	L"Little Boy",
	L"Fat Man",
	L"Big Mama",
	L"Plasma Charge",

	L"Flak Cannon",
	L"Torpedo",
	L"Artillery Shell",

	L"Fireworks",
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
		if (m_hOwner != NULL && DigitanksGame()->GetLocalDigitanksTeam()->GetVisibilityAtPoint(m_hOwner->GetOrigin()) > 0)
			bCanSeeOwner = true;
		else
			bCanSeeOwner = false;

		if (DigitanksGame()->GetLocalDigitanksTeam()->GetVisibilityAtPoint(m_vecLandingSpot) > 0 || bCanSeeOwner)
		{
			EmitSound(L"sound/bomb-drop.wav");
			SetSoundVolume(L"sound/bomb-drop.wav", 0.5f);
		}

		m_bFallSoundPlayed = true;
	}

	if (GetOrigin().y < DigitanksGame()->GetTerrain()->GetHeight(GetOrigin().x, GetOrigin().z) - 20 || GetOrigin().y < -100)
		Delete();

	else if (m_flTimeExploded != 0.0f && GameServer()->GetGameTime() - m_flTimeExploded > 2.0f)
		Delete();
}

void CProjectile::ModifyContext(class CRenderingContext* pContext)
{
	BaseClass::ModifyContext(pContext);

	if (m_flTimeExploded > 0.0f)
	{
		if (DigitanksGame()->GetDigitanksRenderer()->ShouldUseFramebuffers())
			pContext->UseFrameBuffer(DigitanksGame()->GetDigitanksRenderer()->GetExplosionBuffer());
	}
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
			c.Scale(ExplosionRadius(), ExplosionRadius(), ExplosionRadius());
			c.SetColor(Color(255, 255, 255, (int)(flAlpha*255)));
			if (!DigitanksGame()->GetDigitanksRenderer()->ShouldUseFramebuffers())
				c.SetBlend(BLEND_ADDITIVE);
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
		if (m_hOwner != NULL && DigitanksGame()->GetLocalDigitanksTeam()->GetVisibilityAtPoint(m_hOwner->GetOrigin()) > 0)
			bCanSeeOwner = true;
		else
			bCanSeeOwner = false;

		if (DigitanksGame()->GetLocalDigitanksTeam()->GetVisibilityAtPoint(m_vecLandingSpot) > 0 || bCanSeeOwner)
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
	if (m_hOwner != NULL && DigitanksGame()->GetLocalDigitanksTeam()->GetVisibilityAtPoint(m_hOwner->GetOrigin()) > 0)
		bCanSeeOwner = true;
	else
		bCanSeeOwner = false;

	if (MakesSounds() && DigitanksGame()->GetLocalDigitanksTeam()->GetVisibilityAtPoint(m_vecLandingSpot) > 0 || bCanSeeOwner)
		EmitSound(L"sound/explosion.wav");

	if (m_hOwner != NULL && dynamic_cast<CTerrain*>(pInstigator) && !bHit)
		m_hOwner->Speak(TANKSPEECH_MISSED);

	if (m_iParticleSystem != ~0)
	{
		CParticleSystemLibrary::StopInstance(m_iParticleSystem);
		m_iParticleSystem = 0;
	}

	if (DigitanksGame()->GetLocalDigitanksTeam()->GetVisibilityAtPoint(GetOrigin()) > 0.5f)
		DigitanksGame()->GetDigitanksRenderer()->BloomPulse();
}

void CProjectile::SetOwner(CDigitank* pOwner)
{
	m_hOwner = pOwner;
	if (pOwner)
		SetOrigin(pOwner->GetOrigin() + Vector(0, 1, 0));
	SetSimulated(true);
	SetCollisionGroup(CG_POWERUP);

	if (DigitanksGame()->GetLocalDigitanksTeam()->GetVisibilityAtPoint(m_vecLandingSpot) > 0 || DigitanksGame()->GetLocalDigitanksTeam()->GetVisibilityAtPoint(m_hOwner->GetOrigin()) > 0)
		m_bShouldRender = true;
	else
		m_bShouldRender = false;

	if (m_bShouldRender)
	{
		m_iParticleSystem = CreateParticleSystem();
		if (m_iParticleSystem != ~0)
			CParticleSystemLibrary::GetInstance(m_iParticleSystem)->FollowEntity(this);
	}

	m_flDamage = GetProjectileDamage(GetProjectileType());
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

float CProjectile::GetProjectileEnergy(projectile_t eProjectile)
{
	return g_aflProjectileEnergies[eProjectile];
}

float CProjectile::GetProjectileDamage(projectile_t eProjectile)
{
	return g_aflProjectileDamages[eProjectile];
}

char16_t* CProjectile::GetProjectileName(projectile_t eProjectile)
{
	return g_apszProjectileNames[eProjectile];
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

	if (DigitanksGame()->GetLocalDigitanksTeam()->GetVisibilityAtPoint(GetOrigin()) > 0.5f)
		DigitanksGame()->GetDigitanksRenderer()->BloomPulse();
}

NETVAR_TABLE_BEGIN(CFireworks);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CFireworks);
SAVEDATA_TABLE_END();

bool CFireworks::ShouldTouch(CBaseEntity* pOther) const
{
	if (!pOther)
		return false;

	if (pOther->GetCollisionGroup() == CG_TERRAIN)
		return true;

	return false;
}
