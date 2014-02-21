#pragma once

#include <glgui/panel.h>

class CEntityPropertiesPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CEntityPropertiesPanel, glgui::CPanel);

public:
							CEntityPropertiesPanel(bool bCommon);

public:
	void					Layout();

	EVENT_CALLBACK(CEntityPropertiesPanel, ModelChanged);
	EVENT_CALLBACK(CEntityPropertiesPanel, TargetChanged);
	EVENT_CALLBACK(CEntityPropertiesPanel, PropertyChanged);

	void					SetPropertyChangedListener(glgui::IEventListener* pListener, glgui::IEventListener::Callback pfnCallback);

	void					SetClass(const tstring& sClass) { m_sClass = sClass; }
	void					SetEntity(class CLevelEntity* pEntity);

public:
	tstring								m_sClass;
	float								m_bCommonProperties;
	class CLevelEntity*					m_pEntity;

	tvector<glgui::CControl<glgui::CLabel>>				m_ahPropertyLabels;
	tvector<glgui::CControl<glgui::CBaseControl>>		m_ahPropertyOptions;
	tvector<tstring>					m_asPropertyHandle;

	glgui::IEventListener::Callback		m_pfnPropertyChangedCallback;
	glgui::IEventListener*				m_pPropertyChangedListener;
};
