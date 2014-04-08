#include "dt_camera.h"

#include <maths.h>
#include <mtrand.h>
#include <renderer/game_renderer.h>
#include <tinker/cvar.h>
#include <tinker/keys.h>
#include <network/commands.h>

#include <digitanksgame.h>
#include <ui/digitankswindow.h>
#include <ui/instructor.h>
#include <weapons/cameraguided.h>

REGISTER_ENTITY(COverheadCamera);

NETVAR_TABLE_BEGIN(COverheadCamera);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(COverheadCamera);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(COverheadCamera);
INPUTS_TABLE_END();

void COverheadCamera::Spawn()
{
	BaseClass::Spawn();

	m_flTargetRamp = m_flDistanceRamp = m_flAngleRamp = 0;
	m_flDistance = 40;
	m_angCamera = EAngle(45, 0, 0);
	m_bRotatingCamera = false;
	m_bDraggingCamera = false;
	m_bFastDraggingCamera = false;
	m_flShakeMagnitude = 0;

	m_bMouseDragLeft = m_bMouseDragRight = m_bMouseDragUp = m_bMouseDragDown = false;

	m_flTransitionToProjectileTime = 0;

	DigitanksGame()->SetOverheadCamera(this);
}

void COverheadCamera::SetTarget(Vector vecTarget)
{
	if (GameServer()->IsLoading())
	{
		SnapTarget(vecTarget);
		return;
	}

	vecTarget = DigitanksGame()->GetTerrain()->GetPointHeight(DigitanksGame()->GetTerrain()->ConstrainVectorToMap(vecTarget));

	m_flTargetRamp = GameServer()->GetGameTime();
	m_vecOldTarget = m_vecTarget;
	m_vecNewTarget = vecTarget;
	m_vecVelocity = m_vecGoalVelocity = Vector(0,0,0);
}

void COverheadCamera::SnapTarget(Vector vecTarget)
{
	if (!DigitanksGame()->GetTerrain())
		return;

	vecTarget = DigitanksGame()->GetTerrain()->GetPointHeight(vecTarget);

	m_flTargetRamp = 0;
	m_vecNewTarget = m_vecTarget = vecTarget;
	m_vecVelocity = m_vecGoalVelocity = Vector(0,0,0);
}

void COverheadCamera::SetDistance(float flDistance)
{
	if (GameServer()->IsLoading())
	{
		SnapDistance(flDistance);
		return;
	}

	if (flDistance < 40)
		flDistance = 40;

	if (flDistance > 400)
		flDistance = 400;

	m_flDistanceRamp = GameServer()->GetGameTime();
	m_flOldDistance = m_flDistance;
	m_flNewDistance = flDistance;
}

void COverheadCamera::SnapDistance(float flDistance)
{
	m_flDistanceRamp = 0;
	m_flNewDistance = flDistance;
}

void COverheadCamera::SetAngle(EAngle angCamera)
{
	if (GameServer()->IsLoading())
	{
		SnapAngle(angCamera);
		return;
	}

	m_flAngleRamp = GameServer()->GetGameTime();
	m_angOldAngle = m_angCamera;
	m_angNewAngle = angCamera;
}

void COverheadCamera::SnapAngle(EAngle angCamera)
{
	m_angCamera = angCamera;
}

void COverheadCamera::ZoomOut()
{
	SetDistance(m_flNewDistance+20);

	DigitanksWindow()->GetInstructor()->FinishedLesson("mission-1-zoomview");
}

void COverheadCamera::ZoomIn()
{
	SetDistance(m_flNewDistance-20);

	DigitanksWindow()->GetInstructor()->FinishedLesson("mission-1-zoomview");
}

void COverheadCamera::Shake(Vector vecLocation, float flMagnitude)
{
	m_vecShakeLocation = vecLocation;
	m_flShakeMagnitude = flMagnitude;
}

void COverheadCamera::SetCameraGuidedMissile(CCameraGuidedMissile* pMissile)
{
	TStubbed("Camera guided missile camera");
	m_hCameraGuidedMissile = pMissile;
	m_flCameraGuidedFOV = m_flCameraGuidedFOVGoal = 0;
}

CCameraGuidedMissile* COverheadCamera::GetCameraGuidedMissile()
{
	TStubbed("Camera guided missile camera");
	return m_hCameraGuidedMissile;
}

