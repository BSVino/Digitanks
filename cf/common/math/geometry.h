#ifndef CF_GEOMETRY_H
#define CF_GEOMETRY_H

#include "vector.h"

// Geometry-related functions

bool SameSide(Vector p1, Vector p2, Vector a, Vector b)
{
    Vector cp1 = (b-a).Cross(p1-a);
    Vector cp2 = (b-a).Cross(p2-a);
    return (cp1.Dot(cp2) > 0);
}

bool PointInTriangle(Vector p, Vector a, Vector b, Vector c)
{
	return (SameSide(p, a, b, c) && SameSide(p, b, a, c) && SameSide(p, c, a, b));
}

float DistanceToLine(Vector p, Vector v1, Vector v2)
{
	Vector v = v2 - v1;
	Vector w = p - v1;

	float c1 = w.Dot(v);
	float c2 = v.Dot(v);

	float b = c1/c2;

	Vector vb = v1 + v*b;
	return (vb - p).Length();
}

#endif