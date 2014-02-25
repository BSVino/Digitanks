#include "digitankswindow.h"

#include <sound/sound.h>
#include <glgui/glgui.h>
#include <renderer/renderer.h>
#include <tinker/keys.h>

#include "dissolver.h"
#include "digitanksgame.h"
#include "hud.h"
#include "ui.h"
#include <structures/cpu.h>
#include <dt_camera.h>

bool CDigitanksWindow::GetBoxSelection(size_t& iX, size_t& iY, size_t& iX2, size_t& iY2)
{
	if (!IsMouseDragging())
		return false;

#if 0
	if (m_bBoxSelect)
	{
		iX = (m_iMouseInitialX < m_iMouseCurrentX) ? m_iMouseInitialX : m_iMouseCurrentX;
		iY = (m_iMouseInitialY < m_iMouseCurrentY) ? m_iMouseInitialY : m_iMouseCurrentY;
		iX2 = (m_iMouseInitialX > m_iMouseCurrentX) ? m_iMouseInitialX : m_iMouseCurrentX;
		iY2 = (m_iMouseInitialY > m_iMouseCurrentY) ? m_iMouseInitialY : m_iMouseCurrentY;
		return true;
	}
#endif

	return false;
}

bool CDigitanksWindow::IsMouseDragging()
{
	size_t iDifference = abs((int)m_iMouseCurrentX - (int)m_iMouseInitialX) + abs((int)m_iMouseCurrentY - (int)m_iMouseInitialY);

	return iDifference > 30;
}
