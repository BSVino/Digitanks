#include "dt_camera.h"

#include <maths.h>
#include <mtrand.h>
#include <renderer/renderer.h>

#include "digitanks/digitanksgame.h"

#include <ui/digitankswindow.h>
#include <ui/instructor.h>
#include <tinker/keys.h>
#include <game/digitanks/weapons/cameraguided.h>

CDigitanksCamera::CDigitanksCamera()
{
	m_flTargetRamp = m_flDistanceRamp = m_flAngleRamp = 0;
	m_angCamera = EAngle(45, 0, 0);
	m_bRotatingCamera = false;
	m_flShakeMagnitude = 0;

	m_bMouseDragLeft = m_bMouseDragRight = m_bMouseDragUp = m_bMouseDragDown = false;
}

void CDigitanksCamera::SetTarget(Vector vecTarget)
{
	if (vecTarget.x < -DigitanksGame()->GetTerrain()->GetMapSize())
		vecTarget.x = -DigitanksGame()->GetTerrain()->GetMapSize();
	if (vecTarget.z < -DigitanksGame()->GetTerrain()->GetMapSize())
		vecTarget.z = -DigitanksGame()->GetTerrain()->GetMapSize();
	if (vecTarget.x > DigitanksGame()->GetTerrain()->GetMapSize())
		vecTarget.x = DigitanksGame()->GetTerrain()->GetMapSize();
	if (vecTarget.z > DigitanksGame()->GetTerrain()->GetMapSize())
		vecTarget.z = DigitanksGame()->GetTerrain()->GetMapSize();

	DigitanksGame()->GetTerrain()->SetPointHeight(vecTarget);

	m_flTargetRamp = GameServer()->GetGameTime();
	m_vecOldTarget = m_vecTarget;
	m_vecNewTarget = vecTarget;
	m_vecVelocity = m_vecGoalVelocity = Vector(0,0,0);
}

void CDigitanksCamera::SnapTarget(Vector vecTarget)
{
	DigitanksGame()->GetTerrain()->SetPointHeight(vecTarget);

	m_flTargetRamp = 0;
	m_vecNewTarget = m_vecTarget = vecTarget;
	m_vecVelocity = m_vecGoalVelocity = Vector(0,0,0);
}

void CDigitanksCamera::SetDistance(float flDistance)
{
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

	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_ZOOMCAMERA);
}

void CDigitanksCamera::ZoomIn()
{
	SetDistance(m_flNewDistance-20);

	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_ZOOMCAMERA);
}

void CDigitanksCamera::Shake(Vector vecLocation, float flMagnitude)
{
	m_vecShakeLocation = vecLocation;
	m_flShakeMagnitude = flMagnitude;
}

void CDigitanksCamera::SetCameraGuidedMissile(CCameraGuidedMissile* pMissile)
{
	m_hCameraGuidedMissile = pMissile;
}

