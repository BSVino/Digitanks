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

#include "matrix.h"

#include "quaternion.h"
#include "common.h"
#include "geometry.h"

Matrix4x4::Matrix4x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
	Init(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33);
}

Matrix4x4::Matrix4x4(const Matrix4x4& i)
{
	m[0][0] = i.m[0][0]; m[0][1] = i.m[0][1]; m[0][2] = i.m[0][2]; m[0][3] = i.m[0][3];
	m[1][0] = i.m[1][0]; m[1][1] = i.m[1][1]; m[1][2] = i.m[1][2]; m[1][3] = i.m[1][3];
	m[2][0] = i.m[2][0]; m[2][1] = i.m[2][1]; m[2][2] = i.m[2][2]; m[2][3] = i.m[2][3];
	m[3][0] = i.m[3][0]; m[3][1] = i.m[3][1]; m[3][2] = i.m[3][2]; m[3][3] = i.m[3][3];
}

Matrix4x4::Matrix4x4(const DoubleMatrix4x4& i)
{
	m[0][0] = (float)i.m[0][0]; m[0][1] = (float)i.m[0][1]; m[0][2] = (float)i.m[0][2]; m[0][3] = (float)i.m[0][3];
	m[1][0] = (float)i.m[1][0]; m[1][1] = (float)i.m[1][1]; m[1][2] = (float)i.m[1][2]; m[1][3] = (float)i.m[1][3];
	m[2][0] = (float)i.m[2][0]; m[2][1] = (float)i.m[2][1]; m[2][2] = (float)i.m[2][2]; m[2][3] = (float)i.m[2][3];
	m[3][0] = (float)i.m[3][0]; m[3][1] = (float)i.m[3][1]; m[3][2] = (float)i.m[3][2]; m[3][3] = (float)i.m[3][3];
}

Matrix4x4::Matrix4x4(float* aflValues)
{
	memcpy(&m[0][0], aflValues, sizeof(float)*16);
}

Matrix4x4::Matrix4x4(const Vector& vecForward, const Vector& vecLeft, const Vector& vecUp, const Vector& vecPosition)
{
	SetForwardVector(vecForward);
	SetLeftVector(vecLeft);
	SetUpVector(vecUp);
	SetTranslation(vecPosition);

	m[0][3] = 0;
	m[1][3] = 0;
	m[2][3] = 0;
	m[3][3] = 1;
}

Matrix4x4::Matrix4x4(const Quaternion& q)
{
	SetRotation(q);
}

Matrix4x4::Matrix4x4(const EAngle& angDirection, const Vector& vecPosition)
{
	SetAngles(angDirection);
	SetTranslation(vecPosition);

	m[0][3] = 0;
	m[1][3] = 0;
	m[2][3] = 0;
	m[3][3] = 1;
}

void Matrix4x4::Identity()
{
	memset(this, 0, sizeof(Matrix4x4));

	m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
}

void Matrix4x4::Init(const Matrix4x4& i)
{
	memcpy(&m[0][0], &i.m[0][0], sizeof(float)*16);
}

void Matrix4x4::Init(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
	m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
	m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
	m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
	m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
}

bool Matrix4x4::IsIdentity() const
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

Matrix4x4 Matrix4x4::Transposed() const
{
	Matrix4x4 r;
	r.m[0][0] = m[0][0]; r.m[1][0] = m[0][1]; r.m[2][0] = m[0][2]; r.m[3][0] = m[0][3];
	r.m[0][1] = m[1][0]; r.m[1][1] = m[1][1]; r.m[2][1] = m[1][2]; r.m[3][1] = m[1][3];
	r.m[0][2] = m[2][0]; r.m[1][2] = m[2][1]; r.m[2][2] = m[2][2]; r.m[3][2] = m[2][3];
	r.m[0][3] = m[3][0]; r.m[1][3] = m[3][1]; r.m[2][3] = m[3][2]; r.m[3][3] = m[3][3];
	return r;
}

