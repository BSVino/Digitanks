#ifndef DT_INTRO_CAMERA_H
#define DT_INTRO_CAMERA_H

#include <game/entities/camera.h>
#include <common.h>

class CIntroCamera : public CCamera
{
	REGISTER_ENTITY_CLASS(CIntroCamera, CCamera);

public:
	void  Spawn();

	virtual void	CameraThink();

	virtual float	GetOrthoHeight() const { return m_flOrthoHeight; }
	virtual float	GetCameraNear() const { return 1; }
	virtual float	GetCameraFar() const { return 2000; }
};

#endif
