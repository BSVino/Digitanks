#ifndef DT_INTRO_CAMERA_H
#define DT_INTRO_CAMERA_H

#include <game/entities/camera.h>
#include <common.h>

class CIntroCamera : public CCamera
{
	DECLARE_CLASS(CIntroCamera, CCamera);

public:
	virtual Vector	GetCameraPosition();
	virtual Vector	GetCameraTarget();
	virtual float	GetCameraFOV();
};

#endif