bool COverheadCamera::HasCameraGuidedMissile()
{
	TStubbed("Camera guided missile camera");
	return m_hCameraGuidedMissile != NULL;
}

void COverheadCamera::ShowEnemyMoves()
{
	CDigitanksPlayer* pCurrentTeam = DigitanksGame()->GetCurrentPlayer();

	if (!pCurrentTeam)
		return;

	if (pCurrentTeam->GetNumTanks() == 0)
		return;

	tvector<const CDigitank*> apTargets;

	Vector vecAveragePosition = Vector(0,0,0);

	for (size_t i = 0; i < pCurrentTeam->GetNumTanks(); i++)
	{
		const CDigitank* pTank = pCurrentTeam->GetTank(i);

		if (!pTank)
			continue;

		if (pTank->GetVisibility() < 0.1f)
			continue;

		if (!pTank->HasFiredWeapon())
			continue;

		Vector vecAim = pTank->GetLastAim();

		CDigitank* pClosestTank = CBaseEntity::FindClosest<CDigitank>(vecAim);
		CStructure* pClosestStructure = NULL;
		if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
			pClosestStructure = CBaseEntity::FindClosest<CStructure>(vecAim);

		if (pClosestTank && pClosestTank->GetPlayerOwner())
		{
			if (pClosestTank->GetPlayerOwner()->IsHumanControlled())
			{
				apTargets.push_back(pTank);
				vecAveragePosition += pTank->GetGlobalOrigin();
			}

			continue;
		}

		if (pClosestStructure && pClosestStructure->GetPlayerOwner() && pClosestStructure->GetPlayerOwner()->IsHumanControlled())
		{
			apTargets.push_back(pTank);
			vecAveragePosition += pTank->GetGlobalOrigin();
		}
	}

	if (apTargets.size() == 0)
	{
		// No targets of interest? Show whatever's the first thing we can find.
		const CDigitank* pFollow = NULL;
		for (size_t i = 0; i < pCurrentTeam->GetNumTanks(); i++)
		{
			const CDigitank* pTank = pCurrentTeam->GetTank(i);

			if (!pTank)
				continue;

			if (pTank->GetVisibility() < 0.1f)
				continue;

			if (!pFollow)
			{
				pFollow = pTank;

				if (pFollow->HasFiredWeapon())
					break;

				continue;
			}

			if (pTank->HasFiredWeapon() && !pFollow->HasFiredWeapon())
			{
				pFollow = pTank;
				break;
			}
		}

		if (!pFollow)
			return;

		if (!pFollow->HasFiredWeapon())
		{
			SetTarget(pFollow->GetRealOrigin());
			return;
		}

		SetTarget((pFollow->GetRealOrigin() + pFollow->GetLastAim())/2);

		if (RandomInt(0, 2) == 0)
		{
			m_hTankTarget = pFollow;
			// Set this so that when we come out of the tank target mode we are looking at the explosion.
			SetTarget(pFollow->GetLastAim());
		}

		// No targets that are interesting to us? Pick a random one.
		return;
	}

	if (apTargets.size() == 1 && RandomInt(0, 2) == 0)
	{
		m_hTankTarget = apTargets[0];
		// Set this so that when we come out of the tank target mode we are looking at the explosion.
		SetTarget(m_hTankTarget->GetLastAim());
		return;
	}

	vecAveragePosition /= (float)apTargets.size();

	// Find the closest target to the center.
	const CDigitank* pClosestTarget = apTargets[0];
	for (size_t i = 1; i < apTargets.size(); i++)
	{
		if ((apTargets[i]->GetGlobalOrigin() - vecAveragePosition).LengthSqr() < (pClosestTarget->GetGlobalOrigin() - vecAveragePosition).LengthSqr())
			pClosestTarget = apTargets[i];
	}

	// Find nearby tanks to that closest tank
	Vector vecAverageNearbyTanks = Vector(0,0,0);
	int iNearbyTargets = 0;
	for (size_t i = 0; i < apTargets.size(); i++)
	{
		if ((apTargets[i]->GetGlobalOrigin() - vecAveragePosition).LengthSqr() < 100*100)
		{
			vecAverageNearbyTanks += apTargets[i]->GetRealOrigin();
			iNearbyTargets++;

			if (apTargets[i]->HasFiredWeapon())
			{
				vecAverageNearbyTanks += apTargets[i]->GetLastAim();
				iNearbyTargets++;
			}
		}
	}

	if (iNearbyTargets > 0)
	{
		vecAverageNearbyTanks /= (float)iNearbyTargets;

		// Point the camera in the center of the nearby targets
		SetTarget(vecAverageNearbyTanks);
	}
	else
		SetTarget(pClosestTarget->GetRealOrigin());
}

