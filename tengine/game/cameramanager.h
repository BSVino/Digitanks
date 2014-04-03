#pragma once

#include <tengine_config.h>
#include <vector.h>
#include <tvector.h>
#include <matrix.h>

#include <keys.h>
#include <game/entityhandle.h>

class CCamera;

class CCameraManager
{
public:
					CCameraManager();

public:
	virtual void	Think();

	virtual TVector	GetCameraPosition();
	virtual Vector  GetCameraDirection();
	virtual Vector  GetCameraUp();
	virtual float	GetCameraFOV();
	virtual float	GetCameraOrthoHeight();
	virtual float	GetCameraNear();
	virtual float	GetCameraFar();
	virtual bool	ShouldRenderOrthographic();

	virtual bool      UseCustomProjection() { return true; }
	virtual Matrix4x4 GetCustomProjection();

	virtual bool	ShouldTransition();
	virtual float	GetTransitionLerp();

	void            SetPermaFreeMode(bool bPermaFreeMode);
	bool			GetFreeMode() { return m_bFreeMode || m_bPermaFreeMode; };
	TVector			GetFreeCameraPosition() const { return m_vecFreeCamera; };
	EAngle			GetFreeCameraAngles() const { return m_angFreeCamera; };
	float			GetFreeCameraOrthoHeight() const { return m_flFreeOrthoHeight; };

	virtual void MouseMotion(int x, int y, int dx, int dy);
	virtual bool MouseInput(int iButton, tinker_mouse_state_t iState);
	virtual bool KeyDown(int c);
	virtual bool KeyUp(int c);

	void			AddCamera(CCamera* pCamera);
	void			RemoveCamera(CCamera* pCamera);

	void			SetActiveCamera(CCamera* pCamera, bool bSnap = false);
	CCamera*		GetActiveCamera();

	size_t			GetNumCameras() { return m_ahCameras.size(); }
	class CCamera*	GetCamera(size_t iCamera);

public:
	bool			m_bPermaFreeMode;
	bool			m_bFreeMode;
	TVector			m_vecFreeCamera;
	EAngle			m_angFreeCamera;
	TVector			m_vecFreeVelocity;
	float			m_flFreeOrthoHeight;

	tvector<CEntityHandle<CCamera>>	m_ahCameras;
	size_t			m_iCurrentCamera;

	double			m_flTransitionBegin;
	float			m_flTransitionTime;
	size_t			m_iLastCamera;
};

CCameraManager* CameraManager();
