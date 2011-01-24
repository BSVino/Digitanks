#ifndef LW_PLANE_H
#define LW_PLANE_H

#include "vector.h"

class Plane
{
public:
	void		Normalize()
	{
		float flMagnitude = 1/sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
		n.x *= flMagnitude;
		n.y *= flMagnitude;
		n.z *= flMagnitude;
		d *= flMagnitude;
	}

public:
	Vector		n;
	float		d;
};

#endif
