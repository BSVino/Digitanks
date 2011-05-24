#include "intro_renderer.h"

#include <GL/glew.h>

#include <tinker/application.h>
#include <tinker/cvar.h>

CIntroRenderer::CIntroRenderer()
	: CRenderer(CApplication::Get()->GetWindowWidth(), CApplication::Get()->GetWindowHeight())
{
	m_bUseFramebuffers = false;
//	m_bUseShaders = false;
}

#define FRUSTUM_NEAR	0
#define FRUSTUM_FAR		1
#define FRUSTUM_LEFT	2
#define FRUSTUM_RIGHT	3
#define FRUSTUM_UP		4
#define FRUSTUM_DOWN	5

CVar cam_free_ortho("cam_free_ortho", "off");

void CIntroRenderer::StartRendering()
{
	if (!cam_free_ortho.GetBool() && CVar::GetCVarBool("cam_free"))
	{
		BaseClass::StartRendering();
		return;
	}

	float flWidth = (float)CApplication::Get()->GetWindowWidth();
	float flHeight = (float)CApplication::Get()->GetWindowHeight();

	glViewport(0, 0, (GLsizei)flWidth, (GLsizei)flHeight);

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-flWidth/2, flWidth/2, -flHeight/2, flHeight/2, 1, flWidth);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	gluLookAt(m_vecCameraPosition.x, m_vecCameraPosition.y, m_vecCameraPosition.z,
		m_vecCameraTarget.x, m_vecCameraTarget.y, m_vecCameraTarget.z,
		0.0, 1.0, 0.0);

	glGetDoublev( GL_MODELVIEW_MATRIX, m_aiModelView );
	glGetDoublev( GL_PROJECTION_MATRIX, m_aiProjection );

	Matrix4x4 mModelView, mProjection;
	glGetFloatv( GL_MODELVIEW_MATRIX, mModelView );
	glGetFloatv( GL_PROJECTION_MATRIX, mProjection );

	Matrix4x4 mFrustum = mModelView * mProjection;

	float* pflFrustum = mFrustum;

	m_aoFrustum[FRUSTUM_RIGHT].n.x = mFrustum.m[0][3] - mFrustum.m[0][0];
	m_aoFrustum[FRUSTUM_RIGHT].n.y = mFrustum.m[1][3] - mFrustum.m[1][0];
	m_aoFrustum[FRUSTUM_RIGHT].n.z = mFrustum.m[2][3] - mFrustum.m[2][0];
	m_aoFrustum[FRUSTUM_RIGHT].d = mFrustum.m[3][3] - mFrustum.m[3][0];

	m_aoFrustum[FRUSTUM_LEFT].n.x = mFrustum.m[0][3] + mFrustum.m[0][0];
	m_aoFrustum[FRUSTUM_LEFT].n.y = mFrustum.m[1][3] + mFrustum.m[1][0];
	m_aoFrustum[FRUSTUM_LEFT].n.z = mFrustum.m[2][3] + mFrustum.m[2][0];
	m_aoFrustum[FRUSTUM_LEFT].d = mFrustum.m[3][3] + mFrustum.m[3][0];

	m_aoFrustum[FRUSTUM_DOWN].n.x = mFrustum.m[0][3] + mFrustum.m[0][1];
	m_aoFrustum[FRUSTUM_DOWN].n.y = mFrustum.m[1][3] + mFrustum.m[1][1];
	m_aoFrustum[FRUSTUM_DOWN].n.z = mFrustum.m[2][3] + mFrustum.m[2][1];
	m_aoFrustum[FRUSTUM_DOWN].d = mFrustum.m[3][3] + mFrustum.m[3][1];

	m_aoFrustum[FRUSTUM_UP].n.x = mFrustum.m[0][3] - mFrustum.m[0][1];
	m_aoFrustum[FRUSTUM_UP].n.y = mFrustum.m[1][3] - mFrustum.m[1][1];
	m_aoFrustum[FRUSTUM_UP].n.z = mFrustum.m[2][3] - mFrustum.m[2][1];
	m_aoFrustum[FRUSTUM_UP].d = mFrustum.m[3][3] - mFrustum.m[3][1];

	m_aoFrustum[FRUSTUM_FAR].n.x = mFrustum.m[0][3] - mFrustum.m[0][2];
	m_aoFrustum[FRUSTUM_FAR].n.y = mFrustum.m[1][3] - mFrustum.m[1][2];
	m_aoFrustum[FRUSTUM_FAR].n.z = mFrustum.m[2][3] - mFrustum.m[2][2];
	m_aoFrustum[FRUSTUM_FAR].d = mFrustum.m[3][3] - mFrustum.m[3][2];

	m_aoFrustum[FRUSTUM_NEAR].n.x = mFrustum.m[0][3] + mFrustum.m[0][2];
	m_aoFrustum[FRUSTUM_NEAR].n.y = mFrustum.m[1][3] + mFrustum.m[1][2];
	m_aoFrustum[FRUSTUM_NEAR].n.z = mFrustum.m[2][3] + mFrustum.m[2][2];
	m_aoFrustum[FRUSTUM_NEAR].d = mFrustum.m[3][3] + mFrustum.m[3][2];

	// Normalize all plane normals
	for(int i = 0; i < 6; i++)
	{
		m_aoFrustum[i].d = -m_aoFrustum[i].d; // Why? I don't know.
		m_aoFrustum[i].Normalize();
	}

	// Momentarily return the viewport to the window size. This is because if the scene buffer is not the same as the window size,
	// the viewport here will be the scene buffer size, but we need it to be the window size so we can do world/screen transformations.
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);
	glGetIntegerv( GL_VIEWPORT, m_aiViewport );
	glPopAttrib();

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);
}

void CIntroRenderer::FinishRendering()
{
	if (!cam_free_ortho.GetBool() && CVar::GetCVarBool("cam_free"))
	{
		BaseClass::FinishRendering();
		return;
	}

	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}