Matrix4x4 Matrix4x4::operator*(float f) const
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

Matrix4x4 Matrix4x4::operator+(const Matrix4x4& t) const
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

Matrix4x4 Matrix4x4::operator-(const Matrix4x4& t) const
{
	Matrix4x4 r;

	r.m[0][0] = m[0][0]-t.m[0][0];
	r.m[0][1] = m[0][1]-t.m[0][1];
	r.m[0][2] = m[0][2]-t.m[0][2];
	r.m[0][3] = m[0][3]-t.m[0][3];

	r.m[1][0] = m[1][0]-t.m[1][0];
	r.m[1][1] = m[1][1]-t.m[1][1];
	r.m[1][2] = m[1][2]-t.m[1][2];
	r.m[1][3] = m[1][3]-t.m[1][3];

	r.m[2][0] = m[2][0]-t.m[2][0];
	r.m[2][1] = m[2][1]-t.m[2][1];
	r.m[2][2] = m[2][2]-t.m[2][2];
	r.m[2][3] = m[2][3]-t.m[2][3];

	r.m[3][0] = m[3][0]-t.m[3][0];
	r.m[3][1] = m[3][1]-t.m[3][1];
	r.m[3][2] = m[3][2]-t.m[3][2];
	r.m[3][3] = m[3][3]-t.m[3][3];

	return r;
}

void Matrix4x4::SetTranslation(const Vector& vecPos)
{
	m[3][0] = vecPos.x;
	m[3][1] = vecPos.y;
	m[3][2] = vecPos.z;
}

void Matrix4x4::SetAngles(const EAngle& angDir)
{
	float sp = sin(angDir.p * M_PI/180);
	float sy = sin(-angDir.y * M_PI/180);
	float sr = sin(angDir.r * M_PI/180);
	float cp = cos(angDir.p * M_PI/180);
	float cy = cos(-angDir.y * M_PI/180);
	float cr = cos(angDir.r * M_PI/180);

	// Forward vector
	m[0][0] = cy*cp;
	m[0][1] = -sy*cp;
	m[0][2] = sp;

	// Left vector
	m[1][0] = sp*sr*cy+cr*sy;
	m[1][1] = cr*cy-sy*sp*sr;
	m[1][2] = -cp*sr;

	// Up vector
	m[2][0] = sr*sy-sp*cr*cy;
	m[2][1] = sp*cr*sy+sr*cy;
	m[2][2] = cp*cr;
}

void Matrix4x4::SetRotation(float flAngle, const Vector& v)
{
	// Normalize beforehand
	TAssertNoMsg(fabs(v.LengthSqr() - 1) < 0.000001f);

	// c = cos(angle), s = sin(angle), t = (1-c)
	// [ xxt+c   xyt-zs  xzt+ys ]
	// [ yxt+zs  yyt+c   yzt-xs ]
	// [ zxt-ys  zyt+xs  zzt+c  ]

	float x = v.x;
	float y = v.y;
	float z = v.z;

	float c = cos(flAngle*M_PI/180);
	float s = sin(flAngle*M_PI/180);
	float t = 1-c;

	m[0][0] = x*x*t + c;
	m[1][0] = x*y*t - z*s;
	m[2][0] = x*z*t + y*s;

	m[0][1] = y*x*t + z*s;
	m[1][1] = y*y*t + c;
	m[2][1] = y*z*t - x*s;

	m[0][2] = z*x*t - y*s;
	m[1][2] = z*y*t + x*s;
	m[2][2] = z*z*t + c;
}

