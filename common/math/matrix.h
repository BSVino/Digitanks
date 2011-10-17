#ifndef LW_MATRIX_H
#define LW_MATRIX_H

#include <vector.h>

// The red pill

class Matrix4x4
{
public:
				Matrix4x4() { Identity(); }
				Matrix4x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);
				Matrix4x4(const Matrix4x4& m);
				Matrix4x4(float* aflValues);

	void		Identity();
	void		Init(const Matrix4x4& m);
	void		Init(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);

	bool		IsIdentity() const;

	Matrix4x4	Transposed() const;

	// Simple matrix operations
	Matrix4x4	operator*(float f) const;
	Matrix4x4	operator+(const Matrix4x4& m) const;

	// Set a transformation
	void		SetTranslation(const Vector& vecPos);
	void		SetRotation(const EAngle& angDir);
	void		SetRotation(float flAngle, const Vector& vecAxis);
	void		SetOrientation(const Vector& vecDir);
	void		SetScale(const Vector& vecScale);
	void		SetReflection(const Vector& vecPlane);

	// Add a translation
	Matrix4x4	operator+=(const Vector& v);
	// Add a rotation
	Matrix4x4	operator+=(const EAngle& a);

	// Add a transformation
	Matrix4x4	operator*(const Matrix4x4& t);
	Matrix4x4	operator*=(const Matrix4x4& t);

	// Add a transformation
	Matrix4x4	AddScale(const Vector& vecScale);

	Vector		GetTranslation();
	EAngle		GetAngles();

	// Transform a vector
	Vector		operator*(const Vector& v) const;

	Vector4D	GetRow(int i);
	Vector4D	GetColumn(int i);
	void		SetColumn(int i, Vector4D vecColumn);
	void		SetColumn(int i, Vector vecColumn);

	void		InvertTR();

	float		Trace();
	Vector		RotationAxis();

	operator float*()
	{
		return(&m[0][0]);
	}

	float		m[4][4];
};

inline Matrix4x4::Matrix4x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
	Init(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33);
}

inline Matrix4x4::Matrix4x4(const Matrix4x4& i)
{
	m[0][0] = i.m[0][0]; m[0][1] = i.m[0][1]; m[0][2] = i.m[0][2]; m[0][3] = i.m[0][3];
	m[1][0] = i.m[1][0]; m[1][1] = i.m[1][1]; m[1][2] = i.m[1][2]; m[1][3] = i.m[1][3];
	m[2][0] = i.m[2][0]; m[2][1] = i.m[2][1]; m[2][2] = i.m[2][2]; m[2][3] = i.m[2][3];
	m[3][0] = i.m[3][0]; m[3][1] = i.m[3][1]; m[3][2] = i.m[3][2]; m[3][3] = i.m[3][3];
}

inline Matrix4x4::Matrix4x4(float* aflValues)
{
	memcpy(&m[0][0], aflValues, sizeof(float)*16);
}

inline void Matrix4x4::Identity()
{
	memset(this, 0, sizeof(Matrix4x4));

	m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
}

inline void Matrix4x4::Init(const Matrix4x4& i)
{
	memcpy(&m[0][0], &i.m[0][0], sizeof(float)*16);
}

inline void Matrix4x4::Init(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
	m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
	m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
	m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
	m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
}

inline bool Matrix4x4::IsIdentity() const
{
	if (!(m[0][0] == 1 && m[1][1] == 1 && m[2][2] == 1 && m[3][3] == 1))
		return false;

	if (!(m[0][1] == 0 && m[0][2] == 0 && m[0][3] == 0))
		return false;

	if (!(m[1][0] == 0 && m[1][2] == 0 && m[1][3] == 0))
		return false;

	if (!(m[2][0] == 0 && m[2][1] == 0 && m[2][3] == 0))
		return false;

	if (!(m[3][0] == 0 && m[3][1] == 0 && m[3][2] == 0))
		return false;

	return true;
}

inline Matrix4x4 Matrix4x4::Transposed() const
{
	Matrix4x4 r;
	r.m[0][0] = m[0][0]; r.m[1][0] = m[0][1]; r.m[2][0] = m[0][2]; r.m[3][0] = m[0][3];
	r.m[0][1] = m[1][0]; r.m[1][1] = m[1][1]; r.m[2][1] = m[1][2]; r.m[3][1] = m[1][3];
	r.m[0][2] = m[2][0]; r.m[1][2] = m[2][1]; r.m[2][2] = m[2][2]; r.m[3][2] = m[2][3];
	r.m[0][3] = m[3][0]; r.m[1][3] = m[3][1]; r.m[2][3] = m[3][2]; r.m[3][3] = m[3][3];
	return r;
}

