/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LW_GEOMETRY_H
#define LW_GEOMETRY_H

#include <algorithm>

#include "../common.h"
#include "../tstring.h"
#include "vector.h"
#include <tvector.h>

class CCollisionResult
{
public:
	CCollisionResult()
	{
		flFraction = 1;
		bHit = false;
		bStartInside = false;
	}

public:
	bool	bHit;
	bool	bStartInside;
	double	flFraction;
	Vector	vecHit;
	Vector	vecNormal;
};

template <class T>
class TRect
{
public:
	TRect()
	{
		x = 0;
		y = 0;
		w = 0;
		h = 0;
	}

	TRect(T X, T Y, T W, T H)
	{
		x = X;
		y = Y;
		w = W;
		h = H;
	}

public:
	T		Size() const { return w*h; }

	T		Right() const { return x + w; }
	T		Bottom() const { return y + h; }

	void	SetRight(float r);
	void	SetBottom(float b);

	bool	Intersects(const TRect<T>& F) const;
	bool	Union(const TRect<T>& r);

	void	operator/=(T s)
	{
		x /= s;
		y /= s;
		w /= s;
		h /= s;
	}

public:
	T x, y, w, h;
};

template <class T>
void TRect<T>::SetRight(float r)
{
	w = r - x;
}

template <class T>
void TRect<T>::SetBottom(float b)
{
	h = b - y;
}

template <class T>
bool TRect<T>::Intersects(const TRect<T>& r) const
{
	if (x > r.Right())
		return false;

	if (r.x > Right())
		return false;

	if (y > r.Bottom())
		return false;

	if (r.y > Bottom())
		return false;

	return true;
}

template <class T>
bool TRect<T>::Union(const TRect<T>& r)
{
	if (!Intersects(r))
		return false;

	if (r.x > x)
	{
		T right = Right();
		x = r.x;
		SetRight(right);
	}

	if (r.y > y)
	{
		T bottom = Bottom();
		y = r.y;
		SetBottom(bottom);
	}

	if (Right() > r.Right())
		SetRight(r.Right());

	if (Bottom() > r.Bottom())
		SetBottom(r.Bottom());

	return true;
}

typedef TRect<int> Rect;
typedef TRect<float> FRect;

class Ray
{
public:
	Ray() {};
	Ray(Vector vecPos, Vector vecDir);

public:
	Vector		m_vecPos;
	Vector		m_vecDir;
};

inline Ray::Ray(Vector vecPos, Vector vecDir)
{
	m_vecPos = vecPos;
	m_vecDir = vecDir;
}

template <class F>
class TemplateAABB
{
public:
						TemplateAABB() {};
						TemplateAABB(const TemplateVector<F>& vecMins, const TemplateVector<F>& vecMaxs);

public:
	TemplateVector<F>		Center() const;
	TemplateVector<F>		Size() const;

	bool		Inside(const TemplateAABB<F>& oBox) const;
	bool		Inside(const TemplateVector<F>& vecPoint) const;
	bool		Inside2D(const TemplateVector<F>& vecPoint) const;
	bool		Intersects(const TemplateAABB<F>& oBox) const;

	void        Expand(const TemplateVector<F>& vecPoint);
	void        Expand(const TemplateAABB<F>& aabbBox); // Assumes well formed aabbBox with min < max for x y and z.

	TemplateAABB<F>		operator+(const TemplateAABB<F>& oBox) const;
	TemplateAABB<F>		operator*(float s) const;
	bool		operator==(const TemplateAABB<F>& o) const;
	TemplateAABB<F>&       operator+=(const Vector& v);

public:
	TemplateVector<F>		m_vecMins;
	TemplateVector<F>		m_vecMaxs;
};

typedef TemplateAABB<float> AABB;

template <class F>
inline TemplateAABB<F>::TemplateAABB(const TemplateVector<F>& vecMins, const TemplateVector<F>& vecMaxs)
{
	m_vecMins = vecMins;
	m_vecMaxs = vecMaxs;
}

