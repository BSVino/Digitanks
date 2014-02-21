#include "cameramanager.h"

#include <maths.h>
#include <mtrand.h>
#include <renderer/game_renderer.h>
#include <tinker/cvar.h>
#include <tinker/application.h>
#include <tinker/keys.h>
#include <game/gameserver.h>
#include <game/entities/camera.h>

CCameraManager::CCameraManager()
{
	m_iCurrentCamera = 0;
	m_iLastCamera = 0;

	m_bFreeMode = false;
	m_bPermaFreeMode = false;

	m_iMouseLastX = 0;
	m_iMouseLastY = 0;

	m_flTransitionBegin = 0;
}

CVar shrink_frustum("debug_shrink_frustum", "no");

CVar cam_free("cam_free", "off");

void CCameraManager::Think()
{
	bool bFreeMode = cam_free.GetBool();
	if (bFreeMode != m_bFreeMode)
	{
		m_vecFreeCamera = GetCameraPosition();
		m_angFreeCamera = VectorAngles((GetCameraDirection()).Normalized());
		m_flFreeOrthoHeight = GetCameraOrthoHeight();
		m_bFreeMode = bFreeMode;
		CApplication::Get()->SetMouseCursorEnabled(!m_bFreeMode);
	}

	if (GetFreeMode())
	{
		Vector vecForward, vecLeft, vecUp;
		AngleVectors(m_angFreeCamera, &vecForward, &vecLeft, &vecUp);

		float flScaleSpeed = 20;
		if (ShouldRenderOrthographic())
			flScaleSpeed = m_flFreeOrthoHeight;

		m_vecFreeCamera += vecForward * m_vecFreeVelocity.x * (float)GameServer()->GetFrameTime() * flScaleSpeed;
		m_vecFreeCamera += vecLeft * m_vecFreeVelocity.y * (float)GameServer()->GetFrameTime() * flScaleSpeed;
		m_vecFreeCamera += vecUp * m_vecFreeVelocity.z * (float)GameServer()->GetFrameTime() * flScaleSpeed;

		m_flFreeOrthoHeight -= ((float)GameServer()->GetFrameTime() * (float)m_vecFreeVelocity.x * 5);
		if (m_flFreeOrthoHeight < 1)
			m_flFreeOrthoHeight = 1;
	}
	else
	{
		if (shrink_frustum.GetBool())
			GameServer()->GetRenderer()->FrustumOverride(GetCameraPosition(), GetCameraDirection(), GetCameraFOV()-1, GetCameraNear()+1, GetCameraFar()-1);
	}

	for (size_t i = 0; i < m_ahCameras.size(); i++)
		m_ahCameras[i]->CameraThink();

	if (m_flTransitionBegin != 0)
	{
		if (GameServer()->GetGameTime() > m_flTransitionBegin + m_flTransitionTime)
			m_flTransitionBegin = 0;
	}
}

TVector CCameraManager::GetCameraPosition()
{
	if (GetFreeMode())
		return m_vecFreeCamera;

	CCamera* pCamera = GetActiveCamera();
	if (!pCamera)
		return TVector(30, 30, 30);

	if (ShouldTransition())
	{
		CCamera* pFrom = m_ahCameras[m_iLastCamera];
		CCamera* pTo = m_ahCameras[m_iCurrentCamera];
		float flLerp = GetTransitionLerp();
		Vector vecDifference = pTo->GetGlobalOrigin() - pFrom->GetGlobalOrigin();
		return pFrom->GetGlobalOrigin() + vecDifference*flLerp;
	}

	return pCamera->GetGlobalOrigin();
}

Vector CCameraManager::GetCameraDirection()
{
	if (GetFreeMode())
		return AngleVector(m_angFreeCamera);

	CCamera* pCamera = GetActiveCamera();
	if (!pCamera)
		return TVector(1,0,0);

	if (ShouldTransition())
	{
		CCamera* pFrom = m_ahCameras[m_iLastCamera];
		CCamera* pTo = m_ahCameras[m_iCurrentCamera];
		float flLerp = GetTransitionLerp();
		EAngle angDifference = pTo->GetGlobalAngles() - pFrom->GetGlobalAngles();
		return AngleVector(pFrom->GetGlobalAngles() + angDifference*flLerp);
	}

	return pCamera->GetGlobalTransform().GetForwardVector();
}

