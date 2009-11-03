#ifndef CF_VECTOR_H
#define CF_VECTOR_H

#include <math.h>

typedef float unit_t;

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

#endif