#include "introtank.h"

#include <tinker/application.h>
#include <renderer/renderer.h>
#include <renderer/particles.h>
#include <models/models.h>

#include "bomb.h"

REGISTER_ENTITY(CIntroTank);

NETVAR_TABLE_BEGIN(CIntroTank);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CIntroTank);
	SAVEDATA_OMIT(m_flCurrentTurretYaw);
	SAVEDATA_OMIT(m_flGoalTurretYaw);
	SAVEDATA_OMIT(m_iTurretModel);
	SAVEDATA_OMIT(m_flStartedRock);
	SAVEDATA_OMIT(m_flRockIntensity);
	SAVEDATA_OMIT(m_vecRockDirection);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CIntroTank);
INPUTS_TABLE_END();

CIntroTank::CIntroTank()
{
	m_flCurrentTurretYaw = 0;
	m_flGoalTurretYaw = 0;
	m_iTurretModel = ~0;
	m_flStartedRock = 0;
}

void CIntroTank::Precache()
{
	PrecacheParticleSystem(L"tank-fire");
	PrecacheSound(L"sound/tank-fire.wav");
}

void CIntroTank::Think()
{
	BaseClass::Think();

	float flSpeed = fabs(AngleDifference(m_flGoalTurretYaw, m_flCurrentTurretYaw)) * GameServer()->GetFrameTime() * 10;
	m_flCurrentTurretYaw = AngleApproach(m_flGoalTurretYaw, m_flCurrentTurretYaw, flSpeed);
}

void CIntroTank::ModifyContext(class CRenderingContext* pContext, bool bTransparent) const
{
	BaseClass::ModifyContext(pContext, bTransparent);

	float flWidth = (float)CApplication::Get()->GetWindowWidth();
	float flHeight = (float)CApplication::Get()->GetWindowHeight();

	float flScale = 15*flWidth/flHeight;

	pContext->Scale(flScale, flScale, flScale);

	if (GetTeam())
		pContext->SetColorSwap(GetTeam()->GetColor());
}

EAngle CIntroTank::GetRenderAngles() const
{
	if (IsRocking())
	{
		EAngle angReturn = GetAngles();

		Vector vecForward, vecRight;
		AngleVectors(GetAngles(), &vecForward, &vecRight, NULL);
		float flDotForward = -m_vecRockDirection.Dot(vecForward.Normalized());
		float flDotRight = -m_vecRockDirection.Dot(vecRight.Normalized());

		float flLerp = Lerp(1-Oscillate(GameServer()->GetGameTime() - m_flStartedRock, 1), 0.7f);

		angReturn.p += flDotForward*flLerp*m_flRockIntensity*45;
		angReturn.r += flDotRight*flLerp*m_flRockIntensity*45;

		return angReturn;
	}

	return GetAngles();
}

void CIntroTank::OnRender(class CRenderingContext* pContext, bool bTransparent) const
{
	BaseClass::OnRender(pContext, bTransparent);

	if (bTransparent)
		return;

	CRenderingContext r(GameServer()->GetRenderer());

	r.Translate(Vector(-0.0f, 0.810368f, 0));

	r.Rotate(-m_flCurrentTurretYaw, Vector(0, 1, 0));

	if (GetTeam())
		r.SetColorSwap(GetTeam()->GetColor());

	r.RenderModel(m_iTurretModel);
}

void CIntroTank::FireBomb(Vector vecLandingSpot, CBaseEntity* pTarget)
{
	CBomb* pBomb = GameServer()->Create<CBomb>("CBomb");

	pBomb->SetSimulated(true);

	float flGravity = -200;

	Vector vecDirection = vecLandingSpot - GetOrigin();
	vecDirection.y = 0;
	Vector vecMuzzle = vecDirection.Normalized() * 13 + Vector(0, 40, 0);

	float flTime;
	Vector vecForce;
	FindLaunchVelocity(GetOrigin() + vecMuzzle, vecLandingSpot, flGravity, vecForce, flTime, -0.001f);

	pBomb->SetVelocity(vecForce);
	pBomb->SetGravity(Vector(0, flGravity, 0));
	pBomb->SetOrigin(GetOrigin() + vecMuzzle);
	pBomb->SetExplodeTime(GameServer()->GetGameTime() + flTime);
	pBomb->SetTarget(pTarget);

	CParticleSystemLibrary::AddInstance(L"tank-fire", GetOrigin() + vecMuzzle);

	RockTheBoat(0.6f, -vecForce.Normalized());

	EmitSound(L"sound/tank-fire.wav");
}

void CIntroTank::RockTheBoat(float flIntensity, Vector vecDirection)
{
	m_flStartedRock = GameServer()->GetGameTime();
	m_flRockIntensity = flIntensity;
	m_vecRockDirection = vecDirection;
}

bool CIntroTank::IsRocking() const
{
	float flTransitionTime = 1;

	float flTimeSinceRock = GameServer()->GetGameTime() - m_flStartedRock;
	if (m_flStartedRock && flTimeSinceRock < flTransitionTime)
		return true;

	return false;
}
