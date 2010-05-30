#ifndef DT_CAMERA_H
#define DT_CAMERA_H

#include "vector.h"

class CCamera
{
public:
				CCamera();

public:
	void		SetTarget(Vector vecTarget);
	void		SnapTarget(Vector vecTarget);
	void		SetDistance(float flDistance);
	void		SnapDistance(float flDistance);
	void		SnapAngle(EAngle angCamera);

	void		Shake(Vector vecLocation, float flMagnitude);

	void		Think();

	Vector		GetCameraPosition();
	Vector		GetCameraTarget();

	void		SetFPSMode(bool bOn);
	bool		GetFPSMode() { return m_bFPSMode; };

	void		MouseInput(int x, int y);
	void		MouseButton(int iButton, int iState);
	void		KeyDown(int c);
	void		KeyUp(int c);

public:
	Vector		m_vecOldTarget;
	Vector		m_vecNewTarget;
	float		m_flTargetRamp;

	float		m_flOldDistance;
	float		m_flNewDistance;
	float		m_flDistanceRamp;

	Vector		m_vecShakeLocation;
	float		m_flShakeMagnitude;
	Vector		m_vecShake;

	Vector		m_vecTarget;
	float		m_flDistance;

	Vector		m_vecCamera;
	EAngle		m_angCamera;

	bool		m_bRotatingCamera;

	bool		m_bFPSMode;
	Vector		m_vecFPSCamera;
	EAngle		m_angFPSCamera;
	Vector		m_vecFPSVelocity;
};

#endif
