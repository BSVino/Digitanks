#include "entitypropertiespanel.h"

#include <glgui/label.h>
#include <glgui/checkbox.h>
#include <glgui/textfield.h>

#include <game/entities/baseentity.h>
#include <game/level.h>

#include "leveleditor.h"

CEntityPropertiesPanel::CEntityPropertiesPanel(bool bCommon)
{
	SetVerticalScrollBarEnabled(true);
	SetScissoring(true);
	m_bCommonProperties = bCommon;
	m_pEntity = nullptr;
	m_pPropertyChangedListener = nullptr;
}

void CEntityPropertiesPanel::Layout()
{
	if (!m_sClass.length())
		return;

	float flTop = 5;

	TAssert(m_ahPropertyLabels.size() == m_ahPropertyOptions.size());
	for (size_t i = 0; i < m_ahPropertyLabels.size(); i++)
	{
		RemoveControl(m_ahPropertyLabels[i]);
		RemoveControl(m_ahPropertyOptions[i]);
	}
	m_ahPropertyLabels.clear();
	m_ahPropertyOptions.clear();
	m_asPropertyHandle.clear();

	// If we're ready to create then a class has been chosen.
	const tchar* pszClassName = m_sClass.c_str();
	CEntityRegistration* pRegistration = NULL;
	tmap<tstring, bool> abHandlesSet;

	do
	{
		pRegistration = CBaseEntity::GetRegisteredEntity(pszClassName);

		for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
		{
			auto pSaveData = &pRegistration->m_aSaveData[i];

			if (!pSaveData->m_pszHandle || !pSaveData->m_pszHandle[0])
				continue;

			if (!pSaveData->m_bShowInEditor)
				continue;

			if (!m_bCommonProperties)
			{
				if (strcmp(pSaveData->m_pszHandle, "Name") == 0)
					continue;

				if (strcmp(pSaveData->m_pszHandle, "Model") == 0)
					continue;

				if (strcmp(pSaveData->m_pszHandle, "Origin") == 0)
					continue;

				if (strcmp(pSaveData->m_pszHandle, "Angles") == 0)
					continue;
			}

			if (abHandlesSet.find(tstring(pSaveData->m_pszHandle)) != abHandlesSet.end())
				continue;

			abHandlesSet[tstring(pSaveData->m_pszHandle)] = true;

			m_asPropertyHandle.push_back(tstring(pSaveData->m_pszHandle));

			m_ahPropertyLabels.push_back(AddControl(new glgui::CLabel(tstring(pSaveData->m_pszHandle) + ": ", "sans-serif", 10), true));
			m_ahPropertyLabels.back()->SetAlign(glgui::CLabel::TA_TOPLEFT);
			m_ahPropertyLabels.back()->SetLeft(0);
			m_ahPropertyLabels.back()->SetTop(flTop);
			m_ahPropertyLabels.back()->SetWidth(10);
			m_ahPropertyLabels.back()->EnsureTextFits();

			if (strcmp(pSaveData->m_pszType, "bool") == 0)
			{
				glgui::CControl<glgui::CCheckBox> hCheckBox(AddControl(new glgui::CCheckBox(), true));
				m_ahPropertyOptions.push_back(hCheckBox.GetHandle());
				hCheckBox->SetLeft(m_ahPropertyLabels.back()->GetRight() + 10);
				hCheckBox->SetTop(flTop);
				hCheckBox->SetSize(12, 12);

				if (m_pEntity && m_pEntity->HasParameterValue(pSaveData->m_pszHandle))
					hCheckBox->SetState(UnserializeString_bool(m_pEntity->GetParameterValue(pSaveData->m_pszHandle)));
				else if (pSaveData->m_bDefault)
					hCheckBox->SetState(!!pSaveData->m_oDefault[0], false);

				hCheckBox->SetClickedListener(this, PropertyChanged);
				hCheckBox->SetUnclickedListener(this, PropertyChanged);

				flTop += 17;
			}
			else
			{
				if (strcmp(pSaveData->m_pszType, "Vector") == 0)
					m_ahPropertyLabels.back()->AppendText(" (x y z)");
				else if (strcmp(pSaveData->m_pszType, "Vector2D") == 0)
					m_ahPropertyLabels.back()->AppendText(" (x y)");
				else if (strcmp(pSaveData->m_pszType, "QAngle") == 0)
					m_ahPropertyLabels.back()->AppendText(" (p y r)");
				else if (strcmp(pSaveData->m_pszType, "Matrix4x4") == 0)
					m_ahPropertyLabels.back()->AppendText(" (p y r x y z)");
				m_ahPropertyLabels.back()->SetWidth(200);

				glgui::CControl<glgui::CTextField> hTextField(AddControl(new glgui::CTextField(), true));
				m_ahPropertyOptions.push_back(hTextField.GetHandle());
				hTextField->Layout_FullWidth(0);
				hTextField->SetWidth(hTextField->GetWidth()-15);
				hTextField->SetTop(flTop+12);

				hTextField->SetContentsChangedListener(nullptr, nullptr);

				if (m_pEntity && m_pEntity->HasParameterValue(pSaveData->m_pszHandle))
				{
					hTextField->SetText(m_pEntity->GetParameterValue(pSaveData->m_pszHandle));

					if (strcmp(pSaveData->m_pszType, "Vector") == 0 && CanUnserializeString_TVector(hTextField->GetText()))
					{
						Vector v = UnserializeString_TVector(hTextField->GetText());
						hTextField->SetText(pretty_float(v.x, 4) + " " + pretty_float(v.y, 4) + " " + pretty_float(v.z, 4));
					}
					else if (strcmp(pSaveData->m_pszType, "EAngle") == 0 && CanUnserializeString_EAngle(hTextField->GetText()))
					{
						EAngle v = UnserializeString_EAngle(hTextField->GetText());
						hTextField->SetText(pretty_float(v.p, 4) + " " + pretty_float(v.y, 4) + " " + pretty_float(v.r, 4));
					}
				}
				else if (pSaveData->m_bDefault)
				{
					if (strcmp(pSaveData->m_pszVariableName, "m_iModel") == 0)
					{
						hTextField->SetText(pSaveData->m_oDefault);
					}
					else if (strcmp(pSaveData->m_pszType, "size_t") == 0)
					{
						size_t i = *((size_t*)&pSaveData->m_oDefault[0]);
						hTextField->SetText(sprintf("%d", i));
					}
					else if (strcmp(pSaveData->m_pszType, "float") == 0)
					{
						float v = *((float*)&pSaveData->m_oDefault[0]);
						hTextField->SetText(pretty_float(v));
					}
					else if (strcmp(pSaveData->m_pszType, "Vector") == 0)
					{
						Vector v = *((Vector*)&pSaveData->m_oDefault[0]);
						hTextField->SetText(pretty_float(v.x, 4) + " " + pretty_float(v.y, 4) + " " + pretty_float(v.z, 4));
					}
					else if (strcmp(pSaveData->m_pszType, "Vector2D") == 0)
					{
						Vector2D v = *((Vector2D*)&pSaveData->m_oDefault[0]);
						hTextField->SetText(pretty_float(v.x, 4) + " " + pretty_float(v.y, 4));
					}
					else if (strcmp(pSaveData->m_pszType, "EAngle") == 0)
					{
						EAngle v = *((EAngle*)&pSaveData->m_oDefault[0]);
						hTextField->SetText(pretty_float(v.p, 4) + " " + pretty_float(v.y, 4) + " " + pretty_float(v.r, 4));
					}
					else if (strcmp(pSaveData->m_pszType, "Matrix4x4") == 0)
					{
						Matrix4x4 m = *((Matrix4x4*)&pSaveData->m_oDefault[0]);
						EAngle e = m.GetAngles();
						Vector v = m.GetTranslation();
						hTextField->SetText(pretty_float(e.p, 4) + " " + pretty_float(v.y, 4) + " " + pretty_float(e.r, 4) + " " + pretty_float(v.x, 4) + " " + pretty_float(v.y, 4) + " " + pretty_float(v.z, 4));
					}
					else if (strcmp(pSaveData->m_pszType, "AABB") == 0)
					{
						AABB b = *((AABB*)&pSaveData->m_oDefault[0]);
						Vector v1 = b.m_vecMins;
						Vector v2 = b.m_vecMaxs;
						hTextField->SetText(pretty_float(v1.x, 4) + " " + pretty_float(v1.y, 4) + " " + pretty_float(v1.z, 4) + " " + pretty_float(v2.x, 4) + " " + pretty_float(v2.y, 4) + " " + pretty_float(v2.z, 4));
					}
					else
					{
						TUnimplemented();
					}
				}

				if (strcmp(pSaveData->m_pszHandle, "Model") == 0)
					hTextField->SetContentsChangedListener(this, ModelChanged, sprintf("%d", m_ahPropertyOptions.size()-1));
				else if (tstring(pSaveData->m_pszType).startswith("CEntityHandle"))
					hTextField->SetContentsChangedListener(this, TargetChanged, sprintf("%d ", m_ahPropertyOptions.size()-1) + pSaveData->m_pszType);
				else
					hTextField->SetContentsChangedListener(this, PropertyChanged);

				flTop += 43;
			}
		}

		pszClassName = pRegistration->m_pszParentClass;
	} while (pRegistration->m_pszParentClass);

	float flMaxHeight = GetParent()->GetHeight() - GetParent()->GetDefaultMargin();
	if (flTop > flMaxHeight)
		flTop = flMaxHeight;

	SetHeight(flTop);

	BaseClass::Layout();
}

