#ifndef CF_MATRIX_H
#define CF_MATRIX_H

#include <vector.h>

// The red pill

class Matrix4x4
{
public:
				Matrix4x4() { Identity(); }

	void		Identity();
	void		Init(Matrix4x4& m);

	// Add a translation
	Matrix4x4	operator+=(const Vector& v);

	// Add a transformation
	Matrix4x4	operator*=(const Matrix4x4& t);

	// Set a rotation
	void		SetRotation(EAngle angDir);
	void		SetOrientation(Vector vecDir);

	// Transform a vector
	Vector		operator*(const Vector& v) const;

	void		SetColumn(int i, Vector vecColumn);

	void		InvertTR();

	float		m[4][4];
};

inline void Matrix4x4::Identity()
{
	m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
	m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
	m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;
	m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
}

inline void Matrix4x4::Init(Matrix4x4& i)
{
	for (int h = 0; h < 3; h++)
		for (int v = 0; v < 3; v++)
			m[h][v] = i.m[h][v];
}

inline Matrix4x4 Matrix4x4::operator+=(const Vector& v)
{
	Matrix4x4 r = *this;
	r.m[0][3] += v.x;
	r.m[1][3] += v.y;
	r.m[2][3] += v.z;

	Init(r);

	return r;
}

inline Matrix4x4 Matrix4x4::operator*=(const Matrix4x4& t)
{
	Matrix4x4 r;

	r.m[0][0] = m[0][0]*t.m[0][0] + m[0][1]*t.m[1][0] + m[0][2]*t.m[2][0] + m[0][3]*t.m[3][0];
	r.m[0][1] = m[0][0]*t.m[0][1] + m[0][1]*t.m[1][1] + m[0][2]*t.m[2][1] + m[0][3]*t.m[3][1];
	r.m[0][2] = m[0][0]*t.m[0][2] + m[0][1]*t.m[1][2] + m[0][2]*t.m[2][2] + m[0][3]*t.m[3][2];
	r.m[0][3] = m[0][0]*t.m[0][3] + m[0][1]*t.m[1][3] + m[0][2]*t.m[2][3] + m[0][3]*t.m[3][3];

	r.m[1][0] = m[1][0]*t.m[0][0] + m[1][1]*t.m[1][0] + m[1][2]*t.m[2][0] + m[1][3]*t.m[3][0];
	r.m[1][1] = m[1][0]*t.m[0][1] + m[1][1]*t.m[1][1] + m[1][2]*t.m[2][1] + m[1][3]*t.m[3][1];
	r.m[1][2] = m[1][0]*t.m[0][2] + m[1][1]*t.m[1][2] + m[1][2]*t.m[2][2] + m[1][3]*t.m[3][2];
	r.m[1][3] = m[1][0]*t.m[0][3] + m[1][1]*t.m[1][3] + m[1][2]*t.m[2][3] + m[1][3]*t.m[3][3];

	r.m[2][0] = m[2][0]*t.m[0][0] + m[2][1]*t.m[1][0] + m[2][2]*t.m[2][0] + m[2][3]*t.m[3][0];
	r.m[2][1] = m[2][0]*t.m[0][1] + m[2][1]*t.m[1][1] + m[2][2]*t.m[2][1] + m[2][3]*t.m[3][1];
	r.m[2][2] = m[2][0]*t.m[0][2] + m[2][1]*t.m[1][2] + m[2][2]*t.m[2][2] + m[2][3]*t.m[3][2];
	r.m[2][3] = m[2][0]*t.m[0][3] + m[2][1]*t.m[1][3] + m[2][2]*t.m[2][3] + m[2][3]*t.m[3][3];

	r.m[3][0] = m[3][0]*t.m[0][0] + m[3][1]*t.m[1][0] + m[3][2]*t.m[2][0] + m[3][3]*t.m[3][0];
	r.m[3][1] = m[3][0]*t.m[0][1] + m[3][1]*t.m[1][1] + m[3][2]*t.m[2][1] + m[3][3]*t.m[3][1];
	r.m[3][2] = m[3][0]*t.m[0][2] + m[3][1]*t.m[1][2] + m[3][2]*t.m[2][2] + m[3][3]*t.m[3][2];
	r.m[3][3] = m[3][0]*t.m[0][3] + m[3][1]*t.m[1][3] + m[3][2]*t.m[2][3] + m[3][3]*t.m[3][3];

	Init(r);

	return r;
}

inline void Matrix4x4::SetRotation(EAngle angDir)
{
	float sr = sin(angDir.r * M_PI/180);
	float sp = sin(angDir.p * M_PI/180);
	float sy = sin(angDir.y * M_PI/180);
	float cr = cos(angDir.r * M_PI/180);
	float cp = cos(angDir.p * M_PI/180);
	float cy = cos(angDir.y * M_PI/180);

	m[0][0] = cp*cy;
	m[0][1] = sr*sp*cy+cr*-sy;
	m[0][2] = (cr*sp*cy+-sr*-sy);
	m[1][0] = cp*sy;
	m[1][1] = sr*sp*sy+cr*cy;
	m[1][2] = (cr*sp*sy+-sr*cy);
	m[2][0] = -sp;
	m[2][1] = sr*cp;
	m[2][2] = cr*cp;
}

inline void Matrix4x4::SetOrientation(Vector vecDir)
{
	EAngle angDir = VectorAngles(vecDir);
	SetRotation(angDir);
}

inline Vector Matrix4x4::operator*(const Vector& v) const
{
	Vector vecResult;
	vecResult.x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3];
	vecResult.y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3];
	vecResult.z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3];
	return vecResult;
}

inline void Matrix4x4::SetColumn(int i, Vector vecColumn)
{
	m[0][i] = vecColumn.x;
	m[1][i] = vecColumn.y;
	m[2][i] = vecColumn.z;
}

// Not a true inversion, only works if the matrix is a translation/rotation matrix.
inline void Matrix4x4::InvertTR()
{
	Matrix4x4 t;

	for (int h = 0; h < 3; h++)
		for (int v = 0; v < 3; v++)
			t.m[h][v] = m[v][h];

	Init(t);

	SetColumn(3, t*Vector(-m[0][3], -m[1][3], -m[2][3]));
}

#endif