template <class F>
inline TemplateVector<F> TemplateAABB<F>::Center() const
{
	return (m_vecMins + m_vecMaxs)/2;
}

template <class F>
inline TemplateVector<F> TemplateAABB<F>::Size() const
{
	return m_vecMaxs - m_vecMins;
}

template <class F>
inline bool TemplateAABB<F>::Inside(const TemplateAABB<F>& oBox) const
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

template <class F>
inline bool TemplateAABB<F>::Inside(const TemplateVector<F>& vecPoint) const
{
	const float flEpsilon = 1e-4f;

	for (size_t i = 0; i < 3; i++)
	{
		float flVI = vecPoint[i];

		if (flVI < m_vecMins[i] - flEpsilon || flVI > m_vecMaxs[i] + flEpsilon)
			return false;
	}

	return true;
}

template <class F>
inline bool TemplateAABB<F>::Inside2D(const TemplateVector<F>& vecPoint) const
{
	const float flEpsilon = 1e-4f;

	if (vecPoint.x < m_vecMins.x - flEpsilon || vecPoint.x > m_vecMaxs.x + flEpsilon)
		return false;

	if (vecPoint.y < m_vecMins.y - flEpsilon || vecPoint.y > m_vecMaxs.y + flEpsilon)
		return false;

	return true;
}

template <class F>
inline bool TemplateAABB<F>::Intersects(const TemplateAABB<F>& oBox) const
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

template <class F>
inline void TemplateAABB<F>::Expand(const TemplateVector<F>& vecNew)
{
	for (size_t i = 0; i < 3; i++)
	{
		if (vecNew[i] < m_vecMins[i])
			m_vecMins[i] = vecNew[i];
		else if (vecNew[i] > m_vecMaxs[i])
			m_vecMaxs[i] = vecNew[i];
	}
}

template <class F>
inline void TemplateAABB<F>::Expand(const TemplateAABB<F>& aabbNew)
{
	for (size_t i = 0; i < 3; i++)
	{
		if (aabbNew.m_vecMins[i] < m_vecMins[i])
			m_vecMins[i] = aabbNew.m_vecMins[i];
		else if (aabbNew.m_vecMaxs[i] > m_vecMaxs[i])
			m_vecMaxs[i] = aabbNew.m_vecMaxs[i];
	}
}

template<>
inline AABB AABB::operator+(const AABB& oBox) const
{
	AABB r(*this);

	r.m_vecMaxs += oBox.m_vecMaxs;
	r.m_vecMins += oBox.m_vecMins;

	return r;
}

template<>
inline AABB AABB::operator*(float s) const
{
	AABB r(*this);

	r.m_vecMaxs *= s;
	r.m_vecMins *= s;

	return r;
}

template<>
inline bool AABB::operator==(const AABB& o) const
{
	return (m_vecMins == o.m_vecMins) && (m_vecMaxs == o.m_vecMaxs);
}

template<>
inline AABB& AABB::operator+=(const Vector& v)
{
	m_vecMaxs += v;
	m_vecMins += v;

	return *this;
}

// Geometry-related functions

inline bool SameSide(const Vector& p1, const Vector& p2, const Vector& a, const Vector& b)
{
	Vector ba = b-a;
    Vector cp1 = ba.Cross(p1-a);
    Vector cp2 = ba.Cross(p2-a);
    return (cp1.Dot(cp2) > 0);
}

inline bool PointInTriangle(const Vector& p, const Vector& a, const Vector& b, const Vector& c)
{
	return (SameSide(p, a, b, c) && SameSide(p, b, a, c) && SameSide(p, c, a, b));
}

inline float DistanceToLine(const Vector& p, const Vector& v1, const Vector& v2)
{
	Vector v = v2 - v1;
	Vector w = p - v1;

	float c1 = w.Dot(v);
	float c2 = v.Dot(v);

	float b = c1/c2;

	Vector vb = v1 + v*b;
	return (vb - p).Length();
}

