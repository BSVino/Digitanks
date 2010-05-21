#include "camera.h"

#include <maths.h>

#include "game/digitanksgame.h"

CCamera::CCamera()
{
	m_bFPSMode = false;
	m_flTargetRamp = m_flTargetRamp = 0;
	m_angCamera = EAngle(45, 0, 0);
	m_bRotatingCamera = false;
}

void CCamera::SetTarget(Vector vecTarget)
{
	m_flTargetRamp = DigitanksGame()->GetGameTime();
	m_vecOldTarget = m_vecTarget;
	m_vecNewTarget = vecTarget;
}

void CCamera::SetDistance(float flDistance)
{
	m_flDistanceRamp = DigitanksGame()->GetGameTime();
	m_flOldDistance = m_flDistance;
	m_flNewDistance = flDistance;
}

void CCamera::Think()
{
	float flGameTime = DigitanksGame()->GetGameTime();
	float flLerpTime = 0.5f;
	float flLerpAmount = 0.7f;

	if (flGameTime - m_flTargetRamp < flLerpTime)
	{
		float flLerp = Lerp((flGameTime - m_flTargetRamp)/flLerpTime, flLerpAmount);
		m_vecTarget = m_vecNewTarget * flLerp + m_vecOldTarget * (1-flLerp);
	}
	else
		m_vecTarget = m_vecNewTarget;

	if (flGameTime - m_flDistanceRamp < flLerpTime)
	{
		float flLerp = Lerp((flGameTime - m_flDistanceRamp)/flLerpTime, flLerpAmount);
		m_flDistance = m_flNewDistance * flLerp + m_flOldDistance * (1-flLerp);
	}
	else
		m_flDistance = m_flNewDistance;

	m_vecCamera = AngleVector(m_angCamera) * m_flDistance + m_vecTarget;

	if (m_bFPSMode)
	{
		Vector vecForward, vecRight;
		AngleVectors(m_angFPSCamera, &vecForward, &vecRight, NULL);

		m_vecFPSCamera += vecForward * m_vecFPSVelocity.x * DigitanksGame()->GetFrameTime() * 20;
		m_vecFPSCamera -= vecRight * m_vecFPSVelocity.z * DigitanksGame()->GetFrameTime() * 20;
	}
}

Vector CCamera::GetCameraPosition()
{
	if (m_bFPSMode)
		return m_vecFPSCamera;

	return m_vecCamera;
}

Vector CCamera::GetCameraTarget()
{
	if (m_bFPSMode)
		return m_vecFPSCamera + AngleVector(m_angFPSCamera);

	return m_vecTarget;
}

void CCamera::SetFPSMode(bool bOn)
{
	m_vecFPSCamera = m_vecCamera;
	m_angFPSCamera = VectorAngles((m_vecTarget - m_vecCamera).Normalized());
	m_bFPSMode = bOn;
}

void CCamera::MouseInput(int x, int y)
{
	if (m_bRotatingCamera)
	{
		m_angCamera.y += (x/5.0f);
		m_angCamera.p += (y/5.0f);

		if (m_angCamera.p > 89)
			m_angCamera.p = 89;

		if (m_angCamera.p < 20)
			m_angCamera.p = 20;

		while (m_angCamera.y > 180)
			m_angCamera.y -= 360;

		while (m_angCamera.y < -180)
			m_angCamera.y += 360;
	}

	if (m_bFPSMode)
	{
		m_angFPSCamera.y += (x/5.0f);
		m_angFPSCamera.p -= (y/5.0f);

		if (m_angFPSCamera.p > 89)
			m_angFPSCamera.p = 89;

		if (m_angFPSCamera.p < -89)
			m_angFPSCamera.p = -89;

		while (m_angFPSCamera.y > 180)
			m_angFPSCamera.y -= 360;

		while (m_angFPSCamera.y < -180)
			m_angFPSCamera.y += 360;
	}
}

void CCamera::MouseButton(int iButton, int iState)
{
	if (iButton == 2)
		m_bRotatingCamera = !iState;
}

void CCamera::KeyDown(int c)
{
#ifdef _DEBUG
	if (c == 'z')
	{
		SetFPSMode(!m_bFPSMode);
		//glutSetCursor(m_bFPSMode?GLUT_CURSOR_NONE:GLUT_CURSOR_LEFT_ARROW);
	}
#endif

	if (m_bFPSMode)
	{
		if (c == 'w')
			m_vecFPSVelocity.x = 10.0f;
		if (c == 's')
			m_vecFPSVelocity.x = -10.0f;
		if (c == 'd')
			m_vecFPSVelocity.z = 10.0f;
		if (c == 'a')
			m_vecFPSVelocity.z = -10.0f;
	}
}

void CCamera::KeyUp(int c)
{
	if (m_bFPSMode)
	{
		if (c == 'w')
			m_vecFPSVelocity.x = 0.0f;
		if (c == 's')
			m_vecFPSVelocity.x = 0.0f;
		if (c == 'd')
			m_vecFPSVelocity.z = 0.0f;
		if (c == 'a')
			m_vecFPSVelocity.z = 0.0f;
	}
}