void COverheadCamera::ProjectileFired(CProjectile* pProjectile)
{
	if (!m_hTankTarget)
		return;

	if (pProjectile == NULL)
		return;

	if (pProjectile->GetOwner() != m_hTankTarget)
		return;

	if (!m_hTankTarget->HasFiredWeapon())
		return;

	if (m_hTankProjectile != NULL)
		return;

	if (pProjectile->GetOwner() == NULL)
		return;

	if (pProjectile->GetOwner()->GetVisibility() < 0.8f)
		return;

	CDigitank* pOwner = dynamic_cast<CDigitank*>(pProjectile->GetOwner());
	if (!pOwner)
		return;

	if (pProjectile->GetOwner()->GetDigitanksPlayer() && pProjectile->GetOwner()->GetDigitanksPlayer()->GetVisibilityAtPoint(pOwner->GetLastAim()) < 0.8f)
		return;

	m_hTankProjectile = pProjectile;
	m_hTankTarget = NULL;
	m_flTransitionToProjectileTime = GameServer()->GetGameTime();
}

void COverheadCamera::ReplaceProjectileTarget(CProjectile* pTarget)
{
	if (m_hTankProjectile != NULL)
		m_hTankProjectile = pTarget;
}

void COverheadCamera::ClearFollowTarget()
{
	m_hTankTarget = NULL;
	m_hTankProjectile = NULL;
}

void COverheadCamera::EnterGame()
{
	m_flTimeSinceNewGame = GameServer()->GetGameTime();
}