void Matrix4x4::SetRotation(const Quaternion& q)
{
	float x = q.x;
	float y = q.y;
	float z = q.z;
	float w = q.w;

	float x2 = 2*x*x;
	float y2 = 2*y*y;
	float z2 = 2*z*z;

	float xy2 = 2*x*y;
	float xz2 = 2*x*z;
	float yz2 = 2*y*z;
	float xw2 = 2*x*w;
	float zw2 = 2*z*w;
	float yw2 = 2*y*w;

	m[0][0] = 1 - y2 - z2;
	m[1][0] = xz2 + yw2;
	m[2][0] = xy2 - zw2;

	m[0][1] = xz2 - yw2;
	m[1][1] = 1 - x2 - y2;
	m[2][1] = yz2 + xw2;

	m[0][2] = xy2 + zw2;
	m[1][2] = yz2 - xw2;
	m[2][2] = 1 - x2 - z2;
}

void Matrix4x4::SetOrientation(const Vector& v, const Vector& vecUp)
{
	Vector vecDir = v.Normalized();

	Vector vecLeft;
	if (vecDir != vecUp && vecDir != -vecUp)
		vecLeft = vecUp.Cross(vecDir).Normalized();
	else
		vecLeft = Vector(0, 1, 0);

	SetForwardVector(vecDir);
	SetLeftVector(vecLeft);
	SetUpVector(vecDir.Cross(vecLeft).Normalized());
}

void Matrix4x4::SetScale(const Vector& vecScale)
{
	m[0][0] = vecScale.x;
	m[1][1] = vecScale.y;
	m[2][2] = vecScale.z;
}

void Matrix4x4::SetReflection(const Vector& vecPlane)
{
	// Normalize beforehand or use ::SetReflection()
	TAssertNoMsg(fabs(vecPlane.LengthSqr() - 1) < 0.000001f);

	m[0][0] = 1 - 2 * vecPlane.x * vecPlane.x;
	m[1][1] = 1 - 2 * vecPlane.y * vecPlane.y;
	m[2][2] = 1 - 2 * vecPlane.z * vecPlane.z;
	m[1][0] = m[0][1] = -2 * vecPlane.x * vecPlane.y;
	m[2][0] = m[0][2] = -2 * vecPlane.x * vecPlane.z;
	m[1][2] = m[2][1] = -2 * vecPlane.y * vecPlane.z;
}

Matrix4x4 Matrix4x4::ProjectPerspective(float flFOV, float flAspectRatio, float flNear, float flFar)
{
	float flRight = flNear * tan(flFOV * M_PI / 360);
	float flLeft = -flRight;

	float flBottom = flLeft / flAspectRatio;
	float flTop = flRight / flAspectRatio;

	return ProjectFrustum(flLeft, flRight, flBottom, flTop, flNear, flFar);
}

Matrix4x4 Matrix4x4::ProjectFrustum(float flLeft, float flRight, float flBottom, float flTop, float flNear, float flFar)
{
	Matrix4x4 m;
	
	m.Identity();

	float flXD = flRight - flLeft;
	float flYD = flTop - flBottom;
	float flZD = flFar - flNear;

	m.m[0][0] = (2 * flNear) / flXD;
	m.m[1][1] = (2 * flNear) / flYD;

	m.m[2][0] = (flRight + flLeft) / flXD;
	m.m[2][1] = (flTop + flBottom) / flYD;
	m.m[2][2] = -(flFar + flNear) / flZD;
	m.m[2][3] = -1;

	m.m[3][2] = -(2 * flFar * flNear) / flZD;

	m.m[3][3] = 0;

	return m;
}

Matrix4x4 Matrix4x4::ProjectOrthographic(float flLeft, float flRight, float flBottom, float flTop, float flNear, float flFar)
{
	Matrix4x4 m;
	
	m.Identity();

	float flXD = flRight - flLeft;
	float flYD = flTop - flBottom;
	float flZD = flFar - flNear;

	m.m[0][0] = 2.0f / flXD;
	m.m[1][1] = 2.0f / flYD;
	m.m[2][2] = -2.0f / flZD;

	m.m[3][0] = -(flRight + flLeft) / flXD;
	m.m[3][1] = -(flTop + flBottom) / flYD;
	m.m[3][2] = -(flFar + flNear) / flZD;

	return m;
}

