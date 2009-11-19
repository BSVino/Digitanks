#ifndef CF_GEOMETRY_H
#define CF_GEOMETRY_H

#include "vector.h"
#include <vector>

// Geometry-related functions

inline bool SameSide(Vector p1, Vector p2, Vector a, Vector b)
{
    Vector cp1 = (b-a).Cross(p1-a);
    Vector cp2 = (b-a).Cross(p2-a);
    return (cp1.Dot(cp2) > 0);
}

inline bool PointInTriangle(Vector p, Vector a, Vector b, Vector c)
{
	return (SameSide(p, a, b, c) && SameSide(p, b, a, c) && SameSide(p, c, a, b));
}

inline float DistanceToLine(Vector p, Vector v1, Vector v2)
{
	Vector v = v2 - v1;
	Vector w = p - v1;

	float c1 = w.Dot(v);
	float c2 = v.Dot(v);

	float b = c1/c2;

	Vector vb = v1 + v*b;
	return (vb - p).Length();
}

inline float DistanceToLineSegment(Vector p, Vector v1, Vector v2)
{
	Vector v = v2 - v1;
	Vector w = p - v1;

	float c1 = w.Dot(v);

	if (c1 < 0)
		return (v1-p).Length();

	float c2 = v.Dot(v);
	if (c2 < c1)
		return (v2-p).Length();

	float b = c1/c2;

	Vector vb = v1 + v*b;
	return (vb - p).Length();
}

inline float DistanceToPlane(Vector p, Vector v, Vector n)
{
	float sb, sn, sd;

	sn = -n.Dot(p - v);
	sd = n.Dot(n);
	sb = sn/sd;

	Vector b = p + n * sb;
	return (p - b).Length();
}

inline float DistanceToPolygon(Vector p, std::vector<Vector>& v, Vector n)
{
	float flPlaneDistance = DistanceToPlane(p, v[0], n);

	size_t i;

	bool bFoundPoint = false;

	for (i = 0; i < v.size()-2; i++)
	{
		if (PointInTriangle(p, v[0], v[i+1], v[i+2]))
		{
			bFoundPoint = true;
			break;
		}
	}

	if (bFoundPoint)
		return flPlaneDistance;

	float flClosestPoint = -1;
	for (i = 0; i < v.size(); i++)
	{
		float flPointDistance = (v[i] - p).Length();
		if (flClosestPoint == -1 || (flPointDistance < flClosestPoint))
			flClosestPoint = flPointDistance;

		float flLineDistance;
		if (i == v.size() - 1)
			flLineDistance = DistanceToLineSegment(p, v[i], v[0]);
		else
			flLineDistance = DistanceToLineSegment(p, v[i], v[i+1]);

		if (flClosestPoint == -1 || (flLineDistance < flClosestPoint))
			flClosestPoint = flLineDistance;
	}

	return flClosestPoint;
}

inline float TriangleArea(Vector a, Vector b, Vector c)
{
	return (a-b).Cross(a-c).Length()/2;
}

#endif