inline Matrix4x4 Matrix4x4::operator*(float f) const
{
	Matrix4x4 r;

	r.m[0][0] = m[0][0]*f;
	r.m[0][1] = m[0][1]*f;
	r.m[0][2] = m[0][2]*f;
	r.m[0][3] = m[0][3]*f;

	r.m[1][0] = m[1][0]*f;
	r.m[1][1] = m[1][1]*f;
	r.m[1][2] = m[1][2]*f;
	r.m[1][3] = m[1][3]*f;

	r.m[2][0] = m[2][0]*f;
	r.m[2][1] = m[2][1]*f;
	r.m[2][2] = m[2][2]*f;
	r.m[2][3] = m[2][3]*f;

	r.m[3][0] = m[3][0]*f;
	r.m[3][1] = m[3][1]*f;
	r.m[3][2] = m[3][2]*f;
	r.m[3][3] = m[3][3]*f;

	return r;
}

inline Matrix4x4 Matrix4x4::operator+(const Matrix4x4& t) const
{
	Matrix4x4 r;

	r.m[0][0] = m[0][0]+t.m[0][0];
	r.m[0][1] = m[0][1]+t.m[0][1];
	r.m[0][2] = m[0][2]+t.m[0][2];
	r.m[0][3] = m[0][3]+t.m[0][3];

	r.m[1][0] = m[1][0]+t.m[1][0];
	r.m[1][1] = m[1][1]+t.m[1][1];
	r.m[1][2] = m[1][2]+t.m[1][2];
	r.m[1][3] = m[1][3]+t.m[1][3];

	r.m[2][0] = m[2][0]+t.m[2][0];
	r.m[2][1] = m[2][1]+t.m[2][1];
	r.m[2][2] = m[2][2]+t.m[2][2];
	r.m[2][3] = m[2][3]+t.m[2][3];

	r.m[3][0] = m[3][0]+t.m[3][0];
	r.m[3][1] = m[3][1]+t.m[3][1];
	r.m[3][2] = m[3][2]+t.m[3][2];
	r.m[3][3] = m[3][3]+t.m[3][3];

	return r;
}

inline void Matrix4x4::SetTranslation(const Vector& vecPos)
{
	m[0][3] = vecPos.x;
	m[1][3] = vecPos.y;
	m[2][3] = vecPos.z;
}

inline void Matrix4x4::SetRotation(const EAngle& angDir)
{
	float sp = sin(angDir.p * M_PI/180);
	float sy = sin(angDir.y * M_PI/180);
	float sr = sin(angDir.r * M_PI/180);
	float cp = cos(angDir.p * M_PI/180);
	float cy = cos(angDir.y * M_PI/180);
	float cr = cos(angDir.r * M_PI/180);

	m[0][0] = cy*cp;
	m[0][1] = -sr*sy*cp-cr*sp;
	m[0][2] = -cr*sy*cp+sr*sp;
	m[1][0] = cy*sp;
	m[1][1] = -sr*sy*sp+cr*cp;
	m[1][2] = -cr*sy*sp-sr*cp;
	m[2][0] = sy;
	m[2][1] = sr*cy;
	m[2][2] = cr*cy;
}

inline void Matrix4x4::SetRotation(float flAngle, const Vector& v)
{
	float c = cos(flAngle*M_PI/180);
	float s = sin(flAngle*M_PI/180);
	float t = 1-c;

	float xyt = v.x*v.y*t;
	float zs = v.z*s;
	float xzt = v.x*v.z*t;
	float ys = v.y*s;
	float yzt = v.y*v.z*t;
	float xs = v.x*s;

	Init(c + v.x*v.x*t, xyt - zs, xzt - ys, 0, xyt + zs, c + v.y*v.y*t, yzt - xs, 0, xzt - ys, yzt + xs, c + v.z*v.z*t, 0, 0, 0, 0, 1);
}