Matrix4x4 Matrix4x4::ConstructCameraView(const Vector& vecPosition, const Vector& vecDirection, const Vector& vecUp)
{
	Matrix4x4 m;
	
	m.Identity();

	TAssertNoMsg(fabs(vecDirection.LengthSqr()-1) < 0.0001f);

	Vector vecCamSide = vecDirection.Cross(vecUp).Normalized();
	Vector vecCamUp = vecCamSide.Cross(vecDirection);

	m.SetForwardVector(Vector(vecCamSide.x, vecCamUp.x, -vecDirection.x));
	m.SetLeftVector(Vector(vecCamSide.y, vecCamUp.y, -vecDirection.y));
	m.SetUpVector(Vector(vecCamSide.z, vecCamUp.z, -vecDirection.z));

	m.AddTranslation(-vecPosition);

	return m;
}

Matrix4x4 Matrix4x4::operator+=(const Vector& v)
{
	m[3][0] += v.x;
	m[3][1] += v.y;
	m[3][2] += v.z;

	return *this;
}

Matrix4x4 Matrix4x4::operator-(const Vector& v) const
{
	Matrix4x4 r = *this;

	r.m[3][0] -= v.x;
	r.m[3][1] -= v.y;
	r.m[3][2] -= v.z;

	return r;
}

Matrix4x4 Matrix4x4::operator+=(const EAngle& a)
{
	Matrix4x4 r;
	r.SetAngles(a);
	(*this) *= r;

	return *this;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& t) const
{
	Matrix4x4 r;

	// [a b c d][A B C D]   [aA+bE+cI+dM
	// [e f g h][E F G H] = [eA+fE+gI+hM ...
	// [i j k l][I J K L]
	// [m n o p][M N O P]

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			r.m[i][j] = m[0][j]*t.m[i][0] + m[1][j]*t.m[i][1] + m[2][j]*t.m[i][2] + m[3][j]*t.m[i][3];
	}

	return r;
}

Matrix4x4 Matrix4x4::operator*=(const Matrix4x4& t)
{
	*this = (*this)*t;

	return *this;
}

bool Matrix4x4::operator==(const Matrix4x4& t) const
{
	float flEp = 0.000001f;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (fabs(m[i][j] - t.m[i][j]) > flEp)
				return false;
		}
	}

	return true;
}

bool Matrix4x4::Equals(const Matrix4x4& t, float flEp) const
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (fabs(m[i][j] - t.m[i][j]) > flEp)
				return false;
		}
	}

	return true;
}

Matrix4x4 Matrix4x4::AddTranslation(const Vector& v)
{
	Matrix4x4 r;
	r.SetTranslation(v);
	(*this) *= r;

	return *this;
}

Matrix4x4 Matrix4x4::AddAngles(const EAngle& a)
{
	Matrix4x4 r;
	r.SetAngles(a);
	(*this) *= r;

	return *this;
}

Matrix4x4 Matrix4x4::AddScale(const Vector& vecScale)
{
	Matrix4x4 r;
	r.SetScale(vecScale);
	(*this) *= r;

	return *this;
}

Matrix4x4 Matrix4x4::AddReflection(const Vector& v)
{
	Matrix4x4 r;
	r.SetReflection(v);
	(*this) *= r;

	return *this;
}

Vector Matrix4x4::GetTranslation() const
{
	return Vector((float*)&m[3][0]);
}

