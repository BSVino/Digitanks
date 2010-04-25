#ifndef CF_VECTOR_H
#define CF_VECTOR_H

#include <math.h>

typedef float unit_t;

#ifndef M_PI
#define M_PI 3.14159265f
#endif

// Try to keep this class compatible with Valve's vector.h
class Vector
{
public:
			Vector();
			Vector(unit_t x, unit_t y, unit_t z);
			Vector(unit_t* xyz);

	Vector	operator-(void) const;

	Vector	operator+(const Vector& v) const;
	Vector	operator-(const Vector& v) const;
	Vector	operator*(float s) const;
	Vector	operator/(float s) const;

	void	operator+=(const Vector &v);
	void	operator-=(const Vector &v);
	void	operator*=(float s);
	void	operator/=(float s);

	Vector	operator*(const Vector& v) const;

	friend Vector operator*( float f, const Vector& v )
	{
		return Vector( v.x*f, v.y*f, v.z*f );
	}

	friend Vector operator/( float f, const Vector& v )
	{
		return Vector( f/v.x, f/v.y, f/v.z );
	}

	float	Length() const;
	float	LengthSqr() const;
	void	Normalize();
	Vector	Normalized() const;

	float	Dot(const Vector& v) const;
	Vector	Cross(const Vector& v) const;

	operator float*()
	{
		return(&x);
	}

	float	operator[](int i) const;
	float&	operator[](int i);

	float	operator[](size_t i) const;
	float&	operator[](size_t i);

	unit_t	x, y, z;
};

inline Vector::Vector()
	: x(0), y(0), z(0)
{
}

inline Vector::Vector(unit_t X, unit_t Y, unit_t Z)
	: x(X), y(Y), z(Z)
{
}

inline Vector::Vector(unit_t* xyz)
	: x(*xyz), y(*(xyz+1)), z(*(xyz+2))
{
}

inline Vector Vector::operator-() const
{
	return Vector(-x, -y, -z);
}

inline Vector Vector::operator+(const Vector& v) const
{
	return Vector(x+v.x, y+v.y, z+v.z);
}

inline Vector Vector::operator-(const Vector& v) const
{
	return Vector(x-v.x, y-v.y, z-v.z);
}

inline Vector Vector::operator*(float s) const
{
	return Vector(x*s, y*s, z*s);
}

inline Vector Vector::operator/(float s) const
{
	return Vector(x/s, y/s, z/s);
}

inline void Vector::operator+=(const Vector& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
}

inline void Vector::operator-=(const Vector& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
}

inline void Vector::operator*=(float s)
{
	x *= s;
	y *= s;
	z *= s;
}

inline void Vector::operator/=(float s)
{
	x /= s;
	y /= s;
	z /= s;
}

inline Vector Vector::operator*(const Vector& v) const
{
	return Vector(x*v.x, y*v.y, z*v.z);
}

inline float Vector::Length() const
{
	return sqrt(x*x + y*y + z*z);
}

inline float Vector::LengthSqr() const
{
	return x*x + y*y + z*z;
}

inline void Vector::Normalize()
{
	// Try to save on the sqrt()
	if (fabs(LengthSqr() - 1) < 1e-6)
		return;

	float flLength = Length();
	if (!flLength)
		*this=Vector(0,0,1);
	else
		*this/=flLength;
}

inline Vector Vector::Normalized() const
{
	// Try to save on the sqrt()
	if (fabs(LengthSqr() - 1) < 1e-6)
		return *this;

	float flLength = Length();
	if (!flLength)
		return Vector(0,0,1);
	else
		return *this/flLength;
}

inline float Vector::Dot(const Vector& v) const
{
	return x*v.x + y*v.y + z*v.z;
}

inline Vector Vector::Cross(const Vector& v) const
{
	return Vector(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
}

inline float& Vector::operator[](int i)
{
	return ((float*)this)[i];
}

inline float Vector::operator[](int i) const
{
	return ((float*)this)[i];
}

inline float& Vector::operator[](size_t i)
{
	return ((float*)this)[i];
}

inline float Vector::operator[](size_t i) const
{
	return ((float*)this)[i];
}

// Euler angles
class EAngle
{
public:
			EAngle();
			EAngle(unit_t x, unit_t y, unit_t z);
			EAngle(unit_t* xyz);

	operator float*()
	{
		return(&p);
	}

	unit_t	p, y, r;
};

inline EAngle::EAngle()
	: p(0), y(0), r(0)
{
}

inline EAngle::EAngle(unit_t P, unit_t Y, unit_t R)
	: p(P), y(Y), r(R)
{
}

inline EAngle::EAngle(unit_t* pyr)
	: p(*pyr), y(*(pyr+1)), r(*(pyr+2))
{
}

inline Vector AngleVector(const EAngle& a)
{
	Vector vecResult;

	float p = (float)(a.p * (M_PI*2 / 360));
	float y = (float)(a.y * (M_PI*2 / 360));
	float r = (float)(a.r * (M_PI*2 / 360));

	float sp = sin(p);
	float cp = cos(p);
	float sy = sin(y);
	float cy = cos(y);

	vecResult.x = cp*cy;
	vecResult.y = sp;
	vecResult.z = cp*sy;

	return vecResult;
}

inline void AngleVectors(const EAngle& a, Vector* pvecF, Vector* pvecR, Vector* pvecU)
{
	float p = (float)(a.p * (M_PI*2 / 360));
	float y = (float)(a.y * (M_PI*2 / 360));
	float r = 0;

	float sp = sin(p);
	float cp = cos(p);
	float sy = sin(y);
	float cy = cos(y);
	float sr = 0;
	float cr = 0;

	if (pvecR || pvecU)
	{
		r = (float)(a.r * (M_PI*2 / 360));
		sr = sin(r);
		cr = cos(r);
	}

	if (pvecF)
	{
		pvecF->x = cp*cy;
		pvecF->y = sp;
		pvecF->z = cp*sy;
	}

	if (pvecR)
	{
		pvecR->x = -sr*sp*cy + cr*sy;
		pvecR->y = sr*cp;
		pvecR->z = -sr*sp*sy - cr*cy;
	}

	if (pvecU)
	{
		pvecU->x = cr*sp*cy + sr*sy;
		pvecU->y = -cr*cp;
		pvecU->z = cr*sp*sy - sr*cy;
	}
}

inline EAngle VectorAngles( const Vector& vecForward )
{
	EAngle angReturn(0, 0, 0);

	angReturn.p = atan2(-vecForward.z, sqrt(vecForward.x*vecForward.x + vecForward.y*vecForward.y)) * 180/M_PI;
	angReturn.y = atan2(vecForward.y, vecForward.x) * 180/M_PI;

	return angReturn;
}

class Vector4D
{
public:
			Vector4D();
			Vector4D(unit_t x, unit_t y, unit_t z, unit_t w);

	operator float*()
	{
		return(&x);
	}

	unit_t	x, y, z, w;
};

inline Vector4D::Vector4D()
	: x(0), y(0), z(0), w(0)
{
}

inline Vector4D::Vector4D(unit_t X, unit_t Y, unit_t Z, unit_t W)
	: x(X), y(Y), z(Z), w(W)
{
}

#endif