void COverheadCamera::CameraThink()
{
	BaseClass::CameraThink();

	double flGameTime = GameServer()->GetGameTime();
	float flFrameTime = (float)GameServer()->GetFrameTime();
	float flLerpTime = 0.5f;
	float flBiasAmount = 0.7f;

	if (m_hCameraGuidedMissile != NULL)
	{
		m_flCameraGuidedFOVGoal = m_hCameraGuidedMissile->IsBoosting()?1.0f:0.0f;
		if (m_hCameraGuidedMissile->IsBoosting())
			m_flCameraGuidedFOV = Approach(m_flCameraGuidedFOVGoal, m_flCameraGuidedFOV, 10 * (float)GameServer()->GetFrameTime());
		else
			m_flCameraGuidedFOV = Approach(m_flCameraGuidedFOVGoal, m_flCameraGuidedFOV, 1 * (float)GameServer()->GetFrameTime());
	}

	if (m_hTankProjectile != NULL)
	{
		if (m_hTankProjectile->HasExploded())
			m_hTankProjectile = NULL;
	}

	m_vecVelocity.x = Approach(m_vecGoalVelocity.x, m_vecVelocity.x, flFrameTime*200);
	m_vecVelocity.y = Approach(m_vecGoalVelocity.y, m_vecVelocity.y, flFrameTime*200);

	if (m_flTargetRamp && flGameTime - m_flTargetRamp < flLerpTime)
	{
		float flLerp = Bias((float)(flGameTime - m_flTargetRamp)/flLerpTime, flBiasAmount);
		m_vecTarget = LerpValue<Vector>(m_vecOldTarget, m_vecNewTarget, flLerp);
		m_vecVelocity = m_vecGoalVelocity = Vector(0,0,0);
	}
	else
	{
		if (m_flTargetRamp)
		{
			m_vecTarget = m_vecNewTarget;
			m_flTargetRamp = 0;
		}
		else if (DigitanksGame() && DigitanksGame()->GetTerrain())
		{
			Matrix4x4 mRotation;
			mRotation.SetAngles(EAngle(0, m_angCamera.y, 0));

			Vector vecScrollVelocity = m_vecVelocity;
			if (m_bFastDraggingCamera)
				vecScrollVelocity *= 2;

			Vector vecVelocity = mRotation * vecScrollVelocity;

			m_vecTarget = m_vecTarget + vecVelocity * flFrameTime;

			m_vecTarget = DigitanksGame()->GetTerrain()->ConstrainVectorToMap(m_vecTarget);

			m_vecTarget = DigitanksGame()->GetTerrain()->GetPointHeight(m_vecTarget);
		}
	}

	if (m_flDistanceRamp && flGameTime - m_flDistanceRamp < flLerpTime)
	{
		float flLerp = Bias((float)(flGameTime - m_flDistanceRamp)/flLerpTime, flBiasAmount);
		m_flDistance = LerpValue<float>(m_flOldDistance, m_flNewDistance, flLerp);
	}
	else
		m_flDistance = m_flNewDistance;

	if (m_flDistance < 40)
		m_flDistance = 40;

	if (m_flDistance > 400)
		m_flDistance = 400;

	if (m_flAngleRamp && flGameTime - m_flAngleRamp < flLerpTime)
	{
		float flLerp = Bias((float)(flGameTime - m_flAngleRamp)/flLerpTime, flBiasAmount);
		float flPitch = m_angOldAngle.p + AngleDifference(m_angNewAngle.p, m_angOldAngle.p) * flLerp;
		float flYaw = m_angOldAngle.y + AngleDifference(m_angNewAngle.y, m_angOldAngle.y) * flLerp;
		float flRoll = m_angOldAngle.r + AngleDifference(m_angNewAngle.r, m_angOldAngle.r) * flLerp;
		m_angCamera = EAngle(flPitch, flYaw, flRoll);
	}

	m_vecCamera = AngleVector(m_angCamera) * m_flDistance + m_vecTarget;

	m_flShakeMagnitude = Approach(0, m_flShakeMagnitude, (float)GameServer()->GetFrameTime()*5);
	if (m_flShakeMagnitude)
	{
		Vector vecRight, vecUp;
		GameServer()->GetRenderer()->GetCameraVectors(NULL, &vecRight, &vecUp);

		float flX = RandomFloat(-m_flShakeMagnitude, m_flShakeMagnitude);
		float flY = RandomFloat(-m_flShakeMagnitude, m_flShakeMagnitude);
		m_vecShake = vecRight * flX + vecUp * flY;

		float flDistance = (m_vecTarget-m_vecShakeLocation).Length();
		float flScale = 1;
		if (flDistance > 50)
			flScale = RemapValClamped(flDistance, 50.0f, 200.0f, 1.0f, 0.0f);
		m_vecShake *= flScale;
	}
	else
		m_vecShake = Vector(0,0,0);

	if (m_hTankTarget != NULL)
		SetGlobalOrigin(GetTankFollowPosition(m_hTankTarget));

	else if (m_hTankProjectile != NULL)
	{
		float flLerp = (float)RemapValClamped(GameServer()->GetGameTime(), m_flTransitionToProjectileTime, m_flTransitionToProjectileTime + 0.5, 0.0, 1.0);

		if (m_hTankProjectile->GetWeaponType() == PROJECTILE_TORPEDO)
		{
			Vector vecDirection = m_hTankProjectile->GetLandingSpot() - m_hTankProjectile->GetGlobalOrigin();
			vecDirection.z = 0;
			vecDirection.Normalize();

			Vector vecTorpedoFollow = m_hTankProjectile->GetGlobalOrigin() - vecDirection*15 + Vector(0, 0, 15);

			if (flLerp < 1 && m_hTankProjectile->GetOwner())
				SetGlobalOrigin(LerpValue<Vector>(GetTankFollowPosition(dynamic_cast<CDigitank*>(m_hTankProjectile->GetOwner())), vecTorpedoFollow, flLerp));
			else
				SetGlobalOrigin(vecTorpedoFollow);
		}
		else
		{
			Vector vecVelocity = m_hTankProjectile->GetGlobalVelocity();

			Vector vecForward, vecRight, vecUp;
			AngleVectors(VectorAngles(vecVelocity), &vecForward, &vecRight, &vecUp);

			Vector vecProjectileFollow = m_hTankProjectile->GetGlobalOrigin() - vecForward*13 - vecRight*4;

			if (flLerp < 1 && m_hTankProjectile->GetOwner())
				SetGlobalOrigin(LerpValue<Vector>(GetTankFollowPosition(dynamic_cast<CDigitank*>(m_hTankProjectile->GetOwner())), vecProjectileFollow, flLerp));
			else
				SetGlobalOrigin(vecProjectileFollow);
		}
	}
	else if (m_hCameraGuidedMissile != NULL)
		SetGlobalOrigin(m_hCameraGuidedMissile->GetGlobalOrigin());
	else if (GameServer()->GetGameTime() - m_flTimeSinceNewGame < 3 && DigitanksGame()->GetGameType() != GAMETYPE_MENU)
	{
		float flCameraIntroLerp = Gain((float)RemapVal(GameServer()->GetGameTime() - m_flTimeSinceNewGame, 0.0, 3.0, 0.0, 1.0), 0.2f);

		EAngle angIntro = m_angCamera;
		angIntro.y += 90;

		Vector vecIntroTarget = m_vecTarget;
		Vector vecIntroCamera = AngleVector(angIntro) * 400 + vecIntroTarget;

		SetGlobalOrigin(LerpValue<Vector>(vecIntroCamera, m_vecCamera, flCameraIntroLerp) + m_vecShake);
	}
	else
		SetGlobalOrigin(m_vecCamera + m_vecShake);

	if (m_hTankTarget != NULL)
	{
		if (m_hTankTarget->GetCurrentWeapon() == WEAPON_CHARGERAM)
			SetGlobalAngles(VectorAngles(m_hTankTarget->GetGlobalOrigin() + AngleVector(m_hTankTarget->GetGlobalAngles()) * 10 - GetGlobalOrigin()));
		else
			SetGlobalAngles(VectorAngles(m_hTankTarget->GetLastAim() - GetGlobalOrigin()));
	}
	else if (m_hTankProjectile != NULL)
	{
		float flLerp = (float)RemapValClamped(GameServer()->GetGameTime(), m_flTransitionToProjectileTime, m_flTransitionToProjectileTime + 0.5, 0.0, 1.0);

		CDigitank* pTank = dynamic_cast<CDigitank*>(m_hTankProjectile->GetOwner());
		TAssert(pTank);
		if (pTank)
		{
			if (m_hTankProjectile->GetWeaponType() == PROJECTILE_TORPEDO)
			{
				Vector vecTorpedoTarget = m_hTankProjectile->GetGlobalOrigin();

				if (flLerp < 1 && m_hTankProjectile->GetOwner())
					return SetGlobalAngles(VectorAngles(LerpValue<Vector>(pTank->GetLastAim(), vecTorpedoTarget, flLerp) - GetGlobalOrigin()));
				else
					return SetGlobalAngles(VectorAngles(vecTorpedoTarget - GetGlobalOrigin()));
			}

			Vector vecProjectileTarget = m_hTankProjectile->GetGlobalOrigin() + m_hTankProjectile->GetGlobalVelocity();

			if (flLerp < 1 && m_hTankProjectile->GetOwner())
				return SetGlobalAngles(VectorAngles(LerpValue<Vector>(pTank->GetLastAim(), vecProjectileTarget, flLerp) - GetGlobalOrigin()));
			else
				return SetGlobalAngles(VectorAngles(vecProjectileTarget - GetGlobalOrigin()));
		}
	}
	else if (m_hCameraGuidedMissile != NULL)
		return SetGlobalAngles(VectorAngles(m_hCameraGuidedMissile->GetGlobalOrigin() + AngleVector(m_hCameraGuidedMissile->GetViewAngles()) - GetGlobalOrigin()));
	else if (GameServer()->GetGameTime() - m_flTimeSinceNewGame < 3 && DigitanksGame()->GetGameType() != GAMETYPE_MENU)
	{
		float flCameraIntroLerp = Bias((float)RemapVal(GameServer()->GetGameTime() - m_flTimeSinceNewGame, 0.0, 3.0, 0.0, 1.0), 0.2f);

		EAngle angIntro = m_angCamera;
		angIntro.y += 90;

		Vector vecIntroTarget = m_vecTarget;
		Vector vecIntroCamera = AngleVector(angIntro) * 400 + vecIntroTarget;

		return SetGlobalAngles(VectorAngles(LerpValue<Vector>(vecIntroTarget, m_vecTarget, flCameraIntroLerp) + m_vecShake - GetGlobalOrigin()));
	}
	else
		return SetGlobalAngles(VectorAngles(m_vecTarget + m_vecShake - GetGlobalOrigin()));
}

