#ifndef DT_GENERAL_WINDOW_H
#define DT_GENERAL_WINDOW_H

#include <glgui/glgui.h>
#include <models/texturesheet.h>

class CGeneralWindow : public glgui::CPanel
{
	DECLARE_CLASS(CGeneralWindow, glgui::CPanel);

public:
						CGeneralWindow();

public:
	virtual void		Layout();
	virtual void		Think();
	virtual void		Paint(int x, int y, int w, int h);

	void				Reset();
	void				Deploy();

public:
	float				m_flDeployedGoal;
	float				m_flDeployed;

	glgui::CLabel*		m_pText;
	glgui::CButton*		m_pButton;

	CTextureSheet		m_hGeneral;
	CTextureSheet		m_hGeneralMouth;
	eastl::string		m_sEmotion;
};

#endif
