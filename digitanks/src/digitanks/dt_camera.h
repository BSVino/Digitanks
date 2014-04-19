#ifndef DT_DIGITANKS_CAMERA_H
#define DT_DIGITANKS_CAMERA_H

#include <common.h>

#include <game/entities/camera.h>

class COverheadCamera : public CCamera
{
	REGISTER_ENTITY_CLASS(COverheadCamera, CCamera);

public:
	void			Spawn();

	void			SetTarget(Vector vecTarget);
	void			SnapTarget(Vector vecTarget);
	void			SetDistance(float flDistance);
	void			SnapDistance(float flDistance);
	void			SetAngle(EAngle angCamera);
	void			SnapAngle(EAngle angCamera);

	float			GetDistance() { return m_flDistance; }

	EAngle			GetAngles() { return m_angCamera; }

	void			ZoomOut();
	void			ZoomIn();

	void			Shake(Vector vecLocation, float flMagnitude);

	void			SetCameraGuidedMissile(class CCameraGuidedMissile* pMissile);
	bool			HasCameraGuidedMissile();
	class CCameraGuidedMissile* GetCameraGuidedMissile();

	void			ShowEnemyMoves();
	void			ProjectileFired(class CProjectile* pProjectile);
	void			ReplaceProjectileTarget(class CProjectile* pTarget);
	void			ClearFollowTarget();

	virtual void	EnterGame();
	virtual void	CameraThink();

	virtual float	GetFOV();
	virtual float	GetCameraNear();
	virtual float	GetCameraFar();

	virtual Vector	GetTankFollowPosition(class CDigitank* pTank);

	virtual void MouseMotion(int x, int y, int dx, int dy);
	virtual bool MouseInput(int iButton, tinker_mouse_state_t iState);
	virtual bool KeyDown(int c);
	virtual bool KeyUp(int c);
	virtual void TouchMotion(int iFinger, float x, float y, float dx, float dy);
	virtual bool TouchInput(int iFinger, tinker_mouse_state_t iState, float x, float y);

public:
	Vector			m_vecOldTarget;
	Vector			m_vecNewTarget;
	double			m_flTargetRamp;

	float			m_flOldDistance;
	float			m_flNewDistance;
	double			m_flDistanceRamp;

	EAngle			m_angOldAngle;
	EAngle			m_angNewAngle;
	double			m_flAngleRamp;

	Vector			m_vecShakeLocation;
	float			m_flShakeMagnitude;
	Vector			m_vecShake;

	Vector			m_vecTarget;
	float			m_flDistance;

	Vector			m_vecCamera;
	EAngle			m_angCamera;

	Vector			m_vecVelocity;
	Vector			m_vecGoalVelocity;

	bool			m_bRotatingCamera;
	bool			m_bDraggingCamera;
	bool			m_bFastDraggingCamera;

	bool			m_bMouseDragLeft;
	bool			m_bMouseDragRight;
	bool			m_bMouseDragUp;
	bool			m_bMouseDragDown;

	CEntityHandle<class CCameraGuidedMissile>	m_hCameraGuidedMissile;
	float			m_flCameraGuidedFOV;
	float			m_flCameraGuidedFOVGoal;

	CEntityHandle<class CDigitank>		m_hTankTarget;
	CEntityHandle<class CProjectile>	m_hTankProjectile;
	double			m_flTransitionToProjectileTime;

	double			m_flTimeSinceNewGame;
};

#endif
