#ifndef DT_INTRO_CAMERA_H
#define DT_INTRO_CAMERA_H

#include <game/camera.h>

class CIntroCamera : public CCamera
{
public:
	virtual Vector	GetCameraPosition();
	virtual Vector	GetCameraTarget();
	virtual float	GetCameraFOV();
};

#endif