void CDigitanksCamera::Think()
{
	BaseClass::Think();

	float flGameTime = GameServer()->GetGameTime();
	float flFrameTime = GameServer()->GetFrameTime();
	float flLerpTime = 0.5f;
	float flLerpAmount = 0.7f;

	if (!DigitanksWindow()->ShouldConstrainMouse())
		m_vecGoalVelocity = Vector(0,0,0);

	m_vecVelocity.x = Approach(m_vecGoalVelocity.x, m_vecVelocity.x, flFrameTime*200);
	m_vecVelocity.z = Approach(m_vecGoalVelocity.z, m_vecVelocity.z, flFrameTime*200);

	if (m_flTargetRamp && flGameTime - m_flTargetRamp < flLerpTime)
	{
		float flLerp = Lerp((flGameTime - m_flTargetRamp)/flLerpTime, flLerpAmount);
		m_vecTarget = m_vecNewTarget * flLerp + m_vecOldTarget * (1-flLerp);
		m_vecVelocity = m_vecGoalVelocity = Vector(0,0,0);
	}
	else
	{
		if (m_flTargetRamp)
		{
			m_vecTarget = m_vecNewTarget;
			m_flTargetRamp = 0;
		}
		else
		{
			Matrix4x4 mRotation;
			mRotation.SetRotation(EAngle(0, m_angCamera.y, 0));
			Vector vecVelocity = mRotation * m_vecVelocity;

			m_vecTarget = m_vecTarget + vecVelocity * flFrameTime;

			if (m_vecTarget.x < -DigitanksGame()->GetTerrain()->GetMapSize())
				m_vecTarget.x = -DigitanksGame()->GetTerrain()->GetMapSize();
			if (m_vecTarget.z < -DigitanksGame()->GetTerrain()->GetMapSize())
				m_vecTarget.z = -DigitanksGame()->GetTerrain()->GetMapSize();
			if (m_vecTarget.x > DigitanksGame()->GetTerrain()->GetMapSize())
				m_vecTarget.x = DigitanksGame()->GetTerrain()->GetMapSize();
			if (m_vecTarget.z > DigitanksGame()->GetTerrain()->GetMapSize())
				m_vecTarget.z = DigitanksGame()->GetTerrain()->GetMapSize();

			DigitanksGame()->GetTerrain()->SetPointHeight(m_vecTarget);
		}
	}

	if (m_flDistanceRamp && flGameTime - m_flDistanceRamp < flLerpTime)
	{
		float flLerp = Lerp((flGameTime - m_flDistanceRamp)/flLerpTime, flLerpAmount);
		m_flDistance = m_flNewDistance * flLerp + m_flOldDistance * (1-flLerp);
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
	if (m_hCameraGuidedMissile != NULL)
		return m_hCameraGuidedMissile->GetOrigin();

	if (m_bFreeMode)
		return BaseClass::GetCameraPosition();

	return m_vecCamera + m_vecShake;
}

Vector CDigitanksCamera::GetCameraTarget()
{
	if (m_hCameraGuidedMissile != NULL)
		return m_hCameraGuidedMissile->GetOrigin() + AngleVector(m_hCameraGuidedMissile->GetAngles());

	if (m_bFreeMode)
		return BaseClass::GetCameraTarget();

	return m_vecTarget + m_vecShake;
}

float CDigitanksCamera::GetCameraFOV()
{
	if (m_hCameraGuidedMissile != NULL)
		return 100;

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

void CDigitanksCamera::SetFreeMode(bool bOn)
{
	if (!DigitanksGame()->AllowCheats())
	{
		BaseClass::SetFreeMode(false);
		return;
	}

	BaseClass::SetFreeMode(bOn);
}

void CDigitanksCamera::MouseInput(int x, int y)
{
	int dx, dy;

	dx = x - m_iMouseLastX;
	dy = y - m_iMouseLastY;

	if (m_hCameraGuidedMissile != NULL)
	{
		EAngle angMissile = m_hCameraGuidedMissile->GetAngles();
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
	else if (DigitanksWindow()->ShouldConstrainMouse() && !m_bFreeMode)
	{
		if (!m_bMouseDragLeft && x < 15)
		{
			m_bMouseDragLeft = true;
			m_vecGoalVelocity.z = 80.0f;
		}

		if (m_bMouseDragLeft && x > 15)
		{
			m_bMouseDragLeft = false;
			m_vecGoalVelocity.z = 0;
			DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_MOVECAMERA);
		}

		if (!m_bMouseDragUp && y < 15)
		{
			m_bMouseDragUp = true;
			m_vecGoalVelocity.x = -80.0f;
		}

		if (m_bMouseDragUp && y > 15)
		{
			m_bMouseDragUp = false;
			m_vecGoalVelocity.x = 0;
			DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_MOVECAMERA);
		}

		if (!m_bMouseDragRight && x > DigitanksWindow()->GetWindowWidth()-15)
		{
			m_bMouseDragRight = true;
			m_vecGoalVelocity.z = -80.0f;
		}

		if (m_bMouseDragRight && x < DigitanksWindow()->GetWindowWidth()-15)
		{
			m_bMouseDragRight = false;
			m_vecGoalVelocity.z = 0;
			DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_MOVECAMERA);
		}

		if (!m_bMouseDragDown && y > DigitanksWindow()->GetWindowHeight()-15)
		{
			m_bMouseDragDown = true;
			m_vecGoalVelocity.x = 80.0f;
		}

		if (m_bMouseDragDown && y < DigitanksWindow()->GetWindowHeight()-15)
		{
			m_bMouseDragDown = false;
			m_vecGoalVelocity.x = 0;
			DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_MOVECAMERA);
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
	if (!m_bFreeMode)
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

	BaseClass::KeyUp(c);
}
