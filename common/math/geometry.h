#ifndef LW_GEOMETRY_H
#define LW_GEOMETRY_H

#include "vector.h"
#include <EASTL/vector.h>

class Rect
{
public:
	Rect()
	{
		x = 0;
		y = 0;
		w = 0;
		h = 0;
	}

	Rect(int X, int Y, int W, int H)
	{
		x = X;
		y = Y;
		w = W;
		h = H;
	}

	int x, y, w, h;
};

class Ray
{
public:
				Ray(Vector vecPos, Vector vecDir);

	Vector		m_vecPos;
	Vector		m_vecDir;
};

inline Ray::Ray(Vector vecPos, Vector vecDir)
{
	m_vecPos = vecPos;
	m_vecDir = vecDir;
}

class AABB
{
public:
				AABB() {};
				AABB(Vector vecMins, Vector vecMaxs);

	Vector		Center() const;
	Vector		Size() const;

	bool		Inside(const AABB& oBox) const;
	bool		Intersects(const AABB& oBox) const;

	Vector		m_vecMins;
	Vector		m_vecMaxs;
};

inline AABB::AABB(Vector vecMins, Vector vecMaxs)
{
	m_vecMins = vecMins;
	m_vecMaxs = vecMaxs;
}

inline Vector AABB::Center() const
{
	return (m_vecMins + m_vecMaxs)/2;
}

inline Vector AABB::Size() const
{
	return m_vecMaxs - m_vecMins;
}

inline bool AABB::Inside(const AABB& oBox) const
{
	if (m_vecMins.x < oBox.m_vecMins.x)
		return false;

	if (m_vecMins.y < oBox.m_vecMins.y)
		return false;

	if (m_vecMins.z < oBox.m_vecMins.z)
		return false;

	if (m_vecMins.x > oBox.m_vecMins.x)
		return false;

	if (m_vecMins.y > oBox.m_vecMins.y)
		return false;

	if (m_vecMins.z > oBox.m_vecMins.z)
		return false;

	return true;
}

inline bool AABB::Intersects(const AABB& oBox) const
{
	if (m_vecMins.x > oBox.m_vecMaxs.x)
		return false;

	if (oBox.m_vecMins.x > m_vecMaxs.x)
		return false;

	if (m_vecMins.y > oBox.m_vecMaxs.y)
		return false;

	if (oBox.m_vecMins.y > m_vecMaxs.y)
		return false;

	if (m_vecMins.z > oBox.m_vecMaxs.z)
		return false;

	if (oBox.m_vecMins.z > m_vecMaxs.z)
		return false;

	return true;
}

// Geometry-related functions

