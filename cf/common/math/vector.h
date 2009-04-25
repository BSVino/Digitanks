#ifndef CF_VECTOR_H
#define CF_VECTOR_H

typedef float unit_t;

// Try to keep this class compatible with Valve's vector.h
class Vector
{
public:
	Vector(unit_t x, unit_t y, unit_t z);

	unit_t	x;
	unit_t	y;
	unit_t	z;
};

inline Vector::Vector(unit_t X, unit_t Y, unit_t Z)
	: x(X), y(Y), z(Z)
{
}

#endif