EAngle Matrix4x4::GetAngles() const
{
#ifdef _DEBUG
	// If any of the below is not true then you have a matrix that has been scaled or reflected or something and it won't work to try to pull its Eulers
	bool b = fabs(GetForwardVector().LengthSqr() - 1) < 0.001f;
	if (!b)
	{
		TAssertNoMsg(b);
		return EAngle(0, 0, 0);
	}

	b = fabs(GetUpVector().LengthSqr() - 1) < 0.001f;
	if (!b)
	{
		TAssertNoMsg(b);
		return EAngle(0, 0, 0);
	}

	b = fabs(GetLeftVector().LengthSqr() - 1) < 0.001f;
	if (!b)
	{
		TAssertNoMsg(b);
		return EAngle(0, 0, 0);
	}

	b = GetForwardVector().Cross(GetLeftVector()).Equals(GetUpVector(), 0.001f);
	if (!b)
	{
		TAssertNoMsg(b);
		return EAngle(0, 0, 0);
	}
#endif

	if (m[0][2] > 0.999999f)
		return EAngle(asin(Clamp(m[0][2], -1.0f, 1.0f)) * 180/M_PI, -atan2(m[1][0], m[1][1]) * 180/M_PI, 0);
	else if (m[0][2] < -0.999999f)
		return EAngle(asin(Clamp(m[0][2], -1.0f, 1.0f)) * 180/M_PI, -atan2(m[1][0], m[1][1]) * 180/M_PI, 0);

	// Clamp to [-1, 1] looping
	float flPitch = fmod(m[0][2], 2.0f);
	if (flPitch > 1)
		flPitch -= 2;
	else if (flPitch < -1)
		flPitch += 2;

	return EAngle(asin(flPitch) * 180/M_PI, -atan2(-m[0][1], m[0][0]) * 180/M_PI, atan2(-m[1][2], m[2][2]) * 180/M_PI);
}

Vector Matrix4x4::GetScale() const
{
	Vector vecReturn;
	vecReturn.x = GetForwardVector().Length();
	vecReturn.y = GetLeftVector().Length();
	vecReturn.z = GetUpVector().Length();
	return vecReturn;
}

Vector Matrix4x4::operator*(const Vector& v) const
{
	// [a b c x][X] 
	// [d e f y][Y] = [aX+bY+cZ+x dX+eY+fZ+y gX+hY+iZ+z]
	// [g h i z][Z]
	//          [1]

	Vector vecResult;
	vecResult.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0];
	vecResult.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1];
	vecResult.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2];
	return vecResult;
}

const Ray Matrix4x4::operator*(const Ray& r) const
{
	Ray rResult;
	rResult.m_vecDir = TransformVector(r.m_vecDir);
	rResult.m_vecPos = (*this) * r.m_vecPos;
	return rResult;
}

Vector Matrix4x4::TransformVector(const Vector& v) const
{
	// [a b c][X] 
	// [d e f][Y] = [aX+bY+cZ dX+eY+fZ gX+hY+iZ]
	// [g h i][Z]

	Vector vecResult;
	vecResult.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z;
	vecResult.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z;
	vecResult.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z;
	return vecResult;
}

Vector4D Matrix4x4::operator*(const Vector4D& v) const
{
	// [a b c x][X] 
	// [d e f y][Y] = [aX+bY+cZ+xW dX+eY+fZ+yW gX+hY+iZ+zW jX+kY+lZ+mW]
	// [g h i z][Z]
	// [j k l m][W]

	Vector4D vecResult;
	vecResult.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0] * v.w;
	vecResult.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1] * v.w;
	vecResult.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2] * v.w;
	vecResult.w = m[0][3] * v.x + m[1][3] * v.y + m[2][3] * v.z + m[3][3] * v.w;
	return vecResult;
}

Vector4D Matrix4x4::GetColumn(int i) const
{
	return Vector4D(m[i][0], m[i][1], m[i][2], m[i][3]);
}

void Matrix4x4::SetColumn(int i, const Vector4D& vecColumn)
{
	m[i][0] = vecColumn.x;
	m[i][1] = vecColumn.y;
	m[i][2] = vecColumn.z;
	m[i][3] = vecColumn.w;
}

void Matrix4x4::SetColumn(int i, const Vector& vecColumn)
{
	m[i][0] = vecColumn.x;
	m[i][1] = vecColumn.y;
	m[i][2] = vecColumn.z;
}