inline float DistanceToLineSegment(const Vector& p, const Vector& v1, const Vector& v2, Vector* i = NULL)
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

	if (c2 == 0.0f)
		return 0;

	float b = c1/c2;

	Vector vb = v1 + v*b;

	if (i)
		*i = vb;

	return (vb - p).Length();
}

inline float DistanceToPlane(const Vector& vecPoint, const Vector& vecPlane, const Vector& vecPlaneNormal)
{
	float sb, sn, sd;

	sn = -vecPlaneNormal.Dot(vecPoint - vecPlane);
	sd = vecPlaneNormal.Dot(vecPlaneNormal);
	sb = sn/sd;

	Vector b = vecPoint + vecPlaneNormal * sb;
	return (vecPoint - b).Length();
}

inline float DistanceToPlaneSqr(const Vector& vecPoint, const Vector& vecPlane, const Vector& vecPlaneNormal)
{
	float sb, sn, sd;

	sn = -vecPlaneNormal.Dot(vecPoint - vecPlane);
	sd = vecPlaneNormal.Dot(vecPlaneNormal);
	sb = sn/sd;

	Vector b = vecPoint + vecPlaneNormal * sb;
	return (vecPoint - b).LengthSqr();
}

#define smaller(l, r) (((l)<(r))?(l):(r))

inline float DistanceToQuad(const Vector& p, const Vector& v1, const Vector& v2, const Vector& v3, const Vector& v4, const Vector& n)
{
	float flPlaneDistance = DistanceToPlane(p, v1, n);

	if (PointInTriangle(p, v1, v2, v3))
		return flPlaneDistance;

	if (PointInTriangle(p, v1, v3, v4))
		return flPlaneDistance;

	float flClosestPointSqr = (v1 - p).LengthSqr();

	float flV2Sqr = (v2 - p).LengthSqr();
	float flV3Sqr = (v3 - p).LengthSqr();
	float flV4Sqr = (v4 - p).LengthSqr();

	flClosestPointSqr = smaller(flClosestPointSqr, smaller(flV2Sqr, smaller(flV3Sqr, flV4Sqr)));

	float flClosestPoint = sqrt(flClosestPointSqr);

	float flV12 = DistanceToLineSegment(p, v1, v2);
	float flV23 = DistanceToLineSegment(p, v2, v3);
	float flV34 = DistanceToLineSegment(p, v3, v4);
	float flV41 = DistanceToLineSegment(p, v4, v1);

	flClosestPoint = smaller(flClosestPoint, smaller(flV12, smaller(flV23, smaller(flV34, flV41))));

	return flClosestPoint;
}

inline float DistanceToPolygon(const Vector& p, tvector<Vector>& v, Vector n)
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

inline float DistanceToLineSqr(const Vector& p, const Vector& v1, const Vector& v2)
{
	Vector v = v2 - v1;
	Vector w = p - v1;

	float c1 = w.Dot(v);
	float c2 = v.Dot(v);

	float b = c1/c2;

	Vector vb = v1 + v*b;
	return (vb - p).LengthSqr();
}

