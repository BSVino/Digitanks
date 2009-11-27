#ifndef CF_GEOMETRY_H
#define CF_GEOMETRY_H

#include "vector.h"
#include <vector>

class Ray
{
public:
				Ray(Vector vecPos, Vector vecDir);

	bool		IntersectTriangle(Vector v1, Vector v2, Vector v3, Vector* pvecHit = NULL) const;

	Vector		m_vecPos;
	Vector		m_vecDir;
};

inline Ray::Ray(Vector vecPos, Vector vecDir)
{
	m_vecPos = vecPos;
	m_vecDir = vecDir;
}

inline bool Ray::IntersectTriangle(Vector v0, Vector v1, Vector v2, Vector* pvecHit) const
{
	Vector u = v1 - v0;
	Vector v = v2 - v0;
	Vector n = u.Cross(v);

	Vector w0 = m_vecPos - v0;

	float a = -n.Dot(w0);
	float b = n.Dot(m_vecDir);

	float ep = 1e-4f;

	if (fabs(b) < ep)
	{
		if (a == 0)			// Ray is parallel
			return false;	// Ray is inside plane
		else
			return false;	// Ray is somewhere else
	}

	float r = a/b;
	if (r < 0)
		return false;		// Ray goes away from the triangle

	Vector vecPoint = m_vecPos + m_vecDir*r;
	if (pvecHit)
		*pvecHit = vecPoint;

	float uu = u.Dot(u);
	float uv = u.Dot(v);
	float vv = v.Dot(v);
	Vector w = vecPoint - v0;
	float wu = w.Dot(u);
	float wv = w.Dot(v);

	float D = uv * uv - uu * vv;

	float s, t;

	s = (uv * wv - vv * wu) / D;
	if (s <= ep || s >= 1)		// Intersection point is outside the triangle
		return false;

	t = (uv * wu - uu * wv) / D;
	if (t <= ep || (s+t) >= 1)	// Intersection point is outside the triangle
		return false;

	return true;
}

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