void Matrix4x4::SetForwardVector(const Vector& v)
{
	m[0][0] = v.x;
	m[0][1] = v.y;
	m[0][2] = v.z;
}

void Matrix4x4::SetLeftVector(const Vector& v)
{
	m[1][0] = v.x;
	m[1][1] = v.y;
	m[1][2] = v.z;
}

void Matrix4x4::SetUpVector(const Vector& v)
{
	m[2][0] = v.x;
	m[2][1] = v.y;
	m[2][2] = v.z;
}

// Not a true inversion, only works if the matrix is a translation/rotation matrix.
void Matrix4x4::InvertRT()
{
	TAssertNoMsg(fabs(GetForwardVector().LengthSqr() - 1) < 0.00001f);
	TAssertNoMsg(fabs(GetLeftVector().LengthSqr() - 1) < 0.00001f);
	TAssertNoMsg(fabs(GetUpVector().LengthSqr() - 1) < 0.00001f);

	Matrix4x4 t;

	for (int h = 0; h < 3; h++)
		for (int v = 0; v < 3; v++)
			t.m[h][v] = m[v][h];

	Vector vecTranslation = GetTranslation();

	Init(t);

	SetTranslation(t*(-vecTranslation));
}

Matrix4x4 Matrix4x4::InvertedRT() const
{
	TAssertNoMsg(fabs(GetForwardVector().LengthSqr() - 1) < 0.00001f);
	TAssertNoMsg(fabs(GetLeftVector().LengthSqr() - 1) < 0.00001f);
	TAssertNoMsg(fabs(GetUpVector().LengthSqr() - 1) < 0.00001f);

	Matrix4x4 r;

	for (int h = 0; h < 3; h++)
		for (int v = 0; v < 3; v++)
			r.m[h][v] = m[v][h];

	r.SetTranslation(r*(-GetTranslation()));

	return r;
}

