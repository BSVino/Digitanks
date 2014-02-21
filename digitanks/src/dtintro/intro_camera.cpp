#include "intro_camera.h"

#include <tinker/application.h>

Vector CIntroCamera::GetCameraPosition()
{
	return Vector(500, 0, 0);
}

Vector CIntroCamera::GetCameraTarget()
{
	return Vector(0, 0, 0);
}

float CIntroCamera::GetCameraFOV()
{
	return 90;
}