Vector CCameraManager::GetCameraUp()
{
	CCamera* pCamera = GetActiveCamera();
	if (!pCamera)
		return Vector(0, 0, 1);

	if (ShouldTransition())
	{
		CCamera* pFrom = m_ahCameras[m_iLastCamera];
		CCamera* pTo = m_ahCameras[m_iCurrentCamera];
		float flLerp = GetTransitionLerp();
		Vector vecDifference = pTo->GetUpVector() - pFrom->GetUpVector();
		return pFrom->GetUpVector() + vecDifference*flLerp;
	}

	return pCamera->GetUpVector();
}

float CCameraManager::GetCameraFOV()
{
	CCamera* pCamera = GetActiveCamera();
	if (!pCamera)
		return 80.0f;

	if (ShouldTransition())
	{
		CCamera* pFrom = m_ahCameras[m_iLastCamera];
		CCamera* pTo = m_ahCameras[m_iCurrentCamera];
		float flLerp = GetTransitionLerp();
		return RemapVal(flLerp, 0, 1, pFrom->GetFOV(), pTo->GetFOV());
	}

	return pCamera->GetFOV();
}

float CCameraManager::GetCameraOrthoHeight()
{
	if (GetFreeMode())
		return m_flFreeOrthoHeight;

	CCamera* pCamera = GetActiveCamera();
	if (!pCamera)
		return 10;

	if (ShouldTransition())
	{
		CCamera* pFrom = m_ahCameras[m_iLastCamera];
		CCamera* pTo = m_ahCameras[m_iCurrentCamera];
		float flLerp = GetTransitionLerp();
		return RemapVal(flLerp, 0, 1, pFrom->GetOrthoHeight(), pTo->GetOrthoHeight());
	}

	return pCamera->GetOrthoHeight();
}

float CCameraManager::GetCameraNear()
{
	CCamera* pCamera = GetActiveCamera();
	if (!pCamera)
		return 1;

	if (ShouldTransition())
	{
		CCamera* pFrom = m_ahCameras[m_iLastCamera];
		CCamera* pTo = m_ahCameras[m_iCurrentCamera];
		float flLerp = GetTransitionLerp();
		return RemapVal(flLerp, 0, 1, pFrom->GetCameraNear(), pTo->GetCameraNear());
	}

	return pCamera->GetCameraNear();
}

float CCameraManager::GetCameraFar()
{
	CCamera* pCamera = GetActiveCamera();
	if (!pCamera)
		return 10000;

	if (ShouldTransition())
	{
		CCamera* pFrom = m_ahCameras[m_iLastCamera];
		CCamera* pTo = m_ahCameras[m_iCurrentCamera];
		float flLerp = GetTransitionLerp();
		return RemapVal(flLerp, 0, 1, pFrom->GetCameraFar(), pTo->GetCameraFar());
	}

	return pCamera->GetCameraFar();
}

bool CCameraManager::ShouldRenderOrthographic()
{
	CCamera* pCamera = GetActiveCamera();
	if (!pCamera)
		return false;

	return pCamera->ShouldRenderOrthographic();
}