CVar cam_cg_fov("cam_cg_fov", "80");
CVar cam_cg_boost_fov("cam_cg_boost_fov", "70");

float COverheadCamera::GetFOV()
{
	if (m_hCameraGuidedMissile != NULL)
		return RemapVal(m_flCameraGuidedFOV, 0, 1, cam_cg_fov.GetFloat(), cam_cg_boost_fov.GetFloat());

	return BaseClass::GetFOV();
}

float COverheadCamera::GetCameraNear()
{
	if (m_hCameraGuidedMissile != NULL)
		return 1.0f;

	return 10;
}

float COverheadCamera::GetCameraFar()
{
	if (m_hCameraGuidedMissile != NULL)
		return 1000;

	return 1000;
}

Vector COverheadCamera::GetTankFollowPosition(CDigitank* pTank)
{
	TAssert(pTank);
	if (!pTank)
		return Vector();

	Vector vecTarget = pTank->GetLastAim();

	Vector vecForward, vecRight, vecUp;
	AngleVectors(VectorAngles(vecTarget - pTank->GetGlobalOrigin()), &vecForward, &vecRight, &vecUp);

	return pTank->GetGlobalOrigin() - vecForward*20 - vecRight*6 - vecUp*5;
}

// Camera guided missile angles
CLIENT_GAME_COMMAND(CGAng)
{
	if (pCmd->GetNumArguments() < 4)
	{
		TMsg("CGAng with less than 4 arguments.\n");
		return;
	}

	CEntityHandle<CCameraGuidedMissile> hMissile(pCmd->ArgAsUInt(0));

	if (!hMissile)
	{
		TMsg("CGAng with invalid missile.\n");
		return;
	}

	if (!hMissile->GetOwner())
	{
		TMsg("CGAng with no owner.\n");
		return;
	}

	if (!ToDigitanksPlayer(hMissile->GetOwner()))
	{
		TMsg("CGAng with no team.\n");
		return;
	}

	if (ToDigitanksPlayer(hMissile->GetOwner())->GetClient() != (int)iClient)
	{
		TMsg("CGAng missile does not belong to this client.\n");
		return;
	}

	EAngle angMissile(pCmd->ArgAsFloat(1), pCmd->ArgAsFloat(2), pCmd->ArgAsFloat(3));
	hMissile->SetViewAngles(angMissile);
	hMissile->SetGlobalAngles(angMissile);
}