inline float DistanceToAABBSqr(const Vector& p, const AABB& aabb)
{
	if (p.x < aabb.m_vecMins.x)
	{
		if (p.y < aabb.m_vecMins.y)
		{
			if (p.z < aabb.m_vecMins.z)
				return p.DistanceSqr(aabb.m_vecMins);
			else if (p.z > aabb.m_vecMaxs.z)
				return p.DistanceSqr(Vector(aabb.m_vecMins.x, aabb.m_vecMins.y, aabb.m_vecMaxs.z));
			else
				return DistanceToLineSqr(p, aabb.m_vecMins, Vector(aabb.m_vecMins.x, aabb.m_vecMins.y, aabb.m_vecMaxs.z));
		}
		else if (p.y > aabb.m_vecMaxs.y)
		{
			if (p.z < aabb.m_vecMins.z)
				return p.DistanceSqr(Vector(aabb.m_vecMins.x, aabb.m_vecMaxs.y, aabb.m_vecMins.z));
			else if (p.z > aabb.m_vecMaxs.z)
				return p.DistanceSqr(Vector(aabb.m_vecMins.x, aabb.m_vecMaxs.y, aabb.m_vecMaxs.z));
			else
				return DistanceToLineSqr(p, Vector(aabb.m_vecMins.x, aabb.m_vecMaxs.y, aabb.m_vecMins.z), Vector(aabb.m_vecMins.x, aabb.m_vecMaxs.y, aabb.m_vecMaxs.z));
		}
		else
		{
			if (p.z < aabb.m_vecMins.z)
				return DistanceToLineSqr(p, aabb.m_vecMins, Vector(aabb.m_vecMins.x, aabb.m_vecMaxs.y, aabb.m_vecMins.z));
			else if (p.z > aabb.m_vecMaxs.z)
				return DistanceToLineSqr(p, Vector(aabb.m_vecMins.x, aabb.m_vecMins.y, aabb.m_vecMaxs.z), Vector(aabb.m_vecMins.x, aabb.m_vecMaxs.y, aabb.m_vecMaxs.z));
			else
				return DistanceToPlaneSqr(p, aabb.m_vecMins, Vector(-1, 0, 0));
		}
	}
	else if (p.x > aabb.m_vecMaxs.x)
	{
		if (p.y < aabb.m_vecMins.y)
		{
			if (p.z < aabb.m_vecMins.z)
				return p.DistanceSqr(Vector(aabb.m_vecMaxs.x, aabb.m_vecMins.y, aabb.m_vecMins.z));
			else if (p.z > aabb.m_vecMaxs.z)
				return p.DistanceSqr(Vector(aabb.m_vecMaxs.x, aabb.m_vecMins.y, aabb.m_vecMaxs.z));
			else
				return DistanceToLineSqr(p, Vector(aabb.m_vecMaxs.x, aabb.m_vecMins.y, aabb.m_vecMins.z), Vector(aabb.m_vecMaxs.x, aabb.m_vecMins.y, aabb.m_vecMaxs.z));
		}
		else if (p.y > aabb.m_vecMaxs.y)
		{
			if (p.z < aabb.m_vecMins.z)
				return p.DistanceSqr(Vector(aabb.m_vecMaxs.x, aabb.m_vecMaxs.y, aabb.m_vecMins.z));
			else if (p.z > aabb.m_vecMaxs.z)
				return p.DistanceSqr(Vector(aabb.m_vecMaxs.x, aabb.m_vecMaxs.y, aabb.m_vecMaxs.z));
			else
				return DistanceToLineSqr(p, Vector(aabb.m_vecMaxs.x, aabb.m_vecMaxs.y, aabb.m_vecMins.z), Vector(aabb.m_vecMaxs.x, aabb.m_vecMaxs.y, aabb.m_vecMaxs.z));
		}
		else
		{
			if (p.z < aabb.m_vecMins.z)
				return DistanceToLineSqr(p, Vector(aabb.m_vecMaxs.x, aabb.m_vecMins.y, aabb.m_vecMins.z), Vector(aabb.m_vecMaxs.x, aabb.m_vecMaxs.y, aabb.m_vecMins.z));
			else if (p.z > aabb.m_vecMaxs.z)
				return DistanceToLineSqr(p, Vector(aabb.m_vecMaxs.x, aabb.m_vecMins.y, aabb.m_vecMaxs.z), Vector(aabb.m_vecMaxs.x, aabb.m_vecMaxs.y, aabb.m_vecMaxs.z));
			else
				return DistanceToPlaneSqr(p, aabb.m_vecMaxs, Vector(1, 0, 0));
		}
	}
	else
	{
		if (p.y < aabb.m_vecMins.y)
		{
			if (p.z < aabb.m_vecMins.z)
				return DistanceToLineSqr(p, aabb.m_vecMins, Vector(aabb.m_vecMaxs.x, aabb.m_vecMins.y, aabb.m_vecMins.z));
			else if (p.z > aabb.m_vecMaxs.z)
				return DistanceToLineSqr(p, Vector(aabb.m_vecMins.x, aabb.m_vecMins.y, aabb.m_vecMaxs.z), Vector(aabb.m_vecMaxs.x, aabb.m_vecMins.y, aabb.m_vecMaxs.z));
			else
				return DistanceToPlaneSqr(p, aabb.m_vecMins, Vector(0, -1, 0));
		}
		else if (p.y > aabb.m_vecMaxs.y)
		{
			if (p.z < aabb.m_vecMins.z)
				return DistanceToLineSqr(p, Vector(aabb.m_vecMins.x, aabb.m_vecMaxs.y, aabb.m_vecMins.z), Vector(aabb.m_vecMaxs.x, aabb.m_vecMaxs.y, aabb.m_vecMins.z));
			else if (p.z > aabb.m_vecMaxs.z)
				return DistanceToLineSqr(p, Vector(aabb.m_vecMins.x, aabb.m_vecMaxs.y, aabb.m_vecMaxs.z), aabb.m_vecMaxs);
			else
				return DistanceToPlaneSqr(p, aabb.m_vecMaxs, Vector(0, 1, 0));
		}
		else
		{
			if (p.z < aabb.m_vecMins.z)
				return DistanceToPlaneSqr(p, aabb.m_vecMins, Vector(0, 0, -1));
			else if (p.z > aabb.m_vecMaxs.z)
				return DistanceToPlaneSqr(p, aabb.m_vecMaxs, Vector(0, 0, 1));
			else
				return 0;
		}
	}
}