inline bool SameSide(Vector p1, Vector p2, Vector a, Vector b)
{
	Vector ba = b-a;
    Vector cp1 = ba.Cross(p1-a);
    Vector cp2 = ba.Cross(p2-a);
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

inline float DistanceToLineSegment(Vector p, Vector v1, Vector v2, Vector* i = NULL)
{
	Vector v = v2 - v1;
	Vector w = p - v1;

	float c1 = w.Dot(v);

	if (c1 < 0)
	{
		if (i)
			*i = v1;
		return (v1-p).Length();
	}

	float c2 = v.Dot(v);
	if (c2 < c1)
	{
		if (i)
			*i = v2;
		return (v2-p).Length();
	}

	if (c2 < 0.001f)
		return 0;

	float b = c1/c2;

	Vector vb = v1 + v*b;

	if (i)
		*i = vb;

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

inline float DistanceToPolygon(Vector p, eastl::vector<Vector>& v, Vector n)
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

inline bool RayIntersectsTriangle(Ray vecRay, Vector v0, Vector v1, Vector v2, Vector* pvecHit = NULL)
{
	Vector u = v1 - v0;
	Vector v = v2 - v0;
	Vector n = u.Cross(v);

	Vector w0 = vecRay.m_vecPos - v0;

	float a = -n.Dot(w0);
	float b = n.Dot(vecRay.m_vecDir);

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

	Vector vecPoint = vecRay.m_vecPos + vecRay.m_vecDir*r;
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
	if (s < 0 || s > 1)		// Intersection point is outside the triangle
		return false;

	t = (uv * wu - uu * wv) / D;
	if (t < 0 || (s+t) > 1)	// Intersection point is outside the triangle
		return false;

	return true;
}

inline bool ClipRay(float flMin, float flMax, float a, float d, float& tmin, float& tmax)
{
	const float flEpsilon = 1e-5f;

	if (fabs(d) < flEpsilon)
	{
		if (d >= 0.0f)
			return (a <= flMax);
		else
			return (a >= flMin);
	}

	float umin = (flMin - a)/d;
	float umax = (flMax - a)/d;

	if (umin > umax)
	{
		float yar = umin;
		umin = umax;
		umax = yar;
	}

	if (umax < tmin || umin > tmax)
		return false;

	tmin = (umin>tmin)?umin:tmin;
	tmax = (umax<tmax)?umax:tmax;

	return (tmax>tmin);
}

inline bool RayIntersectsAABB(const Ray& r, const AABB& b)
{
	float tmin = 0;
	float tmax = b.Size().LengthSqr();	// It's a ray so make tmax effectively infinite.
	if (tmax < 1)
		tmax = 100;
	float flDistToBox = (r.m_vecPos - b.Center()).LengthSqr();
	if (flDistToBox < 1)
		flDistToBox = 100;
	tmax *= flDistToBox * 100;

	if (!ClipRay(b.m_vecMins.x, b.m_vecMaxs.x, r.m_vecPos.x, r.m_vecDir.x, tmin, tmax))
		return false;

	if (!ClipRay(b.m_vecMins.y, b.m_vecMaxs.y, r.m_vecPos.y, r.m_vecDir.y, tmin, tmax))
		return false;

	if (!ClipRay(b.m_vecMins.z, b.m_vecMaxs.z, r.m_vecPos.z, r.m_vecDir.z, tmin, tmax))
		return false;

	return true;
}

inline bool ClipSegment(float flMin, float flMax, float a, float b, float d, float& tmin, float& tmax)
{
	const float flEpsilon = 1e-5f;

	if (fabs(d) < flEpsilon)
	{
		if (d >= 0.0f)
			return !(b < flMin || a > flMax);
		else
			return !(a < flMin || b > flMax);
	}

	float umin = (flMin - a)/d;
	float umax = (flMax - a)/d;

	if (umin > umax)
	{
		float yar = umin;
		umin = umax;
		umax = yar;
	}

	if (umax < tmin || umin > tmax)
		return false;

	tmin = (umin>tmin)?umin:tmin;
	tmax = (umax<tmax)?umax:tmax;

	return (tmax>tmin);
}

inline bool SegmentIntersectsAABB(const Vector& v1, const Vector& v2, const AABB& b)
{
	float tmin = 0;
	float tmax = 1;

	Vector vecDir = v2 - v1;

	if (!ClipSegment(b.m_vecMins.x, b.m_vecMaxs.x, v1.x, v2.x, vecDir.x, tmin, tmax))
		return false;

	if (!ClipSegment(b.m_vecMins.y, b.m_vecMaxs.y, v1.y, v2.y, vecDir.y, tmin, tmax))
		return false;

	if (!ClipSegment(b.m_vecMins.z, b.m_vecMaxs.z, v1.z, v2.z, vecDir.z, tmin, tmax))
		return false;

	return true;
}

inline bool LineSegmentIntersectsTriangle(Vector s0, Vector s1, Vector v0, Vector v1, Vector v2, Vector* pvecHit = NULL)
{
	Vector u = v1 - v0;
	Vector v = v2 - v0;
	Vector n = u.Cross(v);

	Vector w0 = s0 - v0;

	float a = -n.Dot(w0);
	float b = n.Dot(s1-s0);

	float ep = 1e-4f;

	if (fabs(b) < ep)
	{
		if (a == 0)			// Segment is parallel
			return true;	// Segment is inside plane
		else
			return false;	// Segment is somewhere else
	}

	float r = a/b;
	if (r < 0)
		return false;		// Segment goes away from the triangle
	if (r > 1)
		return false;		// Segment goes away from the triangle

	Vector vecPoint = s0 + (s1-s0)*r;
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

inline bool LineSegmentIntersectsSphere(const Vector& v1, const Vector& v2, const Vector& s, float flRadius, Vector& vecPoint)
{
	Vector vecLine = v2 - v1;
	Vector vecSphere = v1 - s;

	float flA = vecLine.LengthSqr();
	float flB = 2 * vecSphere.Dot(vecLine);
	float flC = vecSphere.LengthSqr() - flRadius*flRadius;

	float flBB4AC = flB*flB - 4*flA*flC;
	if (flBB4AC < 0)
		return false;

	float flSqrt = sqrt(flBB4AC);
	float flPlus = (-flB + flSqrt)/(2*flA);
	float flMinus = (-flB - flSqrt)/(2*flA);

	float flDistance = vecLine.Length();

	Vector vecDirection = vecLine / flDistance;
	Vector vecPlus = v1 + vecDirection * (flPlus * flDistance);
	Vector vecMinus = v1 + vecDirection * (flMinus * flDistance);

	if ((vecPlus - v1).LengthSqr() < (vecMinus - v1).LengthSqr())
		vecPoint = vecPlus;
	else
		vecPoint = vecMinus;

	return true;
}

inline bool PointInsideAABB( AABB oBox, Vector v )
{
	const float flEpsilon = 1e-4f;

	for (size_t i = 0; i < 3; i++)
	{
		float flVI = v[i];

		if (flVI < oBox.m_vecMins[i] - flEpsilon || flVI > oBox.m_vecMaxs[i] + flEpsilon)
			return false;
	}

	return true;
}

inline bool	TriangleIntersectsAABB( AABB oBox, Vector v0, Vector v1, Vector v2)
{
	// Trivial case rejection: If any of the points are inside the box, return true immediately.
	if (PointInsideAABB(oBox, v0))
		return true;
	if (PointInsideAABB(oBox, v1))
		return true;
	if (PointInsideAABB(oBox, v2))
		return true;

	size_t i;

	// Trivial case rejection: If all three points are on one side of the box then the triangle must be outside of it.
	for (i = 0; i < 3; i++)
	{
		float flBoxMax = oBox.m_vecMaxs[i];
		float flBoxMin = oBox.m_vecMins[i];

		float flV0 = v0[i];
		float flV1 = v1[i];
		float flV2 = v2[i];

		if (flV0 > flBoxMax && flV1 > flBoxMax && flV2 > flBoxMax)
			return false;
		if (flV0 < flBoxMin && flV1 < flBoxMin && flV2 < flBoxMin)
			return false;
	}

	if (SegmentIntersectsAABB(v0, v1, oBox))
		return true;

	if (SegmentIntersectsAABB(v1, v2, oBox))
		return true;

	if (SegmentIntersectsAABB(v0, v2, oBox))
		return true;

	Vector c0 = oBox.m_vecMins;
	Vector c1 = Vector(oBox.m_vecMins.x, oBox.m_vecMins.y, oBox.m_vecMaxs.z);
	Vector c2 = Vector(oBox.m_vecMins.x, oBox.m_vecMaxs.y, oBox.m_vecMins.z);
	Vector c3 = Vector(oBox.m_vecMins.x, oBox.m_vecMaxs.y, oBox.m_vecMaxs.z);
	Vector c4 = Vector(oBox.m_vecMaxs.x, oBox.m_vecMins.y, oBox.m_vecMins.z);
	Vector c5 = Vector(oBox.m_vecMaxs.x, oBox.m_vecMins.y, oBox.m_vecMaxs.z);
	Vector c6 = Vector(oBox.m_vecMaxs.x, oBox.m_vecMaxs.y, oBox.m_vecMins.z);
	Vector c7 = oBox.m_vecMaxs;

	// Build a list of line segments in the cube to test against the triangle.
	Vector aLines[32];

	// Bottom four
	aLines[0] = c0;
	aLines[1] = c1;

	aLines[2] = c1;
	aLines[3] = c2;

	aLines[4] = c2;
	aLines[5] = c3;

	aLines[6] = c3;
	aLines[7] = c0;

	// Sides
	aLines[8] = c0;
	aLines[9] = c4;

	aLines[10] = c1;
	aLines[11] = c5;

	aLines[12] = c2;
	aLines[13] = c6;

	aLines[14] = c3;
	aLines[15] = c7;

	// Top
	aLines[16] = c4;
	aLines[17] = c5;

	aLines[18] = c5;
	aLines[19] = c6;

	aLines[20] = c6;
	aLines[21] = c7;

	aLines[22] = c7;
	aLines[23] = c4;

	// Diagonals
	aLines[24] = c0;
	aLines[25] = c6;

	aLines[26] = c1;
	aLines[27] = c7;

	aLines[28] = c2;
	aLines[29] = c4;

	aLines[30] = c3;
	aLines[31] = c5;

	// If any of the segments intersects with the triangle then we have a winner.
	for (i = 0; i < 32; i+=2)
	{
		if (LineSegmentIntersectsTriangle(aLines[i], aLines[i+1], v0, v1, v2))
			return true;
	}

	return false;
}

inline size_t FindEar(const eastl::vector<Vector>& avecPoints)
{
	size_t iPoints = avecPoints.size();

	// A triangle is always an ear.
	if (iPoints <= 3)
		return 0;

	size_t i;

	Vector vecFaceNormal;

	// Calculate the face normal.
	for (i = 0; i < iPoints; i++)
	{
		size_t iNext = (i+1)%iPoints;

		Vector vecPoint = avecPoints[i];
		Vector vecNextPoint = avecPoints[iNext];

		vecFaceNormal.x += (vecPoint.y - vecNextPoint.y) * (vecPoint.z + vecNextPoint.z);
		vecFaceNormal.y += (vecPoint.z - vecNextPoint.z) * (vecPoint.x + vecNextPoint.x);
		vecFaceNormal.z += (vecPoint.x - vecNextPoint.x) * (vecPoint.y + vecNextPoint.y);
	}

	vecFaceNormal.Normalize();

	for (i = 0; i < iPoints; i++)
	{
		size_t iLast = i==0?iPoints-1:i-1;
		size_t iNext = i==iPoints-1?0:i+1;

		Vector vecLast = avecPoints[iLast];
		Vector vecThis = avecPoints[i];
		Vector vecNext = avecPoints[iNext];

		// Concave ones can not be ears.
		if ((vecLast-vecThis).Cross(vecLast-vecNext).Dot(vecFaceNormal) < 0)
			continue;

		bool bFoundPoint = false;
		for (size_t j = 0; j < iPoints; j++)
		{
			if (j == i || j == iLast || j == iNext)
				continue;

			if (PointInTriangle(avecPoints[j], vecLast, vecThis, vecNext))
			{
				bFoundPoint = true;
				break;
			}
		}

		if (!bFoundPoint)
			return i;
	}

	return 0;
}

inline void FindLaunchVelocity(const Vector& vecOrigin, const Vector& vecTarget, float flGravity, Vector& vecForce, float& flTime, float flCurve = -0.03f)
{
	Vector vecDistance = vecTarget - vecOrigin;
	float flY = vecDistance.y;
	vecDistance.y = 0;
	float flX = vecDistance.Length();

	float flA = flCurve;
	float flH = (flX*flX - (flY/flA))/(2*flX);
	float flK = -flA*flH*flH;
	float flB = -2*flH*flA;

	float flForce = sqrt(2*-flGravity*flK);

	float flTimeToVertex = -flForce/flGravity;
	float flTimeToLand = sqrt(2*-(flK-flY)/flGravity);

	flTime = flTimeToVertex + flTimeToLand;

	Vector vecDirection = vecDistance.Normalized() * flX / flTime;
	vecDirection.y = flForce;

	vecForce = vecDirection;
}

#endif