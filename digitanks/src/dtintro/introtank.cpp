#include "introtank.h"

#include <tinker/application.h>
#include <game/gameserver.h>
#include <renderer/game_renderer.h>
#include <renderer/game_renderingcontext.h>
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
	PrecacheParticleSystem("tank-fire");
	PrecacheSound("sound/tank-fire.wav");
	PrecacheModel("models/weapons/shell.toy");
}

void CIntroTank::Think()
{
	BaseClass::Think();

	float flSpeed = fabs(AngleDifference(m_flGoalTurretYaw, m_flCurrentTurretYaw)) * (float)GameServer()->GetFrameTime() * 10;
	m_flCurrentTurretYaw = AngleApproach(m_flGoalTurretYaw, m_flCurrentTurretYaw, flSpeed);
}

void CIntroTank::ModifyContext(class CRenderingContext* pContext) const
{
	BaseClass::ModifyContext(pContext);

	float flWidth = (float)CApplication::Get()->GetWindowWidth();
	float flHeight = (float)CApplication::Get()->GetWindowHeight();

	float flScale = 15*flWidth/flHeight;

	pContext->Scale(flScale, flScale, flScale);
}

bool CIntroTank::ModifyShader(class CRenderingContext* pContext) const
{
	if (GetTeam() && pContext->m_pShader)
	{
		pContext->SetUniform("bColorSwapInAlpha", true);
		pContext->SetUniform("vecColorSwap", GetTeam()->GetColor());
	}

	return true;
}

const Matrix4x4 CIntroTank::GetRenderTransform() const
{
	if (IsRocking())
	{
		EAngle angReturn;

		Vector vecForward, vecRight;
		AngleVectors(GetGlobalAngles(), &vecForward, &vecRight, NULL);
		float flDotForward = -m_vecRockDirection.Dot(vecForward.Normalized());
		float flDotRight = -m_vecRockDirection.Dot(vecRight.Normalized());

		float flLerp = Bias(1-Oscillate((float)(GameServer()->GetGameTime() - m_flStartedRock), 1), 0.7f);

		angReturn.p += flDotForward*flLerp*m_flRockIntensity*45;
		angReturn.r += flDotRight*flLerp*m_flRockIntensity*45;

		Matrix4x4 m = BaseClass::GetRenderTransform();
		return m.AddAngles(angReturn);
	}

	return BaseClass::GetRenderTransform();
}

void CIntroTank::OnRender(class CGameRenderingContext* pContext) const
{
	BaseClass::OnRender(pContext);

	if (GameServer()->GetRenderer()->IsRenderingTransparent())
		return;

	CGameRenderingContext r(GameServer()->GetRenderer(), true);

	r.Translate(Vector(-0.0f, 0, 0.810368f));

	r.Rotate(-m_flCurrentTurretYaw, Vector(0, 0, 1));

	r.RenderModel(m_iTurretModel, this);
}

void CIntroTank::FireBomb(Vector vecLandingSpot, CBaseEntity* pTarget)
{
	CBomb* pBomb = GameServer()->Create<CBomb>("CBomb");

	float flGravity = -200;

	Vector vecDirection = vecLandingSpot - GetGlobalOrigin();
	vecDirection.z = 0;
	Vector vecMuzzle = vecDirection.Normalized() * 13 + Vector(0, 0, 40);

	float flTime;
	Vector vecForce;
	FindLaunchVelocity(GetGlobalOrigin() + vecMuzzle, vecLandingSpot, flGravity, vecForce, flTime, -0.001f);

	pBomb->SetGlobalVelocity(vecForce);
	pBomb->SetGlobalGravity(Vector(0, 0, flGravity));
	pBomb->SetGlobalOrigin(GetGlobalOrigin() + vecMuzzle);
	pBomb->SetExplodeTime(GameServer()->GetGameTime() + flTime);
	pBomb->SetTarget(pTarget);

	CParticleSystemLibrary::AddInstance("tank-fire", GetGlobalOrigin() + vecMuzzle);

	RockTheBoat(0.6f, -vecForce.Normalized());

	EmitSound("sound/tank-fire.wav");
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

	float flTimeSinceRock = (float)(GameServer()->GetGameTime() - m_flStartedRock);
	if (m_flStartedRock && flTimeSinceRock < flTransitionTime)
		return true;

	return false;
}