inline float TriangleArea(const Vector& a, const Vector& b, const Vector& c)
{
	return (a-b).Cross(a-c).Length()/2;
}

inline bool RayIntersectsTriangle(const Ray& vecRay, const Vector& v0, const Vector& v1, const Vector& v2, Vector* pvecHit = NULL)
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

inline bool RayIntersectsPlane(const Ray& vecRay, const Vector& v0, const Vector& v1, const Vector& v2, Vector* pvecHit = NULL)
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
		return false;		// Ray goes away from the plane

	Vector vecPoint = vecRay.m_vecPos + vecRay.m_vecDir*r;
	if (pvecHit)
		*pvecHit = vecPoint;

	return true;
}

inline bool RayIntersectsPlane(const Ray& vecRay, const Vector& p, const Vector& n, Vector* pvecHit = NULL)
{
	Vector w0 = vecRay.m_vecPos - p;

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
		return false;		// Ray goes away from the plane

	Vector vecPoint = vecRay.m_vecPos + vecRay.m_vecDir*r;
	if (pvecHit)
		*pvecHit = vecPoint;

	return true;
}

inline bool RayIntersectsQuad(const Ray& vecRay, const Vector& v0, const Vector& v1, const Vector& v2, const Vector& v3, Vector* pvecHit = NULL)
{
	if (RayIntersectsTriangle(vecRay, v0, v1, v2, pvecHit))
		return true;

	else return RayIntersectsTriangle(vecRay, v0, v2, v3, pvecHit);
}

inline bool ClipRay(float flMin, float flMax, float a, float d, float& tmin, float& tmax)
{
	const float flEpsilon = 1e-5f;

	if (fabs(d) < flEpsilon)
		return a <= flMax && a >= flMin;

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

inline bool RayIntersectsAABB(const Ray& r, const AABB& b, Vector& vecIntersection)
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

	vecIntersection = r.m_vecPos + r.m_vecDir * tmin;

	return true;
}

inline bool RayIntersectsAABB(const Ray& r, const AABB& b)
{
	Vector vecIntersection;
	return RayIntersectsAABB(r, b, vecIntersection);
}

