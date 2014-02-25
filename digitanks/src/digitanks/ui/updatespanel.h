#ifndef DT_UPDATESPANEL_H
#define DT_UPDATESPANEL_H

#include "glgui/picturebutton.h"
#include "glgui/panel.h"

class CUpdatesPanel;

class CUpdateButton : public glgui::CPictureButton, public glgui::IEventListener
{
public:
									CUpdateButton(class CUpdatesPanel* pPanel);

public:
	void							SetLocation(int x, int y);

	virtual void					CursorIn();
	virtual void					CursorOut();

	virtual void					Think();
	virtual void					Paint(float x, float y, float w, float h);

	EVENT_CALLBACK(CUpdateButton, ChooseDownload);

public:
	glgui::CControl<CUpdatesPanel>	m_pUpdatesPanel;

	int								m_iX;
	int								m_iY;

	float							m_flFocusRamp;
	float							m_flFocusRampGoal;
};

class CUpdatesPanel : public glgui::CPanel, public glgui::IEventListener
{
public:
									CUpdatesPanel();

public:
	virtual void					Layout();
	virtual void					Paint(float x, float y, float w, float h);

	EVENT_CALLBACK(CUpdatesPanel, Close);

	void							UpdateInfo(class CUpdateItem* pInfo);

	void							GetTextureForUpdateItem(class CUpdateItem* pInfo, CMaterialHandle& hSheet, int& sx, int& sy, int& sw, int& sh, int& tw, int& th);

	int								GetButtonSize() { return m_iButtonSize; }

protected:
	glgui::CControl<glgui::CButton>					m_pCloseButton;
	glgui::CControl<glgui::CLabel>					m_pAvailable;
	glgui::CControl<glgui::CLabel>					m_pInfo;
	glgui::CControl<glgui::CLabel>					m_pTutorial;

	tvector<glgui::CControl<CUpdateButton>>			m_apUpdates;

	int								m_iButtonSize;
};

#endif
