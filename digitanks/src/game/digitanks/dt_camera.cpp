#include "dt_camera.h"

#include <maths.h>
#include <mtrand.h>
#include <renderer/renderer.h>
#include <tinker/cvar.h>
#include <tinker/keys.h>
#include <network/commands.h>

#include "digitanks/digitanksgame.h"

#include <ui/digitankswindow.h>
#include <ui/instructor.h>
#include <game/digitanks/weapons/cameraguided.h>

CDigitanksCamera::CDigitanksCamera()
{
	m_flTargetRamp = m_flDistanceRamp = m_flAngleRamp = 0;
	m_angCamera = EAngle(45, 0, 0);
	m_bRotatingCamera = false;
	m_bDraggingCamera = false;
	m_bFastDraggingCamera = false;
	m_flShakeMagnitude = 0;

	m_bMouseDragLeft = m_bMouseDragRight = m_bMouseDragUp = m_bMouseDragDown = false;

	m_flTransitionToProjectileTime = 0;
}

void CDigitanksCamera::SetTarget(Vector vecTarget)
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

void CDigitanksCamera::SnapTarget(Vector vecTarget)
{
	if (!DigitanksGame()->GetTerrain())
		return;

	vecTarget = DigitanksGame()->GetTerrain()->GetPointHeight(vecTarget);

	m_flTargetRamp = 0;
	m_vecNewTarget = m_vecTarget = vecTarget;
	m_vecVelocity = m_vecGoalVelocity = Vector(0,0,0);
}

void CDigitanksCamera::SetDistance(float flDistance)
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

void CDigitanksCamera::SnapDistance(float flDistance)
{
	m_flDistanceRamp = 0;
	m_flNewDistance = flDistance;
}

void CDigitanksCamera::SetAngle(EAngle angCamera)
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

void CDigitanksCamera::SnapAngle(EAngle angCamera)
{
	m_angCamera = angCamera;
}

void CDigitanksCamera::ZoomOut()
{
	SetDistance(m_flNewDistance+20);

	DigitanksWindow()->GetInstructor()->FinishedTutorial("mission-1-zoomview");
}

void CDigitanksCamera::ZoomIn()
{
	SetDistance(m_flNewDistance-20);

	DigitanksWindow()->GetInstructor()->FinishedTutorial("mission-1-zoomview");
}

void CDigitanksCamera::Shake(Vector vecLocation, float flMagnitude)
{
	m_vecShakeLocation = vecLocation;
	m_flShakeMagnitude = flMagnitude;
}

void CDigitanksCamera::SetCameraGuidedMissile(CCameraGuidedMissile* pMissile)
{
	m_hCameraGuidedMissile = pMissile;
	m_flCameraGuidedFOV = m_flCameraGuidedFOVGoal = 0;
}

CCameraGuidedMissile* CDigitanksCamera::GetCameraGuidedMissile()
{
	return m_hCameraGuidedMissile;
}

bool CDigitanksCamera::HasCameraGuidedMissile()
{
	return m_hCameraGuidedMissile != NULL;
}