template <class F>
inline bool ClipSegment(F flMin, F flMax, F a, F b, F d, F& tmin, F& tmax)
{
	const F flEpsilon = 1e-5f;

	if (fabs(d) < flEpsilon)
	{
		if (d >= 0.0f)
			return !(b < flMin || a > flMax);
		else
			return !(a < flMin || b > flMax);
	}

	F umin = (flMin - a)/d;
	F umax = (flMax - a)/d;

	if (umin > umax)
	{
		F yar = umin;
		umin = umax;
		umax = yar;
	}

	if (umax < tmin || umin > tmax)
		return false;

	tmin = (umin>tmin)?umin:tmin;
	tmax = (umax<tmax)?umax:tmax;

	return (tmax>tmin);
}

template <class F>
inline bool SegmentIntersectsAABB(const TemplateVector<F>& v1, const TemplateVector<F>& v2, const TemplateAABB<F>& b, CCollisionResult& tr)
{
	F tmin = 0;
	F tmax = 1;

	TemplateVector<F> vecDir = v2 - v1;

	if (!ClipSegment(b.m_vecMins.x, b.m_vecMaxs.x, v1.x, v2.x, vecDir.x, tmin, tmax))
		return false;

	if (!ClipSegment(b.m_vecMins.y, b.m_vecMaxs.y, v1.y, v2.y, vecDir.y, tmin, tmax))
		return false;

	if (!ClipSegment(b.m_vecMins.z, b.m_vecMaxs.z, v1.z, v2.z, vecDir.z, tmin, tmax))
		return false;

	F flFraction = (tmin < 0) ? 0 : tmin;
	if (flFraction > tr.flFraction)
		return false;

	tr.bHit = true;
	tr.bStartInside = tmin < 0;
	tr.flFraction = (float)flFraction;
	//tr.vecHit not set and not used.
	//tr.vecNormal not set and not used.

	return true;
}

template <class F>
inline bool LineSegmentIntersectsTriangle(const TemplateVector<F>& s0, const TemplateVector<F>& s1, const TemplateVector<F>& v0, const TemplateVector<F>& v1, const TemplateVector<F>& v2, CCollisionResult& tr)
{
	TemplateVector<F> u = v1 - v0;
	TemplateVector<F> v = v2 - v0;
	Vector n = u.Cross(v);

	TemplateVector<F> w0 = s0 - v0;

	F a = -n.Dot(w0);
	F b = n.Dot(s1-s0);

	F ep = 1e-4f;

	if (fabs(b) < ep)
	{
		if (a == 0)			// Segment is parallel
			return true;	// Segment is inside plane
		else
			return false;	// Segment is somewhere else
	}

	F r = a/b;
	if (r < 0)
		return false;		// Segment goes away from the triangle
	if (r > 1)
		return false;		// Segment goes away from the triangle

	if (tr.flFraction < r)
		return false;

	TemplateVector<F> vecPoint = s0 + (s1-s0)*r;

	F uu = u.Dot(u);
	F uv = u.Dot(v);
	F vv = v.Dot(v);
	TemplateVector<F> w = vecPoint - v0;
	F wu = w.Dot(u);
	F wv = w.Dot(v);

	F D = uv * uv - uu * vv;

	F s, t;

	s = (uv * wv - vv * wu) / D;
	if (s <= ep || s >= 1)		// Intersection point is outside the triangle
		return false;

	t = (uv * wu - uu * wv) / D;
	if (t <= ep || (s+t) >= 1)	// Intersection point is outside the triangle
		return false;

	tr.bHit = true;
	tr.flFraction = r;
	tr.vecHit = vecPoint;
	tr.vecNormal = (v1-v0).Normalized().Cross((v2-v0).Normalized()).Normalized();

	return true;
}