inline void Matrix4x4::SetOrientation(const Vector& vecDir)
{
	Vector vecRight, vecUp;
	vecUp = Vector(0, 1, 0);
	if (vecDir.DistanceSqr(vecUp) > 0.001f && vecDir.DistanceSqr(-vecUp) > 0.001f)
		vecRight = vecDir.Cross(vecUp).Normalized();
	else
		vecRight = Vector(1, 0, 0);

	vecUp = vecRight.Cross(vecDir).Normalized();

	m[0][0] = vecRight.x;
	m[1][0] = vecRight.y;
	m[2][0] = vecRight.z; 
	m[0][1] = vecUp.x;
	m[1][1] = vecUp.y;
	m[2][1] = vecUp.z; 
	m[0][2] = -vecDir.x;
	m[1][2] = -vecDir.y;
	m[2][2] = -vecDir.z; 

	m[3][0] = m[3][1] = m[3][2] = 0.0f;
	m[0][3] = m[1][3] = m[2][3] = 0.0f;
	m[3][3] = 1.0f;
}

inline void Matrix4x4::SetScale(const Vector& vecScale)
{
	m[0][0] = vecScale.x;
	m[1][1] = vecScale.y;
	m[2][2] = vecScale.z;
}

inline void Matrix4x4::SetReflection(const Vector& vecPlane)
{
	m[0][0] = 1 - 2 * vecPlane.x * vecPlane.x;
	m[1][1] = 1 - 2 * vecPlane.y * vecPlane.y;
	m[2][2] = 1 - 2 * vecPlane.z * vecPlane.z;
	m[1][0] = m[0][1] = -2 * vecPlane.x * vecPlane.y;
	m[2][0] = m[0][2] = -2 * vecPlane.x * vecPlane.z;
	m[1][2] = m[2][1] = -2 * vecPlane.y * vecPlane.z;
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

inline Matrix4x4 Matrix4x4::operator+=(const EAngle& a)
{
	Matrix4x4 r;
	r.SetRotation(a);
	(*this) *= r;

	return *this;
}

inline Matrix4x4 Matrix4x4::operator*(const Matrix4x4& t)
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

	return r;
}

inline Matrix4x4 Matrix4x4::operator*=(const Matrix4x4& t)
{
	Matrix4x4 r;
	r.Init(*this);

	Init(r*t);

	return *this;
}

inline Matrix4x4 Matrix4x4::AddScale(const Vector& vecScale)
{
	Matrix4x4 r;
	r.SetScale(vecScale);
	(*this) *= r;

	return *this;
}

inline Vector Matrix4x4::GetTranslation()
{
	return Vector(m[0][3], m[1][3], m[2][3]);
}

inline EAngle Matrix4x4::GetAngles()
{
	// Clamp to [-1, 1] looping
	float flPitch = fmod(m[0][1], 2);
	if (flPitch > 1)
		flPitch -= 2;
	else if (flPitch < -1)
		flPitch += 2;

	return EAngle(asin(flPitch) * 180/M_PI, atan2(-m[0][2], m[0][0]) * 180/M_PI, atan2(-m[2][1], m[1][1]) * 180/M_PI);
}

inline Vector Matrix4x4::operator*(const Vector& v) const
{
	Vector vecResult;
	vecResult.x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3];
	vecResult.y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3];
	vecResult.z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3];
	return vecResult;
}

inline Vector4D Matrix4x4::GetRow(int i)
{
	return Vector4D(m[i][0], m[i][1], m[i][2], m[i][3]);
}

inline Vector4D Matrix4x4::GetColumn(int i)
{
	return Vector4D(m[0][i], m[1][i], m[2][i], m[3][i]);
}

inline void Matrix4x4::SetColumn(int i, Vector4D vecColumn)
{
	m[0][i] = vecColumn.x;
	m[1][i] = vecColumn.y;
	m[2][i] = vecColumn.z;
	m[3][i] = vecColumn.w;
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

	float fl03 = m[0][3];
	float fl13 = m[1][3];
	float fl23 = m[2][3];

	Init(t);

	SetColumn(3, t*Vector(-fl03, -fl13, -fl23));
}

inline float Matrix4x4::Trace()
{
	return m[0][0] + m[1][1] + m[2][2];
}

inline Vector Matrix4x4::RotationAxis()
{
	return Vector(m[2][1] - m[1][2], m[0][2] - m[2][0], m[1][0] + m[0][1]).Normalized();
}

#endif
