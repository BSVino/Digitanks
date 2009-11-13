#include "modelwindow.h"

#include "../crunch.h"
#include "modelgui.h"

void CModelWindow::MouseMotion(int x, int y)
{
	modelgui::CRootPanel::Get()->CursorMoved(x, (int)m_iWindowHeight-y);
}

void CModelWindow::MouseDragged(int x, int y)
{
	if (m_bCameraRotating)
	{
		m_flCameraPitch += (x - m_iMouseStartX)/2;
		m_flCameraYaw += (y - m_iMouseStartY)/2;
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

	modelgui::CRootPanel::Get()->CursorMoved(x, (int)m_iWindowHeight-y);
}

void CModelWindow::MouseInput(int iButton, int iState, int x, int y)
{
	if (iState == GLUT_DOWN)
		modelgui::CRootPanel::Get()->MousePressed(iButton, x, (int)m_iWindowHeight-y);
	else
		modelgui::CRootPanel::Get()->MouseReleased(iButton, x, (int)m_iWindowHeight-y);

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

void CModelWindow::KeyPress(unsigned char c, int x, int y)
{
	if (c == 27)
		exit(0);

	if (c == 'a')
	{
		CAOGenerator ao(&m_Scene, &m_aoMaterials);
		ao.SetSize(128, 128);
		ao.SetUseTexture(true);
		ao.Generate();
		ao.SaveToFile("ao.bmp");

		size_t iAO = ao.GenerateTexture();
		for (size_t i = 0; i < m_aoMaterials.size(); i++)
		{
			if (m_aoMaterials[0].m_iAO)
				glDeleteTextures(1, &m_aoMaterials[0].m_iAO);
			m_aoMaterials[0].m_iAO = iAO;
		}
		CreateGLLists();
	}

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

void CModelWindow::OpenCallback()
{
	ReadFile(OpenFile());
}

void CModelWindow::ExitCallback()
{
	exit(0);
}

void CModelWindow::AboutCallback()
{
	OpenAboutPanel();
}