const Matrix4x4 Matrix4x4::Inverted() const
{
	Matrix4x4 r;

	float D;

	r.mm[0] =   mm[5]*mm[10]*mm[15] - mm[5]*mm[11]*mm[14] - mm[9]*mm[6]*mm[15]
	          + mm[9]*mm[7]*mm[14]  + mm[13]*mm[6]*mm[11] - mm[13]*mm[7]*mm[10];
	r.mm[4] =  -mm[4]*mm[10]*mm[15] + mm[4]*mm[11]*mm[14] + mm[8]*mm[6]*mm[15]
	          - mm[8]*mm[7]*mm[14]  - mm[12]*mm[6]*mm[11] + mm[12]*mm[7]*mm[10];
	r.mm[8] =   mm[4]*mm[9]*mm[15]  - mm[4]*mm[11]*mm[13] - mm[8]*mm[5]*mm[15]
	          + mm[8]*mm[7]*mm[13]  + mm[12]*mm[5]*mm[11] - mm[12]*mm[7]*mm[9];
	r.mm[12] = -mm[4]*mm[9]*mm[14]  + mm[4]*mm[10]*mm[13] + mm[8]*mm[5]*mm[14]
	          - mm[8]*mm[6]*mm[13]  - mm[12]*mm[5]*mm[10] + mm[12]*mm[6]*mm[9];
	r.mm[1] =  -mm[1]*mm[10]*mm[15] + mm[1]*mm[11]*mm[14] + mm[9]*mm[2]*mm[15]
	          - mm[9]*mm[3]*mm[14]  - mm[13]*mm[2]*mm[11] + mm[13]*mm[3]*mm[10];
	r.mm[5] =   mm[0]*mm[10]*mm[15] - mm[0]*mm[11]*mm[14] - mm[8]*mm[2]*mm[15]
	          + mm[8]*mm[3]*mm[14]  + mm[12]*mm[2]*mm[11] - mm[12]*mm[3]*mm[10];
	r.mm[9] =  -mm[0]*mm[9]*mm[15]  + mm[0]*mm[11]*mm[13] + mm[8]*mm[1]*mm[15]
	          - mm[8]*mm[3]*mm[13]  - mm[12]*mm[1]*mm[11] + mm[12]*mm[3]*mm[9];
	r.mm[13] =  mm[0]*mm[9]*mm[14]  - mm[0]*mm[10]*mm[13] - mm[8]*mm[1]*mm[14]
	          + mm[8]*mm[2]*mm[13]  + mm[12]*mm[1]*mm[10] - mm[12]*mm[2]*mm[9];
	r.mm[2] =   mm[1]*mm[6]*mm[15]  - mm[1]*mm[7]*mm[14]  - mm[5]*mm[2]*mm[15]
	          + mm[5]*mm[3]*mm[14]  + mm[13]*mm[2]*mm[7]  - mm[13]*mm[3]*mm[6];
	r.mm[6] =  -mm[0]*mm[6]*mm[15]  + mm[0]*mm[7]*mm[14]  + mm[4]*mm[2]*mm[15]
	          - mm[4]*mm[3]*mm[14]  - mm[12]*mm[2]*mm[7]  + mm[12]*mm[3]*mm[6];
	r.mm[10] =  mm[0]*mm[5]*mm[15]  - mm[0]*mm[7]*mm[13]  - mm[4]*mm[1]*mm[15]
	          + mm[4]*mm[3]*mm[13]  + mm[12]*mm[1]*mm[7]  - mm[12]*mm[3]*mm[5];
	r.mm[14] = -mm[0]*mm[5]*mm[14]  + mm[0]*mm[6]*mm[13]  + mm[4]*mm[1]*mm[14]
	          - mm[4]*mm[2]*mm[13]  - mm[12]*mm[1]*mm[6]  + mm[12]*mm[2]*mm[5];
	r.mm[3] =  -mm[1]*mm[6]*mm[11]  + mm[1]*mm[7]*mm[10]  + mm[5]*mm[2]*mm[11]
	          - mm[5]*mm[3]*mm[10]  - mm[9]*mm[2]*mm[7]   + mm[9]*mm[3]*mm[6];
	r.mm[7] =   mm[0]*mm[6]*mm[11]  - mm[0]*mm[7]*mm[10]  - mm[4]*mm[2]*mm[11]
	          + mm[4]*mm[3]*mm[10]  + mm[8]*mm[2]*mm[7]   - mm[8]*mm[3]*mm[6];
	r.mm[11] = -mm[0]*mm[5]*mm[11]  + mm[0]*mm[7]*mm[9]   + mm[4]*mm[1]*mm[11]
	          - mm[4]*mm[3]*mm[9]   - mm[8]*mm[1]*mm[7]   + mm[8]*mm[3]*mm[5];
	r.mm[15] =  mm[0]*mm[5]*mm[10]  - mm[0]*mm[6]*mm[9]   - mm[4]*mm[1]*mm[10]
	          + mm[4]*mm[2]*mm[9]   + mm[8]*mm[1]*mm[6]   - mm[8]*mm[2]*mm[5];

	D = mm[0] * r.mm[0] + mm[1] * r.mm[4] + mm[2] * r.mm[8] + mm[3] * r.mm[12];

	if (D == 0)
		return Matrix4x4();

	for (int i = 0; i < 16; i++)
		r.mm[i] = r.mm[i] / D;

	return r;
}

float Matrix4x4::Trace() const
{
	return m[0][0] + m[1][1] + m[2][2];
}

DoubleMatrix4x4::DoubleMatrix4x4(const Matrix4x4& i)
{
	m[0][0] = i.m[0][0]; m[0][1] = i.m[0][1]; m[0][2] = i.m[0][2]; m[0][3] = i.m[0][3];
	m[1][0] = i.m[1][0]; m[1][1] = i.m[1][1]; m[1][2] = i.m[1][2]; m[1][3] = i.m[1][3];
	m[2][0] = i.m[2][0]; m[2][1] = i.m[2][1]; m[2][2] = i.m[2][2]; m[2][3] = i.m[2][3];
	m[3][0] = i.m[3][0]; m[3][1] = i.m[3][1]; m[3][2] = i.m[3][2]; m[3][3] = i.m[3][3];
}

