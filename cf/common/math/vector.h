#ifndef CF_VECTOR_H
#define CF_VECTOR_H

#include <math.h>

typedef float unit_t;

#ifndef M_PI
#define M_PI 3.14159265
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

	float	Length();
	float	LengthSqr();
	void	Normalize();
	Vector	Normalized();

	float	Dot(const Vector& v) const;
	Vector	Cross(const Vector& v) const;

	operator float*()
	{
		return(&x);
	}

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

inline float Vector::Length()
{
	return sqrt(x*x + y*y + z*z);
}

inline float Vector::LengthSqr()
{
	return x*x + y*y + z*z;
}

inline void Vector::Normalize()
{
	float flLength = Length();
	if (!flLength)
		*this=Vector(0,0,1);
	else
		*this/=Length();
}

inline Vector Vector::Normalized()
{
	float flLength = Length();
	if (!flLength)
		return Vector(0,0,1);
	else
		return *this/Length();
}

inline float Vector::Dot(const Vector& v) const
{
	return x*v.x + y*v.y + z*v.z;
}

inline Vector Vector::Cross(const Vector& v) const
{
	return Vector(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
}

inline Vector AngleVector(const Vector& v)
{
	Vector vecResult;

	float p = (float)(v.x * (M_PI*2 / 360));
	float y = (float)(v.y * (M_PI*2 / 360));
	float r = (float)(v.z * (M_PI*2 / 360));

	float sp = sin(p);
	float cp = cos(p);
	float sy = sin(y);
	float cy = cos(y);

	vecResult.x = cp*cy;
	vecResult.y = sp;
	vecResult.z = cp*sy;

	return vecResult;
}

inline void AngleVectors(const Vector& v, Vector* pvecF, Vector* pvecR, Vector* pvecU)
{
	float p = (float)(v.x * (M_PI*2 / 360));
	float y = (float)(v.y * (M_PI*2 / 360));
	float r = 0;

	float sp = sin(p);
	float cp = cos(p);
	float sy = sin(y);
	float cy = cos(y);
	float sr = 0;
	float cr = 0;

	if (pvecR || pvecU)
	{
		r = (float)(v.z * (M_PI*2 / 360));
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

#endif