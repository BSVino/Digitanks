#include "intro_camera.h"

#include <tinker/application.h>

Vector CIntroCamera::GetCameraPosition()
{
	if (m_bFreeMode)
		return BaseClass::GetCameraPosition();

	return Vector(0, 0, 0);
}

Vector CIntroCamera::GetCameraTarget()
{
	if (m_bFreeMode)
		return BaseClass::GetCameraTarget();

	return Vector(-500, 0, 0);
}

float CIntroCamera::GetCameraFOV()
{
	return 90;
}