inline bool LineSegmentIntersectsSphere(const Vector& v1, const Vector& v2, const Vector& s, float flRadius, Vector& vecPoint, Vector& vecNormal)
{
	Vector vecLine = v2 - v1;
	Vector vecSphere = v1 - s;

	if (vecLine.LengthSqr() == 0)
	{
		if (vecSphere.LengthSqr() < flRadius*flRadius)
		{
			vecPoint = v1;
			vecNormal = vecSphere.Normalized();
			return true;
		}
		else
			return false;
	}

	float flA = vecLine.LengthSqr();
	float flB = 2 * vecSphere.Dot(vecLine);
	float flC1 = s.LengthSqr() + v1.LengthSqr();
	float flC2 = (s.Dot(v1)*2);
	float flC = flC1 - flC2 - flRadius*flRadius;

	float flBB4AC = flB*flB - 4*flA*flC;
	if (flBB4AC < 0)
		return false;

	float flSqrt = sqrt(flBB4AC);
	float flPlus = (-flB + flSqrt)/(2*flA);
	float flMinus = (-flB - flSqrt)/(2*flA);

	bool bPlusBelow0 = flPlus < 0;
	bool bMinusBelow0 = flMinus < 0;
	bool bPlusAbove1 = flPlus > 1;
	bool bMinusAbove1 = flMinus > 1;

	// If both are above 1 or below 0, then we're not touching the sphere.
	if (bMinusBelow0 && bPlusBelow0 || bPlusAbove1 && bMinusAbove1)
		return false;

	if (bMinusBelow0 && bPlusAbove1)
	{
		// We are inside the sphere.
		vecPoint = v1;
		vecNormal = (v1 - s).Normalized();
		return true;
	}

	if (bMinusAbove1 && bPlusBelow0)
	{
		// We are inside the sphere. Is this even possible? I dunno. I'm putting an assert here to see.
		// If it's still here later that means no.
		TAssertNoMsg(false);
		vecPoint = v1;
		vecNormal = (v1 - s).Normalized();
		return true;
	}

	// If flPlus is below 1 and flMinus is below 0 that means we started our trace inside the sphere and we're heading out.
	// Don't intersect with the sphere in this case so that things on the inside can get out without getting stuck.
	if (bMinusBelow0 && !bPlusAbove1)
		return false;

	// So at this point, flMinus is between 0 and 1, and flPlus is above 1.
	// In any other case, we intersect with the sphere and we use the flMinus value as the intersection point.
	float flDistance = vecLine.Length();
	Vector vecDirection = vecLine / flDistance;

	vecPoint = v1 + vecDirection * (flDistance * flMinus);

	// Oftentimes we are slightly stuck inside the sphere. Pull us out a little bit.
	Vector vecDifference = vecPoint - s;
	float flDifferenceLength = vecDifference.Length();
	vecNormal = vecDifference / flDifferenceLength;
	if (flDifferenceLength < flRadius)
		vecPoint += vecNormal * ((flRadius - flDifferenceLength) + 0.00001f);
	TAssertNoMsg((vecPoint - s).LengthSqr() >= flRadius*flRadius);

	return true;
}

template <class F>
inline bool PointInsideAABB( const TemplateAABB<F>& oBox, const TemplateVector<F>& v )
{
	const F flEpsilon = 1e-4f;

	for (size_t i = 0; i < 3; i++)
	{
		F flVI = v[i];

		if (flVI < oBox.m_vecMins[i] - flEpsilon || flVI > oBox.m_vecMaxs[i] + flEpsilon)
			return false;
	}

	return true;
}