Matrix4x4 CCameraManager::GetCustomProjection()
{
	CCamera* pCamera = GetActiveCamera();
	if (!pCamera)
		return Matrix4x4();

	float flAspectRatio = (float)Application()->GetWindowWidth()/(float)Application()->GetWindowHeight();

	if (ShouldTransition())
	{
		CCamera* pFrom = m_ahCameras[m_iLastCamera];
		CCamera* pTo = m_ahCameras[m_iCurrentCamera];
		float flLerp = GetTransitionLerp();

		Matrix4x4 mFromProjection, mToProjection;

		if (pFrom->ShouldRenderOrthographic())
			mFromProjection = Matrix4x4::ProjectOrthographic(
					-flAspectRatio*pFrom->GetOrthoHeight(), flAspectRatio*pFrom->GetOrthoHeight(),
					-pFrom->GetOrthoHeight(), pFrom->GetOrthoHeight(),
					-100, 100
				);
		else
			mFromProjection = Matrix4x4::ProjectPerspective(
					pFrom->GetFOV(),
					flAspectRatio,
					GetCameraNear(),
					GetCameraFar()
				);

		if (pTo->ShouldRenderOrthographic())
			mToProjection = Matrix4x4::ProjectOrthographic(
					-flAspectRatio*pTo->GetOrthoHeight(), flAspectRatio*pTo->GetOrthoHeight(),
					-pTo->GetOrthoHeight(), pTo->GetOrthoHeight(),
					-100, 100
				);
		else
			mToProjection = Matrix4x4::ProjectPerspective(
					pTo->GetFOV(),
					flAspectRatio,
					GetCameraNear(),
					GetCameraFar()
				);

		return RemapValClamped<Matrix4x4>(flLerp, 0, 1, mFromProjection, mToProjection);
	}

	if (pCamera->ShouldRenderOrthographic())
		return Matrix4x4::ProjectOrthographic(
				-flAspectRatio*pCamera->GetOrthoHeight(), flAspectRatio*pCamera->GetOrthoHeight(),
				-pCamera->GetOrthoHeight(), pCamera->GetOrthoHeight(),
				-100, 100
			);
	else
		return Matrix4x4::ProjectPerspective(
				pCamera->GetFOV(),
				flAspectRatio,
				GetCameraNear(),
				GetCameraFar()
			);
}

bool CCameraManager::ShouldTransition()
{
	if (m_flTransitionBegin == 0)
		return false;

	if (GameServer()->GetGameTime() > m_flTransitionBegin + m_flTransitionTime)
		return false;

	if (m_iLastCamera >= m_ahCameras.size())
		return false;

	if (m_iCurrentCamera >= m_ahCameras.size())
		return false;

	if (!m_ahCameras[m_iLastCamera])
		return false;

	if (!m_ahCameras[m_iCurrentCamera])
		return false;

	return true;
}

CVar cam_transitionlerp("cam_transitionlerp", "0.7");

float CCameraManager::GetTransitionLerp()
{
	return Bias(RemapVal((float)GameServer()->GetGameTime(), (float)m_flTransitionBegin, (float)m_flTransitionBegin+m_flTransitionTime, 0, 1), cam_transitionlerp.GetFloat());
}

void CCameraManager::SetPermaFreeMode(bool bPerma)
{
	m_bPermaFreeMode = bPerma;

	if (bPerma)
	{
		m_vecFreeCamera = GetCameraPosition();
		m_angFreeCamera = VectorAngles((GetCameraDirection()).Normalized());
		m_flFreeOrthoHeight = GetCameraOrthoHeight();
	}
}

