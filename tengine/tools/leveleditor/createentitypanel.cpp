#include "createentitypanel.h"

#include <glgui/movablepanel.h>
#include <glgui/menu.h>
#include <glgui/textfield.h>

#include <game/entities/baseentity.h>

#include "entitypropertiespanel.h"

CCreateEntityPanel::CCreateEntityPanel()
	: glgui::CMovablePanel("Create Entity Tool")
{
	m_hClass = AddControl(new glgui::CMenu("Choose Class"));

	for (size_t i = 0; i < CBaseEntity::GetNumEntitiesRegistered(); i++)
	{
		CEntityRegistration* pRegistration = CBaseEntity::GetEntityRegistration(i);

		if (!pRegistration->m_bCreatableInEditor)
			continue;

		m_hClass->AddSubmenu(pRegistration->m_pszEntityClass+1, this, ChooseClass);
	}

	m_hNameLabel = AddControl(new glgui::CLabel("Name:", "sans-serif", 10));
	m_hNameLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_hNameText = AddControl(new glgui::CTextField());

	m_hModelLabel = AddControl(new glgui::CLabel("Model:", "sans-serif", 10));
	m_hModelLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_hModelText = AddControl(new glgui::CTextField());
	m_hModelText->SetContentsChangedListener(this, ModelChanged);

	m_hPropertiesPanel = AddControl(new CEntityPropertiesPanel(false));
	m_hPropertiesPanel->SetVisible(false);

	m_bReadyToCreate = false;
}

void CCreateEntityPanel::Layout()
{
	m_hClass->SetWidth(100);
	m_hClass->SetHeight(30);
	m_hClass->CenterX();
	m_hClass->SetTop(30);

	float flTop = 70;
	m_hNameLabel->SetLeft(15);
	m_hNameLabel->SetTop(flTop);
	m_hNameText->SetWidth(GetWidth()-30);
	m_hNameText->CenterX();
	m_hNameText->SetTop(flTop+12);

	flTop += 43;

	m_hModelLabel->SetLeft(15);
	m_hModelLabel->SetTop(flTop);
	m_hModelText->SetWidth(GetWidth()-30);
	m_hModelText->CenterX();
	m_hModelText->SetTop(flTop+12);

	flTop += 43;

	m_hPropertiesPanel->SetTop(flTop);
	m_hPropertiesPanel->SetLeft(10);
	m_hPropertiesPanel->SetWidth(GetWidth()-20);
	m_hPropertiesPanel->SetBackgroundColor(Color(10, 10, 10));

	if (m_bReadyToCreate)
	{
		m_hPropertiesPanel->SetClass("C" + m_hClass->GetText());
		m_hPropertiesPanel->SetVisible(true);
	}

	BaseClass::Layout();

	SetHeight(m_hPropertiesPanel->GetBottom()+15);
}

void CCreateEntityPanel::ChooseClassCallback(const tstring& sArgs)
{
	tvector<tstring> asTokens;
	strtok(sArgs, asTokens);

	m_hClass->SetText(asTokens[1]);
	m_hClass->Pop(true, true);

	m_bReadyToCreate = true;

	Layout();
}

void CCreateEntityPanel::ModelChangedCallback(const tstring& sArgs)
{
	if (!m_hModelText->GetText().length())
		return;

	tvector<tstring> asExtensions;
	tvector<tstring> asExtensionsExclude;

	asExtensions.push_back(".toy");
	asExtensions.push_back(".mat");
	asExtensionsExclude.push_back(".mesh.toy");
	asExtensionsExclude.push_back(".phys.toy");
	asExtensionsExclude.push_back(".area.toy");

	m_hModelText->SetAutoCompleteFiles(".", asExtensions, asExtensionsExclude);
}