template <class F>
inline bool	TriangleIntersectsAABB( const TemplateAABB<F>& oBox, const TemplateVector<F>& v0, const TemplateVector<F>& v1, const TemplateVector<F>& v2)
{
	// Trivial case rejection: If any of the points are inside the box, return true immediately.
	if (oBox.Inside(v0))
		return true;
	if (oBox.Inside(v1))
		return true;
	if (oBox.Inside(v2))
		return true;

	size_t i;

	// Trivial case rejection: If all three points are on one side of the box then the triangle must be outside of it.
	for (i = 0; i < 3; i++)
	{
		F flBoxMax = oBox.m_vecMaxs[i];
		F flBoxMin = oBox.m_vecMins[i];

		F flV0 = v0[i];
		F flV1 = v1[i];
		F flV2 = v2[i];

		if (flV0 > flBoxMax && flV1 > flBoxMax && flV2 > flBoxMax)
			return false;
		if (flV0 < flBoxMin && flV1 < flBoxMin && flV2 < flBoxMin)
			return false;
	}

	CCollisionResult tr1;
	if (SegmentIntersectsAABB(v0, v1, oBox, tr1))
		return true;

	CCollisionResult tr2;
	if (SegmentIntersectsAABB(v1, v2, oBox, tr2))
		return true;

	CCollisionResult tr3;
	if (SegmentIntersectsAABB(v0, v2, oBox, tr3))
		return true;

	TemplateVector<F> c0 = oBox.m_vecMins;
	TemplateVector<F> c1 = TemplateVector<F>(oBox.m_vecMins.x, oBox.m_vecMins.y, oBox.m_vecMaxs.z);
	TemplateVector<F> c2 = TemplateVector<F>(oBox.m_vecMins.x, oBox.m_vecMaxs.y, oBox.m_vecMins.z);
	TemplateVector<F> c3 = TemplateVector<F>(oBox.m_vecMins.x, oBox.m_vecMaxs.y, oBox.m_vecMaxs.z);
	TemplateVector<F> c4 = TemplateVector<F>(oBox.m_vecMaxs.x, oBox.m_vecMins.y, oBox.m_vecMins.z);
	TemplateVector<F> c5 = TemplateVector<F>(oBox.m_vecMaxs.x, oBox.m_vecMins.y, oBox.m_vecMaxs.z);
	TemplateVector<F> c6 = TemplateVector<F>(oBox.m_vecMaxs.x, oBox.m_vecMaxs.y, oBox.m_vecMins.z);
	TemplateVector<F> c7 = oBox.m_vecMaxs;

	// Build a list of line segments in the cube to test against the triangle.
	TemplateVector<F> aLines[32];

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
	CCollisionResult tr;
	for (i = 0; i < 32; i+=2)
	{
		if (LineSegmentIntersectsTriangle(aLines[i], aLines[i+1], v0, v1, v2, tr))
			return true;
	}

	return false;
}

inline bool	ConvexHullIntersectsAABB(const AABB& oBox, const tvector<Vector>& avecPoints, const tvector<size_t>& aiTriangles)
{
	TAssertNoMsg(aiTriangles.size()%3 == 0);

	Vector vecCenter = oBox.Center();
	Vector n;

	for (size_t i = 0; i < aiTriangles.size(); i += 3)
	{
		const Vector& v1 = avecPoints[aiTriangles[i]];
		const Vector& v2 = avecPoints[aiTriangles[i+1]];
		const Vector& v3 = avecPoints[aiTriangles[i+2]];

		n = (v2-v1).Cross(v3-v1).Normalized();

		if (n.Dot(vecCenter-v1) < 0)
			continue;

		if (!TriangleIntersectsAABB(oBox, v1, v2, v3))
			return false;
	}

	return true;
}

inline size_t FindEar(const tvector<Vector>& avecPoints)
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
	float flZ = vecDistance.z;
	vecDistance.z = 0;
	float flX = vecDistance.Length();

	if (flX == 0)
	{
		flTime = 0;
		vecForce = Vector(0, 0, 0);
		return;
	}

	float flA = flCurve;
	float flH = (flX*flX - (flZ/flA))/(2*flX);
	float flK = -flA*flH*flH;

	float flForce = sqrt(2*-flGravity*flK);

	float flTimeToVertex = -flForce/flGravity;
	float flTimeToLand = sqrt(2*-(flK-flZ)/flGravity);

	flTime = flTimeToVertex + flTimeToLand;

	Vector vecDirection = vecDistance.Normalized() * flX / flTime;
	vecDirection.z = flForce;

	vecForce = vecDirection;
}

#endif