void COverheadCamera::MouseMotion(int x, int y, int dx, int dy)
{
	if (m_hCameraGuidedMissile != NULL)
	{
		TUnimplemented();
		EAngle angMissile = m_hCameraGuidedMissile->GetViewAngles();
		angMissile.y += (dx/5.0f);
		angMissile.p -= (dy/5.0f);

		if (angMissile.p > 89)
			angMissile.p = 89;

		if (angMissile.p < -89)
			angMissile.p = -89;

		while (angMissile.y > 180)
			angMissile.y -= 360;

		while (angMissile.y < -180)
			angMissile.y += 360;

		if (GameNetwork()->IsConnected() && !GameNetwork()->IsHost())
			CGAng.RunCommand(tsprintf(tstring("%d %f %f %f"), m_hCameraGuidedMissile->GetHandle(), angMissile.p, angMissile.y, angMissile.r));

		m_hCameraGuidedMissile->SetViewAngles(angMissile);
		m_hCameraGuidedMissile->SetGlobalAngles(angMissile);
	}
	else if (m_bRotatingCamera)
	{
		m_angCamera.y += (dx/5.0f);
		m_angCamera.p += (dy/5.0f);

		if (m_angCamera.p > 89)
			m_angCamera.p = 89;

		if (m_angCamera.p < 20)
			m_angCamera.p = 20;

		while (m_angCamera.y > 180)
			m_angCamera.y -= 360;

		while (m_angCamera.y < -180)
			m_angCamera.y += 360;
	}
	else if (m_bDraggingCamera)
	{
		Matrix4x4 mRotation;
		mRotation.SetAngles(EAngle(0, m_angCamera.y, 0));

		float flStrength = 2;
		if (m_bFastDraggingCamera)
			flStrength = 4;

		flStrength *= DigitanksWindow()->ShouldReverseSpacebar()?-1:1;

		Vector vecVelocity = mRotation * Vector((float)dy*flStrength, (float)dx*flStrength, 0);

		SetTarget(m_vecTarget + vecVelocity);

		DigitanksWindow()->GetInstructor()->FinishedLesson("mission-1-moveview2");
	}
	else if (DigitanksWindow()->ShouldConstrainMouse() && !m_bDraggingCamera)
	{
		if (!m_bMouseDragLeft && x < 15 && !DigitanksGame()->IsFeatureDisabled(DISABLE_VIEW_MOVE))
		{
			m_bMouseDragLeft = true;
			m_vecGoalVelocity.y = -80.0f;
		}

		if (m_bMouseDragLeft && x > 15)
		{
			m_bMouseDragLeft = false;
			m_vecGoalVelocity.y = 0;
			DigitanksWindow()->GetInstructor()->FinishedLesson("mission-1-moveview");
		}

		if (!m_bMouseDragUp && y < 15 && !DigitanksGame()->IsFeatureDisabled(DISABLE_VIEW_MOVE))
		{
			m_bMouseDragUp = true;
			m_vecGoalVelocity.x = -80.0f;
		}

		if (m_bMouseDragUp && y > 15)
		{
			m_bMouseDragUp = false;
			m_vecGoalVelocity.x = 0;
			DigitanksWindow()->GetInstructor()->FinishedLesson("mission-1-moveview");
		}

		if (!m_bMouseDragRight && x > DigitanksWindow()->GetWindowWidth()-15 && !DigitanksGame()->IsFeatureDisabled(DISABLE_VIEW_MOVE))
		{
			m_bMouseDragRight = true;
			m_vecGoalVelocity.y = 80.0f;
		}

		if (m_bMouseDragRight && x < DigitanksWindow()->GetWindowWidth()-15)
		{
			m_bMouseDragRight = false;
			m_vecGoalVelocity.y = 0;
			DigitanksWindow()->GetInstructor()->FinishedLesson("mission-1-moveview");
		}

		if (!m_bMouseDragDown && y > DigitanksWindow()->GetWindowHeight()-15 && !DigitanksGame()->IsFeatureDisabled(DISABLE_VIEW_MOVE))
		{
			m_bMouseDragDown = true;
			m_vecGoalVelocity.x = 80.0f;
		}

		if (m_bMouseDragDown && y < DigitanksWindow()->GetWindowHeight()-15)
		{
			m_bMouseDragDown = false;
			m_vecGoalVelocity.x = 0;
			DigitanksWindow()->GetInstructor()->FinishedLesson("mission-1-moveview");
		}
	}
	else
		m_vecGoalVelocity = Vector(0,0,0);
}

