#ifndef LW_VECTOR_H
#define LW_VECTOR_H

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
			Vector(class Color c);
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
	float	Length2D() const;
	float	Length2DSqr() const;
	void	Normalize();
	Vector	Normalized() const;

	float	Distance(const Vector& v) const;
	float	DistanceSqr(const Vector& v) const;

	float	Dot(const Vector& v) const;
	Vector	Cross(const Vector& v) const;

	operator float*()
	{
		return(&x);
	}

	operator const float*() const
	{
		return(&x);
	}

	float	operator[](int i) const;
	float&	operator[](int i);

	float	operator[](size_t i) const;
	float&	operator[](size_t i);

	unit_t	x, y, z;
};

#include <color.h>

inline Vector::Vector()
	: x(0), y(0), z(0)
{
}

inline Vector::Vector(Color c)
{
	x = (float)c.r()/255.0f;
	y = (float)c.g()/255.0f;
	z = (float)c.b()/255.0f;
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

inline float Vector::Length2D() const
{
	return sqrt(x*x + z*z);
}

inline float Vector::Length2DSqr() const
{
	return x*x + z*z;
}

inline void Vector::Normalize()
{
	float flLength = Length();
	if (!flLength)
		*this=Vector(0,0,1);
	else
		*this/=flLength;
}

inline Vector Vector::Normalized() const
{
	float flLength = Length();
	if (!flLength)
		return Vector(0,0,1);
	else
		return *this/flLength;
}

inline float Vector::Distance(const Vector& v) const
{
	return (*this - v).Length();
}

inline float Vector::DistanceSqr(const Vector& v) const
{
	return (*this - v).LengthSqr();
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
	return (&x)[i];
}

inline float Vector::operator[](int i) const
{
	return (&x)[i];
}

inline float& Vector::operator[](size_t i)
{
	return (&x)[i];
}

inline float Vector::operator[](size_t i) const
{
	return (&x)[i];
}

// Euler angles
class EAngle
{
public:
			EAngle();
			EAngle(unit_t x, unit_t y, unit_t z);
			EAngle(unit_t* xyz);

	EAngle	operator+(const EAngle& v) const;
	EAngle	operator-(const EAngle& v) const;

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

inline EAngle EAngle::operator+(const EAngle& v) const
{
	return EAngle(p+v.p, y+v.y, r+v.r);
}

inline EAngle EAngle::operator-(const EAngle& v) const
{
	return EAngle(p-v.p, y-v.y, r-v.r);
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

	angReturn.p = atan2(vecForward.y, sqrt(vecForward.x*vecForward.x + vecForward.z*vecForward.z)) * 180/M_PI;
	angReturn.y = atan2(vecForward.z, vecForward.x) * 180/M_PI;

	return angReturn;
}

class Vector2D
{
public:
				Vector2D();
				Vector2D(unit_t x, unit_t y);
				Vector2D(Vector v);

public:
	Vector2D	operator+(const Vector2D& v) const;
	Vector2D	operator-(const Vector2D& v) const;
	Vector2D	operator*(float s) const;
	Vector2D	operator/(float s) const;

	operator float*()
	{
		return(&x);
	}

	unit_t	x, y;
};

inline Vector2D::Vector2D()
	: x(0), y(0)
{
}

inline Vector2D::Vector2D(unit_t X, unit_t Y)
	: x(X), y(Y)
{
}

inline Vector2D::Vector2D(Vector v)
	: x(v.x), y(v.y)
{
}

inline Vector2D Vector2D::operator+(const Vector2D& v) const
{
	return Vector2D(x+v.x, y+v.y);
}

inline Vector2D Vector2D::operator-(const Vector2D& v) const
{
	return Vector2D(x-v.x, y-v.y);
}

inline Vector2D Vector2D::operator*(float s) const
{
	return Vector2D(x*s, y*s);
}

inline Vector2D Vector2D::operator/(float s) const
{
	return Vector2D(x/s, y/s);
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