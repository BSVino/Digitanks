#pragma once

#include <glgui/movablepanel.h>

class CEntityPropertiesPanel;

class CCreateEntityPanel : public glgui::CMovablePanel
{
	DECLARE_CLASS(CCreateEntityPanel, glgui::CMovablePanel);

public:
							CCreateEntityPanel();

public:
	void					Layout();

	EVENT_CALLBACK(CCreateEntityPanel, ChooseClass);
	EVENT_CALLBACK(CCreateEntityPanel, ModelChanged);

public:
	bool					m_bReadyToCreate;

	glgui::CControl<glgui::CMenu>			m_hClass;

	glgui::CControl<glgui::CLabel>			m_hNameLabel;
	glgui::CControl<glgui::CTextField>		m_hNameText;

	glgui::CControl<glgui::CLabel>			m_hModelLabel;
	glgui::CControl<glgui::CTextField>		m_hModelText;

	glgui::CControl<CEntityPropertiesPanel>	m_hPropertiesPanel;
};