void CEntityPropertiesPanel::ModelChangedCallback(const tstring& sArgs)
{
	tvector<tstring> asExtensions;
	tvector<tstring> asExtensionsExclude;

	asExtensions.push_back(".toy");
	asExtensions.push_back(".mat");
	asExtensionsExclude.push_back(".mesh.toy");
	asExtensionsExclude.push_back(".phys.toy");
	asExtensionsExclude.push_back(".area.toy");

	m_ahPropertyOptions[stoi(sArgs)].DowncastStatic<glgui::CTextField>()->SetAutoCompleteFiles(".", asExtensions, asExtensionsExclude);

	if (m_pPropertyChangedListener)
		m_pfnPropertyChangedCallback(m_pPropertyChangedListener, "");
}

void CEntityPropertiesPanel::TargetChangedCallback(const tstring& sArgs)
{
	CLevel* pLevel = LevelEditor()->GetLevel();

	if (!pLevel)
		return;

	tvector<tstring> asTokens;
	tstrtok(sArgs, asTokens, "<>");

	TAssert(asTokens.size() == 2);
	if (asTokens.size() != 2)
		return;

	tvector<tstring> asTargets;

	for (size_t i = 0; i < pLevel->GetEntityData().size(); i++)
	{
		auto* pEntity = &pLevel->GetEntityData()[i];
		if (!pEntity)
			continue;

		if (!pEntity->GetName().length())
			continue;

		CEntityRegistration* pRegistration = CBaseEntity::GetRegisteredEntity("C"+pEntity->GetClass());
		TAssert(pRegistration);
		if (!pRegistration)
			continue;

		bool bFound = false;
		while (pRegistration)
		{
			if (asTokens[1] == pRegistration->m_pszEntityClass)
			{
				bFound = true;
				break;
			}

			pRegistration = CBaseEntity::GetRegisteredEntity(pRegistration->m_pszParentClass);
		}

		if (!bFound)
			continue;

		asTargets.push_back(pEntity->GetName());
	}

	m_ahPropertyOptions[stoi(sArgs)].DowncastStatic<glgui::CTextField>()->SetAutoCompleteCommands(asTargets);

	if (m_pPropertyChangedListener)
		m_pfnPropertyChangedCallback(m_pPropertyChangedListener, "");
}

void CEntityPropertiesPanel::PropertyChangedCallback(const tstring& sArgs)
{
	if (m_pPropertyChangedListener)
		m_pfnPropertyChangedCallback(m_pPropertyChangedListener, "");
}

void CEntityPropertiesPanel::SetPropertyChangedListener(glgui::IEventListener* pListener, glgui::IEventListener::Callback pfnCallback)
{
	m_pPropertyChangedListener = pListener;
	m_pfnPropertyChangedCallback = pfnCallback;
}

void CEntityPropertiesPanel::SetEntity(class CLevelEntity* pEntity)
{
	m_pEntity = pEntity;
	Layout();
}
