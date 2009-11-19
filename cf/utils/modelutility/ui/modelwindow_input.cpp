#include "modelwindow.h"

#include "../crunch.h"
#include "modelgui.h"
#include "modelwindow_ui.h"

void CModelWindow::MouseMotion(int x, int y)
{
	modelgui::CRootPanel::Get()->CursorMoved(x, (int)m_iWindowHeight-y);
}

void CModelWindow::MouseDragged(int x, int y)
{
	if (m_bRenderUV)
	{
		if (m_bCameraDollying)
		{
			m_flCameraUVZoom += (float)(y - m_iMouseStartY)/100;

			if (m_flCameraUVZoom < 0.01f)
				m_flCameraUVZoom = 0.01f;

			m_iMouseStartY = y;
			glutPostRedisplay();
		}
	}
	else
	{
		if (m_bCameraRotating)
		{
			m_flCameraPitch += (y - m_iMouseStartY)/2;
			m_flCameraYaw += (x - m_iMouseStartX)/2;

			while (m_flCameraPitch > 89)
				m_flCameraPitch = 89;
			while (m_flCameraPitch < -89)
				m_flCameraPitch = -89;

			m_iMouseStartX = x;
			m_iMouseStartY = y;
			glutPostRedisplay();
		}

		if (m_bCameraDollying)
		{
			m_flCameraDistance += y - m_iMouseStartY;

			if (m_flCameraDistance < 1)
				m_flCameraDistance = 1;

			m_iMouseStartY = y;
			glutPostRedisplay();
		}

		if (m_bLightRotating)
		{
			m_flLightYaw += (m_iMouseStartX - x);
			m_flLightPitch += (m_iMouseStartY - y);

			while (m_flLightYaw >= 180)
				m_flLightYaw -= 360;
			while (m_flLightYaw < -180)
				m_flLightYaw += 360;
			while (m_flLightPitch > 89)
				m_flLightPitch = 89;
			while (m_flLightPitch < -89)
				m_flLightPitch = -89;

			m_iMouseStartX = x;
			m_iMouseStartY = y;
			glutPostRedisplay();
		}
	}

	modelgui::CRootPanel::Get()->CursorMoved(x, (int)m_iWindowHeight-y);
}

void CModelWindow::MouseInput(int iButton, int iState, int x, int y)
{
	if (iState == GLUT_DOWN)
	{
		if (modelgui::CRootPanel::Get()->MousePressed(iButton, x, (int)m_iWindowHeight-y))
			return;
	}
	else
	{
		if (modelgui::CRootPanel::Get()->MouseReleased(iButton, x, (int)m_iWindowHeight-y))
			return;
	}

	if (m_bRenderUV)
	{
		if (iState == GLUT_DOWN)
		{
			m_bCameraDollying = 1;
			m_iMouseStartX = x;
			m_iMouseStartY = y;
		}
		if (iState == GLUT_UP)
			m_bCameraDollying = 0;
	}
	else
	{
		if ((glutGetModifiers() & GLUT_ACTIVE_CTRL) && iButton == GLUT_LEFT_BUTTON)
		{
			if (iState == GLUT_DOWN)
			{
				m_bLightRotating = 1;
				m_iMouseStartX = x;
				m_iMouseStartY = y;
			}
			if (iState == GLUT_UP)
				m_bLightRotating = 0;
		}
		else if (iButton == GLUT_LEFT_BUTTON)
		{
			if (iState == GLUT_DOWN)
			{
				m_bCameraRotating = 1;
				m_iMouseStartX = x;
				m_iMouseStartY = y;
			}
			if (iState == GLUT_UP)
				m_bCameraRotating = 0;
		}
		else if (iButton == GLUT_RIGHT_BUTTON)
		{
			if (iState == GLUT_DOWN)
			{
				m_bCameraDollying = 1;
				m_iMouseStartX = x;
				m_iMouseStartY = y;
			}
			if (iState == GLUT_UP)
				m_bCameraDollying = 0;
		}
	}
}

void CModelWindow::KeyPress(unsigned char c, int x, int y)
{
	if (c == 27)
		exit(0);

	if (c == 'a')
		CAOPanel::Open(false, &m_Scene, &m_aoMaterials);

	if (c == 'r' && (glutGetModifiers()&GLUT_ACTIVE_CTRL))
		ReloadFromFile();

	glutPostRedisplay();
}

void CModelWindow::Special(int k, int x, int y)
{
	if (k == GLUT_KEY_F4 && (glutGetModifiers()&GLUT_ACTIVE_ALT))
		exit(0);

	if (k == GLUT_KEY_F5)
		ReloadFromFile();

	glutPostRedisplay();
}