void CDigitanksCamera::ShowEnemyMoves()
{
	CDigitanksTeam* pCurrentTeam = DigitanksGame()->GetCurrentTeam();

	if (!pCurrentTeam)
		return;

	if (pCurrentTeam->GetNumTanks() == 0)
		return;

	eastl::vector<const CDigitank*> apTargets;

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

		if (pClosestTank && pClosestTank->GetTeam())
		{
			if (pClosestTank->GetTeam()->IsPlayerControlled())
			{
				apTargets.push_back(pTank);
				vecAveragePosition += pTank->GetOrigin();
			}

			continue;
		}

		if (pClosestStructure && pClosestStructure->GetTeam() && pClosestStructure->GetTeam()->IsPlayerControlled())
		{
			apTargets.push_back(pTank);
			vecAveragePosition += pTank->GetOrigin();
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
		if ((apTargets[i]->GetOrigin() - vecAveragePosition).LengthSqr() < (pClosestTarget->GetOrigin() - vecAveragePosition).LengthSqr())
			pClosestTarget = apTargets[i];
	}

	// Find nearby tanks to that closest tank
	Vector vecAverageNearbyTanks = Vector(0,0,0);
	int iNearbyTargets = 0;
	for (size_t i = 0; i < apTargets.size(); i++)
	{
		if ((apTargets[i]->GetOrigin() - vecAveragePosition).LengthSqr() < 100*100)
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

void CDigitanksCamera::ProjectileFired(CProjectile* pProjectile)
{
	if (m_hTankTarget == NULL)
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

	if (pProjectile->GetOwner()->GetDigitanksTeam() && pProjectile->GetOwner()->GetDigitanksTeam()->GetVisibilityAtPoint(pOwner->GetLastAim()) < 0.8f)
		return;

	m_hTankProjectile = pProjectile;
	m_hTankTarget = NULL;
	m_flTransitionToProjectileTime = GameServer()->GetGameTime();
}

void CDigitanksCamera::ReplaceProjectileTarget(CProjectile* pTarget)
{
	if (m_hTankProjectile != NULL)
		m_hTankProjectile = pTarget;
}

void CDigitanksCamera::ClearFollowTarget()
{
	m_hTankTarget = NULL;
	m_hTankProjectile = NULL;
}

void CDigitanksCamera::EnterGame()
{
	m_flTimeSinceNewGame = GameServer()->GetGameTime();
}

void CDigitanksCamera::Think()
{
	BaseClass::Think();

	float flGameTime = GameServer()->GetGameTime();
	float flFrameTime = GameServer()->GetFrameTime();
	float flLerpTime = 0.5f;
	float flLerpAmount = 0.7f;

	if (m_hCameraGuidedMissile != NULL)
	{
		m_flCameraGuidedFOVGoal = m_hCameraGuidedMissile->IsBoosting()?1.0f:0.0f;
		if (m_hCameraGuidedMissile->IsBoosting())
			m_flCameraGuidedFOV = Approach(m_flCameraGuidedFOVGoal, m_flCameraGuidedFOV, 10 * GameServer()->GetFrameTime());
		else
			m_flCameraGuidedFOV = Approach(m_flCameraGuidedFOVGoal, m_flCameraGuidedFOV, 1 * GameServer()->GetFrameTime());
	}

	if (m_hTankProjectile != NULL)
	{
		if (m_hTankProjectile->HasExploded())
			m_hTankProjectile = NULL;
	}

	m_vecVelocity.x = Approach(m_vecGoalVelocity.x, m_vecVelocity.x, flFrameTime*200);
	m_vecVelocity.z = Approach(m_vecGoalVelocity.z, m_vecVelocity.z, flFrameTime*200);

	if (m_flTargetRamp && flGameTime - m_flTargetRamp < flLerpTime)
	{
		float flLerp = Lerp((flGameTime - m_flTargetRamp)/flLerpTime, flLerpAmount);
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
			mRotation.SetRotation(EAngle(0, m_angCamera.y, 0));

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
		float flLerp = Lerp((flGameTime - m_flDistanceRamp)/flLerpTime, flLerpAmount);
		m_flDistance = LerpValue<float>(m_flOldDistance, m_flNewDistance, flLerp);
	}
	else
		m_flDistance = m_flNewDistance;

	if (m_flAngleRamp && flGameTime - m_flAngleRamp < flLerpTime)
	{
		float flLerp = Lerp((flGameTime - m_flAngleRamp)/flLerpTime, flLerpAmount);
		float flPitch = m_angOldAngle.p + AngleDifference(m_angNewAngle.p, m_angOldAngle.p) * flLerp;
		float flYaw = m_angOldAngle.y + AngleDifference(m_angNewAngle.y, m_angOldAngle.y) * flLerp;
		float flRoll = m_angOldAngle.r + AngleDifference(m_angNewAngle.r, m_angOldAngle.r) * flLerp;
		m_angCamera = EAngle(flPitch, flYaw, flRoll);
	}

	m_vecCamera = AngleVector(m_angCamera) * m_flDistance + m_vecTarget;

	m_flShakeMagnitude = Approach(0, m_flShakeMagnitude, GameServer()->GetFrameTime()*5);
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
			flScale = RemapValClamped(flDistance, 50, 200, 1, 0);
		m_vecShake *= flScale;
	}
	else
		m_vecShake = Vector(0,0,0);
}

Vector CDigitanksCamera::GetCameraPosition()
{
	if (m_hTankTarget != NULL)
		return GetTankFollowPosition(m_hTankTarget);

	if (m_hTankProjectile != NULL)
	{
		float flLerp = RemapValClamped(GameServer()->GetGameTime(), m_flTransitionToProjectileTime, m_flTransitionToProjectileTime + 0.5f, 0, 1);

		if (m_hTankProjectile->GetWeaponType() == PROJECTILE_TORPEDO)
		{
			Vector vecDirection = m_hTankProjectile->GetLandingSpot() - m_hTankProjectile->GetOrigin();
			vecDirection.y = 0;
			vecDirection.Normalize();

			Vector vecTorpedoFollow = m_hTankProjectile->GetOrigin() - vecDirection*15 + Vector(0, 15, 0);

			if (flLerp < 1 && m_hTankProjectile->GetOwner())
				return LerpValue<Vector>(GetTankFollowPosition(dynamic_cast<CDigitank*>(m_hTankProjectile->GetOwner())), vecTorpedoFollow, flLerp);

			return vecTorpedoFollow;
		}

		Vector vecVelocity = m_hTankProjectile->GetVelocity();

		Vector vecForward, vecRight, vecUp;
		AngleVectors(VectorAngles(vecVelocity), &vecForward, &vecRight, &vecUp);

		Vector vecProjectileFollow = m_hTankProjectile->GetOrigin() - vecForward*13 - vecRight*4;

		if (flLerp < 1 && m_hTankProjectile->GetOwner())
			return LerpValue<Vector>(GetTankFollowPosition(dynamic_cast<CDigitank*>(m_hTankProjectile->GetOwner())), vecProjectileFollow, flLerp);

		return vecProjectileFollow;
	}

	if (m_hCameraGuidedMissile != NULL)
		return m_hCameraGuidedMissile->GetOrigin();

	if (m_bFreeMode)
		return BaseClass::GetCameraPosition();

	if (GameServer()->GetGameTime() - m_flTimeSinceNewGame < 3 && DigitanksGame()->GetGameType() != GAMETYPE_MENU)
	{
		float flCameraIntroLerp = SLerp(RemapVal(GameServer()->GetGameTime() - m_flTimeSinceNewGame, 0, 3, 0, 1), 0.2f);

		EAngle angIntro = m_angCamera;
		angIntro.y += 90;

		Vector vecIntroTarget = m_vecTarget;
		Vector vecIntroCamera = AngleVector(angIntro) * 400 + vecIntroTarget;

		return LerpValue<Vector>(vecIntroCamera, m_vecCamera, flCameraIntroLerp) + m_vecShake;
	}

	return m_vecCamera + m_vecShake;
}

Vector CDigitanksCamera::GetCameraTarget()
{
	if (m_hTankTarget != NULL)
	{
		if (m_hTankTarget->GetCurrentWeapon() == WEAPON_CHARGERAM)
			return m_hTankTarget->GetOrigin() + AngleVector(m_hTankTarget->GetAngles()) * 10;

		return m_hTankTarget->GetLastAim();
	}

	if (m_hTankProjectile != NULL)
	{
		float flLerp = RemapValClamped(GameServer()->GetGameTime(), m_flTransitionToProjectileTime, m_flTransitionToProjectileTime + 0.5f, 0, 1);

		CDigitank* pTank = dynamic_cast<CDigitank*>(m_hTankProjectile->GetOwner());
		TAssert(pTank);
		if (!pTank)
			return Vector();

		if (m_hTankProjectile->GetWeaponType() == PROJECTILE_TORPEDO)
		{
			Vector vecTorpedoTarget = m_hTankProjectile->GetOrigin();

			if (flLerp < 1 && m_hTankProjectile->GetOwner())
				return LerpValue<Vector>(pTank->GetLastAim(), vecTorpedoTarget, flLerp);

			return vecTorpedoTarget;
		}

		Vector vecProjectileTarget = m_hTankProjectile->GetOrigin() + m_hTankProjectile->GetVelocity();

		if (flLerp < 1 && m_hTankProjectile->GetOwner())
			return LerpValue<Vector>(pTank->GetLastAim(), vecProjectileTarget, flLerp);

		return vecProjectileTarget;
	}

	if (m_hCameraGuidedMissile != NULL)
		return m_hCameraGuidedMissile->GetOrigin() + AngleVector(m_hCameraGuidedMissile->GetViewAngles());

	if (m_bFreeMode)
		return BaseClass::GetCameraTarget();

	if (GameServer()->GetGameTime() - m_flTimeSinceNewGame < 3 && DigitanksGame()->GetGameType() != GAMETYPE_MENU)
	{
		float flCameraIntroLerp = SLerp(RemapVal(GameServer()->GetGameTime() - m_flTimeSinceNewGame, 0, 3, 0, 1), 0.2f);

		EAngle angIntro = m_angCamera;
		angIntro.y += 90;

		Vector vecIntroTarget = m_vecTarget;
		Vector vecIntroCamera = AngleVector(angIntro) * 400 + vecIntroTarget;

		return LerpValue<Vector>(vecIntroTarget, m_vecTarget, flCameraIntroLerp) + m_vecShake;
	}

	return m_vecTarget + m_vecShake;
}

CVar cam_cg_fov("cam_cg_fov", "80");
CVar cam_cg_boost_fov("cam_cg_boost_fov", "70");

float CDigitanksCamera::GetCameraFOV()
{
	if (m_hCameraGuidedMissile != NULL)
		return RemapVal(m_flCameraGuidedFOV, 0, 1, cam_cg_fov.GetFloat(), cam_cg_boost_fov.GetFloat());

	return BaseClass::GetCameraFOV();
}

float CDigitanksCamera::GetCameraNear()
{
	if (m_hCameraGuidedMissile != NULL)
		return 1.0f;

	return 10;
}

float CDigitanksCamera::GetCameraFar()
{
	if (m_hCameraGuidedMissile != NULL)
		return 1000;

	return 1000;
}

Vector CDigitanksCamera::GetTankFollowPosition(CDigitank* pTank)
{
	TAssert(pTank);
	if (!pTank)
		return Vector();

	Vector vecTarget = pTank->GetLastAim();

	Vector vecForward, vecRight, vecUp;
	AngleVectors(VectorAngles(vecTarget - pTank->GetOrigin()), &vecForward, &vecRight, &vecUp);

	return pTank->GetOrigin() - vecForward*20 - vecRight*6 - vecUp*5;
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

	if (!hMissile->GetOwner()->GetTeam())
	{
		TMsg("CGAng with no team.\n");
		return;
	}

	if (hMissile->GetOwner()->GetTeam()->GetClient() != (int)iClient)
	{
		TMsg("CGAng missile does not belong to this client.\n");
		return;
	}

	EAngle angMissile(pCmd->ArgAsFloat(1), pCmd->ArgAsFloat(2), pCmd->ArgAsFloat(3));
	hMissile->SetViewAngles(angMissile);
	hMissile->SetAngles(angMissile);
}

void CDigitanksCamera::MouseInput(int x, int y)
{
	int dx, dy;

	dx = x - m_iMouseLastX;
	dy = y - m_iMouseLastY;

	if (m_hCameraGuidedMissile != NULL)
	{
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
			CGAng.RunCommand(sprintf(tstring("%d %f %f %f"), m_hCameraGuidedMissile->GetHandle(), angMissile.p, angMissile.y, angMissile.r));

		m_hCameraGuidedMissile->SetViewAngles(angMissile);
		m_hCameraGuidedMissile->SetAngles(angMissile);
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
		mRotation.SetRotation(EAngle(0, m_angCamera.y, 0));

		float flStrength = 2;
		if (m_bFastDraggingCamera)
			flStrength = 4;

		flStrength *= DigitanksWindow()->ShouldReverseSpacebar()?-1:1;

		Vector vecVelocity = mRotation * Vector((float)dy*flStrength, 0, (float)-dx*flStrength);

		SetTarget(m_vecTarget + vecVelocity);

		DigitanksWindow()->GetInstructor()->FinishedTutorial("mission-1-moveview2");
	}
	else if (DigitanksWindow()->ShouldConstrainMouse() && !m_bFreeMode && !m_bDraggingCamera)
	{
		CInstructor* pInstructor = DigitanksWindow()->GetInstructor();
		if (!m_bMouseDragLeft && x < 15 && !pInstructor->IsFeatureDisabled(DISABLE_VIEW_MOVE))
		{
			m_bMouseDragLeft = true;
			m_vecGoalVelocity.z = 80.0f;
		}

		if (m_bMouseDragLeft && x > 15)
		{
			m_bMouseDragLeft = false;
			m_vecGoalVelocity.z = 0;
			DigitanksWindow()->GetInstructor()->FinishedTutorial("mission-1-moveview");
		}

		if (!m_bMouseDragUp && y < 15 && !pInstructor->IsFeatureDisabled(DISABLE_VIEW_MOVE))
		{
			m_bMouseDragUp = true;
			m_vecGoalVelocity.x = -80.0f;
		}

		if (m_bMouseDragUp && y > 15)
		{
			m_bMouseDragUp = false;
			m_vecGoalVelocity.x = 0;
			DigitanksWindow()->GetInstructor()->FinishedTutorial("mission-1-moveview");
		}

		if (!m_bMouseDragRight && x > DigitanksWindow()->GetWindowWidth()-15 && !pInstructor->IsFeatureDisabled(DISABLE_VIEW_MOVE))
		{
			m_bMouseDragRight = true;
			m_vecGoalVelocity.z = -80.0f;
		}

		if (m_bMouseDragRight && x < DigitanksWindow()->GetWindowWidth()-15)
		{
			m_bMouseDragRight = false;
			m_vecGoalVelocity.z = 0;
			DigitanksWindow()->GetInstructor()->FinishedTutorial("mission-1-moveview");
		}

		if (!m_bMouseDragDown && y > DigitanksWindow()->GetWindowHeight()-15 && !pInstructor->IsFeatureDisabled(DISABLE_VIEW_MOVE))
		{
			m_bMouseDragDown = true;
			m_vecGoalVelocity.x = 80.0f;
		}

		if (m_bMouseDragDown && y < DigitanksWindow()->GetWindowHeight()-15)
		{
			m_bMouseDragDown = false;
			m_vecGoalVelocity.x = 0;
			DigitanksWindow()->GetInstructor()->FinishedTutorial("mission-1-moveview");
		}
	}
	else
		m_vecGoalVelocity = Vector(0,0,0);

	BaseClass::MouseInput(x, y);
}

void CDigitanksCamera::MouseButton(int iButton, int iState)
{
	if (iButton == TINKER_KEY_MOUSE_RIGHT)
	{
		m_bRotatingCamera = !!iState;
	}

	BaseClass::MouseButton(iButton, iState);
}

void CDigitanksCamera::KeyDown(int c)
{
	if (!m_bFreeMode && !DigitanksWindow()->GetInstructor()->IsFeatureDisabled(DISABLE_VIEW_MOVE))
	{
		if (c == TINKER_KEY_UP)
			m_vecGoalVelocity.x = -80.0f;
		if (c == TINKER_KEY_DOWN)
			m_vecGoalVelocity.x = 80.0f;
		if (c == TINKER_KEY_RIGHT)
			m_vecGoalVelocity.z = -80.0f;
		if (c == TINKER_KEY_LEFT)
			m_vecGoalVelocity.z = 80.0f;
	}

	if (c == ' ' && m_hCameraGuidedMissile == NULL && !DigitanksWindow()->GetInstructor()->IsFeatureDisabled(DISABLE_VIEW_MOVE))
		m_bDraggingCamera = true;

	if (c == TINKER_KEY_LSHIFT)
		m_bFastDraggingCamera = true;

	if (c == TINKER_KEY_PAGEUP)
		ZoomIn();
	if (c == TINKER_KEY_PAGEDOWN)
		ZoomOut();

	BaseClass::KeyDown(c);
}

void CDigitanksCamera::KeyUp(int c)
{
	if (!m_bFreeMode)
	{
		if (c == TINKER_KEY_UP)
			m_vecGoalVelocity.x = 0.0f;
		if (c == TINKER_KEY_DOWN)
			m_vecGoalVelocity.x = 0.0f;
		if (c == TINKER_KEY_RIGHT)
			m_vecGoalVelocity.z = 0.0f;
		if (c == TINKER_KEY_LEFT)
			m_vecGoalVelocity.z = 0.0f;
	}

	if (c == ' ')
		m_bDraggingCamera = false;

	if (c == TINKER_KEY_LSHIFT)
		m_bFastDraggingCamera = false;

	BaseClass::KeyUp(c);
}
