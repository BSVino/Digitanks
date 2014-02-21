#pragma once

#include <glgui/panel.h>

#include <game/level.h>

#include "entitypropertiespanel.h"
#include "leveleditor.h"

class CEditorPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CEditorPanel, glgui::CPanel);

public:
							CEditorPanel();

public:
	void					Layout();
	void					LayoutEntity();
	void					LayoutOutput();
	void					LayoutInput();

	void                    Paint(float x, float y, float w, float h);

	CLevelEntity*			GetCurrentEntity();
	CLevelEntity::CLevelEntityOutput* GetCurrentOutput();

	EVENT_CALLBACK(CEditorPanel, EntitySelected);
	EVENT_CALLBACK(CEditorPanel, PropertyChanged);
	EVENT_CALLBACK(CEditorPanel, OutputSelected);
	EVENT_CALLBACK(CEditorPanel, AddOutput);
	EVENT_CALLBACK(CEditorPanel, RemoveOutput);
	EVENT_CALLBACK(CEditorPanel, ChooseOutput);
	EVENT_CALLBACK(CEditorPanel, TargetEntityChanged);
	EVENT_CALLBACK(CEditorPanel, ChooseInput);
	EVENT_CALLBACK(CEditorPanel, ArgumentsChanged);

public:
	glgui::CControl<glgui::CTree>			m_hEntities;
	glgui::CControl<glgui::CLabel>			m_hObjectTitle;

	glgui::CControl<glgui::CSlidingContainer>	m_hSlider;
	glgui::CControl<glgui::CSlidingPanel>	m_hPropertiesSlider;
	glgui::CControl<glgui::CSlidingPanel>	m_hOutputsSlider;

	glgui::CControl<CEntityPropertiesPanel>	m_hPropertiesPanel;

	glgui::CControl<glgui::CTree>			m_hOutputs;
	glgui::CControl<glgui::CButton>			m_hAddOutput;
	glgui::CControl<glgui::CButton>			m_hRemoveOutput;

	glgui::CControl<glgui::CMenu>			m_hOutput;

	glgui::CControl<glgui::CLabel>			m_hOutputEntityNameLabel;
	glgui::CControl<glgui::CTextField>		m_hOutputEntityNameText;

	glgui::CControl<glgui::CMenu>			m_hInput;

	glgui::CControl<glgui::CLabel>			m_hOutputArgsLabel;
	glgui::CControl<glgui::CTextField>		m_hOutputArgsText;
};