void CCameraManager::MouseInput(int x, int y)
{
	int dx, dy;

	dx = x - m_iMouseLastX;
	dy = y - m_iMouseLastY;

	if (m_bFreeMode)
	{
		m_angFreeCamera.y -= (dx/5.0f);
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

bool CCameraManager::KeyDown(int c)
{
	if (CVar::GetCVarBool("cheats") && c == 'Z')
	{
		cam_free.SetValue(m_bFreeMode?"off":"on");

		if (lock_freemode_frustum.GetBool())
		{
			if (GetFreeMode())
				GameServer()->GetRenderer()->FrustumOverride(GetCameraPosition(), GetCameraDirection(), GetCameraFOV(), GetCameraNear(), GetCameraFar());
			else
				GameServer()->GetRenderer()->CancelFrustumOverride();
		}

		return true;
	}

	if (GetFreeMode())
	{
		if (c == 'W')
		{
			m_vecFreeVelocity.x = 1.0f;
			return true;
		}

		if (c == 'S')
		{
			m_vecFreeVelocity.x = -1.0f;
			return true;
		}

		if (c == 'D')
		{
			m_vecFreeVelocity.y = -1.0f;
			return true;
		}

		if (c == 'A')
		{
			m_vecFreeVelocity.y = 1.0f;
			return true;
		}

		if (c == ' ')
		{
			m_vecFreeVelocity.z = 1.0f;
			return true;
		}

		if (c == 'V')
		{
			m_vecFreeVelocity.z = -1.0f;
			return true;
		}
	}

	return false;
}

bool CCameraManager::KeyUp(int c)
{
	if (GetFreeMode())
	{
		if (c == 'W')
		{
			m_vecFreeVelocity.x = 0.0f;
			return true;
		}

		if (c == 'S')
		{
			m_vecFreeVelocity.x = 0.0f;
			return true;
		}

		if (c == 'D')
		{
			m_vecFreeVelocity.y = 0.0f;
			return true;
		}

		if (c == 'A')
		{
			m_vecFreeVelocity.y = 0.0f;
			return true;
		}

		if (c == ' ')
		{
			m_vecFreeVelocity.z = 0.0f;
			return true;
		}

		if (c == 'V' || c == TINKER_KEY_LCTRL)
		{
			m_vecFreeVelocity.z = 0.0f;
			return true;
		}
	}

	return false;
}

void CCameraManager::AddCamera(CCamera* pCamera)
{
	if (!pCamera)
		return;

#ifdef _DEBUG
	for (size_t i = 0; i < m_ahCameras.size(); i++)
		TAssert(m_ahCameras[i] != (const CCamera*)pCamera);
#endif

	m_ahCameras.push_back(pCamera);

	if (m_ahCameras.size() == 1)
		m_iCurrentCamera = 0;

	if (pCamera == GetActiveCamera())
		pCamera->SetActive(true);
	else
		pCamera->SetActive(false);
}

void CCameraManager::RemoveCamera(CCamera* pCamera)
{
	if (!pCamera)
		return;

	for (size_t i = 0; i < m_ahCameras.size(); i++)
	{
		if (m_ahCameras[i] == (const CCamera*)pCamera)
		{
			pCamera->SetActive(false);

			if (m_iCurrentCamera == i)
			{
				m_iCurrentCamera = 0;
				if (GetActiveCamera())
					GetActiveCamera()->SetActive(true);
			}
			else if (m_iCurrentCamera > i)
			{
				m_iCurrentCamera--;
				if (GetActiveCamera())
					GetActiveCamera()->SetActive(true);
			}

			m_ahCameras.erase(m_ahCameras.begin()+i);
			break;
		}
	}
}

CCamera* CCameraManager::GetActiveCamera()
{
	if (m_iCurrentCamera >= m_ahCameras.size())
		return nullptr;

	return m_ahCameras[m_iCurrentCamera];
}

CVar cam_transitiontime("cam_transitiontime", "0.8");

void CCameraManager::SetActiveCamera(CCamera* pCamera, bool bSnap)
{
	if (!pCamera)
		return;

	if (pCamera == GetActiveCamera())
		return;

	for (size_t i = 0; i < m_ahCameras.size(); i++)
	{
		if (m_ahCameras[i] == (const CCamera*)pCamera)
		{
			if (GetActiveCamera())
			{
				GetActiveCamera()->SetActive(false);

				if (!bSnap)
				{
					m_flTransitionBegin = GameServer()->GetGameTime();
					m_flTransitionTime = cam_transitiontime.GetFloat();
					m_iLastCamera = m_iCurrentCamera;
				}
			}

			m_iCurrentCamera = i;

			if (GetActiveCamera())
				GetActiveCamera()->SetActive(true);
			return;
		}
	}
}

CCamera* CCameraManager::GetCamera(size_t iCamera)
{
	if (iCamera >= m_ahCameras.size())
		return nullptr;

	return m_ahCameras[iCamera];
}

CCameraManager* CameraManager()
{
	return GameServer()->GetCameraManager();
}