bool COverheadCamera::MouseInput(int iButton, tinker_mouse_state_t iState)
{
	if (iButton == TINKER_KEY_MOUSE_RIGHT)
	{
		m_bRotatingCamera = !!iState;
		return true;
	}

	return false;
}

bool COverheadCamera::KeyDown(int c)
{
	if (!DigitanksGame()->IsFeatureDisabled(DISABLE_VIEW_MOVE))
	{
		if (c == TINKER_KEY_UP)
		{
			m_vecGoalVelocity.x = -80.0f;
			return true;
		}

		if (c == TINKER_KEY_DOWN)
		{
			m_vecGoalVelocity.x = 80.0f;
			return true;
		}

		if (c == TINKER_KEY_RIGHT)
		{
			m_vecGoalVelocity.y = 80.0f;
			return true;
		}

		if (c == TINKER_KEY_LEFT)
		{
			m_vecGoalVelocity.y = -80.0f;
			return true;
		}
	}

	if (c == ' ' && !m_hCameraGuidedMissile && !DigitanksGame()->IsFeatureDisabled(DISABLE_VIEW_MOVE))
	{
		m_bDraggingCamera = true;
		return true;
	}

	if (c == TINKER_KEY_LSHIFT)
	{
		m_bFastDraggingCamera = true;
		return true;
	}

	if (c == TINKER_KEY_PAGEUP)
	{
		ZoomIn();
		return true;
	}

	if (c == TINKER_KEY_PAGEDOWN)
	{
		ZoomOut();
		return true;
	}

	return false;
}

bool COverheadCamera::KeyUp(int c)
{
	if (c == TINKER_KEY_UP || c == TINKER_KEY_DOWN)
	{
		m_vecGoalVelocity.x = 0.0f;
		return true;
	}

	if (c == TINKER_KEY_RIGHT || c == TINKER_KEY_LEFT)
	{
		m_vecGoalVelocity.y = 0.0f;
		return true;
	}

	if (c == ' ')
	{
		m_bDraggingCamera = false;
		return true;
	}

	if (c == TINKER_KEY_LSHIFT)
	{
		m_bFastDraggingCamera = false;
		return true;
	}

	return false;
}