void DoubleMatrix4x4::Identity()
{
	memset(this, 0, sizeof(DoubleMatrix4x4));

	m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0;
}

bool DoubleMatrix4x4::operator==(const DoubleMatrix4x4& t) const
{
	double flEp = 0.000001f;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (fabs(m[i][j] - t.m[i][j]) > flEp)
				return false;
		}
	}

	return true;
}

void DoubleMatrix4x4::SetTranslation(const DoubleVector& vecPos)
{
	m[3][0] = vecPos.x;
	m[3][1] = vecPos.y;
	m[3][2] = vecPos.z;
}

DoubleMatrix4x4 DoubleMatrix4x4::operator-(const DoubleVector& v) const
{
	DoubleMatrix4x4 r = *this;

	r.m[3][0] -= v.x;
	r.m[3][1] -= v.y;
	r.m[3][2] -= v.z;

	return r;
}

DoubleVector DoubleMatrix4x4::GetTranslation() const
{
	return DoubleVector((double*)&m[3][0]);
}

DoubleMatrix4x4 DoubleMatrix4x4::operator*(const DoubleMatrix4x4& t) const
{
	DoubleMatrix4x4 r;

	// [a b c d][A B C D]   [aA+bE+cI+dM
	// [e f g h][E F G H] = [eA+fE+gI+hM ...
	// [i j k l][I J K L]
	// [m n o p][M N O P]

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			r.m[i][j] = m[0][j]*t.m[i][0] + m[1][j]*t.m[i][1] + m[2][j]*t.m[i][2] + m[3][j]*t.m[i][3];
	}

	return r;
}

DoubleVector DoubleMatrix4x4::operator*(const DoubleVector& v) const
{
	// [a b c x][X] 
	// [d e f y][Y] = [aX+bY+cZ+x dX+eY+fZ+y gX+hY+iZ+z]
	// [g h i z][Z]
	//          [1]

	DoubleVector vecResult;
	vecResult.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0];
	vecResult.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1];
	vecResult.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2];
	return vecResult;
}

DoubleVector DoubleMatrix4x4::TransformVector(const DoubleVector& v) const
{
	// [a b c][X] 
	// [d e f][Y] = [aX+bY+cZ dX+eY+fZ gX+hY+iZ]
	// [g h i][Z]

	DoubleVector vecResult;
	vecResult.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z;
	vecResult.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z;
	vecResult.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z;
	return vecResult;
}

void DoubleMatrix4x4::SetForwardVector(const DoubleVector& v)
{
	m[0][0] = v.x;
	m[0][1] = v.y;
	m[0][2] = v.z;
}

void DoubleMatrix4x4::SetLeftVector(const DoubleVector& v)
{
	m[1][0] = v.x;
	m[1][1] = v.y;
	m[1][2] = v.z;
}

void DoubleMatrix4x4::SetUpVector(const DoubleVector& v)
{
	m[2][0] = v.x;
	m[2][1] = v.y;
	m[2][2] = v.z;
}

DoubleMatrix4x4 DoubleMatrix4x4::InvertedRT() const
{
	TAssertNoMsg(fabs(GetForwardVector().LengthSqr() - 1) < 0.00001);
	TAssertNoMsg(fabs(GetLeftVector().LengthSqr() - 1) < 0.00001);
	TAssertNoMsg(fabs(GetUpVector().LengthSqr() - 1) < 0.00001);

	DoubleMatrix4x4 r;

	for (int h = 0; h < 3; h++)
		for (int v = 0; v < 3; v++)
			r.m[h][v] = m[v][h];

	r.SetTranslation(r*(-GetTranslation()));

	return r;
}
