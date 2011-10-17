#ifndef DT_CAMERA_H
#define DT_CAMERA_H

#include "vector.h"

class CCamera
{
public:
				CCamera();

public:
	virtual void	Think();

	virtual Vector	GetCameraPosition();
	virtual Vector	GetCameraTarget();
	virtual float	GetCameraFOV();
	virtual float	GetCameraNear() { return 1.0f; };
	virtual float	GetCameraFar() { return 10000.0f; };

	bool			GetFreeMode() { return m_bFreeMode; };

	virtual void	MouseInput(int x, int y);
	virtual void	MouseButton(int iButton, int iState) {};
	virtual void	KeyDown(int c);
	virtual void	KeyUp(int c);

public:
	bool			m_bFreeMode;
	Vector			m_vecFreeCamera;
	EAngle			m_angFreeCamera;
	Vector			m_vecFreeVelocity;

	int				m_iMouseLastX;
	int				m_iMouseLastY;
};

#endif
