#include "projectile.h"

#include <GL/glew.h>
#include <maths.h>

#include <renderer/particles.h>

#include "digitanksgame.h"
#include "dt_camera.h"
#include "dt_renderer.h"

NETVAR_TABLE_BEGIN(CProjectile);
NETVAR_TABLE_END();

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
	PrecacheSound("sound/bomb-drop.wav");
	PrecacheSound("sound/explosion.wav");
}

void CProjectile::Think()
{
	if (MakesSounds() && BombDropNoise() && GetVelocity().y < 10.0f && !m_bFallSoundPlayed && m_flTimeExploded == 0.0f)
	{
		if (DigitanksGame()->GetLocalDigitanksTeam()->GetVisibilityAtPoint(m_vecLandingSpot) > 0 || DigitanksGame()->GetLocalDigitanksTeam()->GetVisibilityAtPoint(m_hOwner->GetOrigin()) > 0)
		{
			EmitSound("sound/bomb-drop.wav");
			SetSoundVolume("sound/bomb-drop.wav", 0.5f);
		}

		m_bFallSoundPlayed = true;
	}

	if (GameServer()->GetGameTime() - m_flTimeCreated > 5.0f && m_flTimeExploded == 0.0f)
		Delete();

	else if (m_flTimeExploded != 0.0f && GameServer()->GetGameTime() - m_flTimeExploded > 2.0f)
		Delete();
}

void CProjectile::ModifyContext(class CRenderingContext* pContext)
{
	BaseClass::ModifyContext(pContext);

	if (m_flTimeExploded > 0.0f)
	{
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
			c.Scale(4.0f, 4.0f, 4.0f);
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
		StopSound("sound/bomb-drop.wav");

		if (DigitanksGame()->GetLocalDigitanksTeam()->GetVisibilityAtPoint(m_vecLandingSpot) > 0 || DigitanksGame()->GetLocalDigitanksTeam()->GetVisibilityAtPoint(m_hOwner->GetOrigin()) > 0)
			EmitSound("sound/explosion.wav");
	}
}

void CProjectile::Explode(CBaseEntity* pInstigator)
{
	SetVelocity(Vector());
	SetGravity(Vector());

	bool bHit = false;
	if (m_flDamage > 0)
		bHit = DigitanksGame()->Explode(m_hOwner, this, 4, m_flDamage, pInstigator, m_hOwner->GetTeam());

	m_flTimeExploded = GameServer()->GetGameTime();

	if (m_bShouldRender)
		DigitanksGame()->GetDigitanksCamera()->Shake(GetOrigin(), 3);

	if (MakesSounds() && DigitanksGame()->GetLocalDigitanksTeam()->GetVisibilityAtPoint(m_vecLandingSpot) > 0 || DigitanksGame()->GetLocalDigitanksTeam()->GetVisibilityAtPoint(m_hOwner->GetOrigin()) > 0)
		EmitSound("sound/explosion.wav");

	if (dynamic_cast<CTerrain*>(pInstigator) && !bHit)
		m_hOwner->Speak(TANKSPEECH_MISSED);

	if (m_iParticleSystem != ~0)
	{
		CParticleSystemLibrary::StopInstance(m_iParticleSystem);
		m_iParticleSystem = 0;
	}
}

void CProjectile::SetOwner(CDigitank* pOwner)
{
	m_hOwner = pOwner;
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
}

size_t CProjectile::CreateParticleSystem()
{
	return CParticleSystemLibrary::AddInstance(L"shell-trail", GetOrigin());
}

NETVAR_TABLE_BEGIN(CShell);
NETVAR_TABLE_END();

NETVAR_TABLE_BEGIN(CArtilleryShell);
NETVAR_TABLE_END();

NETVAR_TABLE_BEGIN(CInfantryFlak);
NETVAR_TABLE_END();

size_t CInfantryFlak::CreateParticleSystem()
{
	return ~0;
}

NETVAR_TABLE_BEGIN(CTorpedo);
NETVAR_TABLE_END();

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

		if (pClosest->Distance(GetOrigin()) > 4)
			break;

		pClosest->Intercept(0.5f);
	}

	BaseClass::Explode(pInstigator);
}
