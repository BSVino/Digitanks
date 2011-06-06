#include "camera.h"

#include <maths.h>
#include <mtrand.h>
#include <renderer/renderer.h>
#include <tinker/cvar.h>
#include <tinker/application.h>
#include <game/gameserver.h>

CCamera::CCamera()
{
	m_bFreeMode = false;

	m_iMouseLastX = 0;
	m_iMouseLastY = 0;
}

CVar shrink_frustum("debug_shrink_frustum", "no");

CVar cam_free("cam_free", "off");

void CCamera::Think()
{
	bool bFreeMode = cam_free.GetBool();
	if (bFreeMode != m_bFreeMode)
	{
		m_vecFreeCamera = GetCameraPosition();
		m_angFreeCamera = VectorAngles((GetCameraTarget() - GetCameraPosition()).Normalized());
		m_bFreeMode = bFreeMode;
		CApplication::Get()->SetMouseCursorEnabled(!m_bFreeMode);
	}

	if (m_bFreeMode)
	{
		Vector vecForward, vecRight;
		AngleVectors(m_angFreeCamera, &vecForward, &vecRight, NULL);

		m_vecFreeCamera += vecForward * m_vecFreeVelocity.x * GameServer()->GetFrameTime() * 20;
		m_vecFreeCamera -= vecRight * m_vecFreeVelocity.z * GameServer()->GetFrameTime() * 20;
	}
	else
	{
		if (shrink_frustum.GetBool())
			GameServer()->GetRenderer()->FrustumOverride(GetCameraPosition(), GetCameraTarget(), GetCameraFOV()-1, GetCameraNear()+1, GetCameraFar()-1);
	}
}

Vector CCamera::GetCameraPosition()
{
	if (m_bFreeMode)
		return m_vecFreeCamera;

	return Vector(30, 30, 30);
}

Vector CCamera::GetCameraTarget()
{
	if (m_bFreeMode)
		return m_vecFreeCamera + AngleVector(m_angFreeCamera);

	return Vector(0,0,0);
}

float CCamera::GetCameraFOV()
{
	return 44.0f;
}

void CCamera::MouseInput(int x, int y)
{
	int dx, dy;

	dx = x - m_iMouseLastX;
	dy = y - m_iMouseLastY;

	if (m_bFreeMode)
	{
		m_angFreeCamera.y += (dx/5.0f);
		m_angFreeCamera.p -= (dy/5.0f);

		if (m_angFreeCamera.p > 89)
			m_angFreeCamera.p = 89;

		if (m_angFreeCamera.p < -89)
			m_angFreeCamera.p = -89;

		while (m_angFreeCamera.y > 180)
			m_angFreeCamera.y -= 360;

		while (m_angFreeCamera.y < -180)
			m_angFreeCamera.y += 360;
	}

	m_iMouseLastX = x;
	m_iMouseLastY = y;
}

CVar lock_freemode_frustum("debug_lock_freemode_frustum", "no");

void CCamera::KeyDown(int c)
{
	if (CVar::GetCVarBool("cheats") && c == 'Z')
	{
		cam_free.SetValue(m_bFreeMode?L"off":L"on");

		if (lock_freemode_frustum.GetBool())
		{
			if (m_bFreeMode)
				GameServer()->GetRenderer()->FrustumOverride(GetCameraPosition(), GetCameraTarget(), GetCameraFOV(), GetCameraNear(), GetCameraFar());
			else
				GameServer()->GetRenderer()->CancelFrustumOverride();
		}
	}

	if (m_bFreeMode)
	{
		if (c == 'W')
			m_vecFreeVelocity.x = 10.0f;
		if (c == 'S')
			m_vecFreeVelocity.x = -10.0f;
		if (c == 'D')
			m_vecFreeVelocity.z = 10.0f;
		if (c == 'A')
			m_vecFreeVelocity.z = -10.0f;
	}
}

void CCamera::KeyUp(int c)
{
	if (m_bFreeMode)
	{
		if (c == 'W')
			m_vecFreeVelocity.x = 0.0f;
		if (c == 'S')
			m_vecFreeVelocity.x = 0.0f;
		if (c == 'D')
			m_vecFreeVelocity.z = 0.0f;
		if (c == 'A')
			m_vecFreeVelocity.z = 0.0f;
	}
}
