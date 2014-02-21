#include "editorpanel.h"

#include <files.h>

#include <glgui/tree.h>
#include <glgui/slidingpanel.h>
#include <glgui/menu.h>
#include <glgui/textfield.h>
#include <glgui/rootpanel.h>

#include <game/entities/baseentity.h>
#include <game/gameserver.h>
#include <renderer/game_renderer.h>
#include <renderer/game_renderingcontext.h>

CEditorPanel::CEditorPanel()
{
	m_hEntities = AddControl(new glgui::CTree());
	m_hEntities->SetBackgroundColor(Color(0, 0, 0, 100));
	m_hEntities->SetSelectedListener(this, EntitySelected);

	m_hObjectTitle = AddControl(new glgui::CLabel("", "sans-serif", 20));

	m_hSlider = AddControl(new glgui::CSlidingContainer());

	m_hPropertiesSlider = new glgui::CSlidingPanel(m_hSlider, "Properties");
	m_hOutputsSlider = new glgui::CSlidingPanel(m_hSlider, "Outputs");

	m_hPropertiesPanel = m_hPropertiesSlider->AddControl(new CEntityPropertiesPanel(true));
	m_hPropertiesPanel->SetBackgroundColor(Color(10, 10, 10, 50));
	m_hPropertiesPanel->SetPropertyChangedListener(this, PropertyChanged);

	m_hOutputs = m_hOutputsSlider->AddControl(new glgui::CTree());
	m_hOutputs->SetBackgroundColor(Color(0, 0, 0, 100));
	m_hOutputs->SetSelectedListener(this, OutputSelected);

	m_hAddOutput = m_hOutputsSlider->AddControl(new glgui::CButton("Add"));
	m_hAddOutput->SetClickedListener(this, AddOutput);

	m_hRemoveOutput = m_hOutputsSlider->AddControl(new glgui::CButton("Remove"));
	m_hRemoveOutput->SetClickedListener(this, RemoveOutput);

	m_hOutput = m_hOutputsSlider->AddControl(new glgui::CMenu("Choose Output"));

	m_hOutputEntityNameLabel = m_hOutputsSlider->AddControl(new glgui::CLabel("Target Entity:", "sans-serif", 10));
	m_hOutputEntityNameLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_hOutputEntityNameText = m_hOutputsSlider->AddControl(new glgui::CTextField());
	m_hOutputEntityNameText->SetContentsChangedListener(this, TargetEntityChanged);

	m_hInput = m_hOutputsSlider->AddControl(new glgui::CMenu("Choose Input"));

	m_hOutputArgsLabel = m_hOutputsSlider->AddControl(new glgui::CLabel("Arguments:", "sans-serif", 10));
	m_hOutputArgsLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	m_hOutputArgsText = m_hOutputsSlider->AddControl(new glgui::CTextField());
	m_hOutputArgsText->SetContentsChangedListener(this, ArgumentsChanged);
}

void CEditorPanel::Layout()
{
	float flWidth = glgui::CRootPanel::Get()->GetWidth();
	float flHeight = glgui::CRootPanel::Get()->GetHeight();

	float flMenuBarBottom = glgui::CRootPanel::Get()->GetMenuBar()->GetBottom();

	float flCurrLeft = 20;
	float flCurrTop = flMenuBarBottom + 10;

	SetDimensions(flCurrLeft, flCurrTop, 200, flHeight-30-flMenuBarBottom);

	m_hEntities->SetPos(10, 10);
	m_hEntities->SetSize(GetWidth() - 20, 200);

	m_hEntities->ClearTree();

	CLevel* pLevel = LevelEditor()->GetLevel();

	if (pLevel)
	{
		auto& aEntities = pLevel->GetEntityData();
		for (size_t i = 0; i < aEntities.size(); i++)
		{
			auto& oEntity = aEntities[i];

			tstring sName = oEntity.GetParameterValue("Name");
			tstring sModel = oEntity.GetParameterValue("Model");

			if (sName.length())
				m_hEntities->AddNode(oEntity.GetClass() + ": " + sName);
			else if (sModel.length())
				m_hEntities->AddNode(oEntity.GetClass() + " (" + GetFilename(sModel) + ")");
			else
				m_hEntities->AddNode(oEntity.GetClass());
		}
	}

	m_hObjectTitle->SetPos(0, 220);
	m_hObjectTitle->SetSize(GetWidth(), 25);

	float flTempMargin = 5;
	m_hSlider->Layout_AlignTop(m_hObjectTitle, flTempMargin);
	m_hSlider->Layout_FullWidth(flTempMargin);
	m_hSlider->SetBottom(GetHeight() - flTempMargin);

	flTempMargin = 2;
	m_hPropertiesPanel->Layout_AlignTop(nullptr, flTempMargin);
	m_hPropertiesPanel->Layout_FullWidth(flTempMargin);

	LayoutEntity();

	BaseClass::Layout();
}

void CEditorPanel::LayoutEntity()
{
	m_hObjectTitle->SetText("(No Object Selected)");
	m_hPropertiesPanel->SetVisible(false);
	m_hPropertiesPanel->SetEntity(nullptr);

	CLevelEntity* pEntity = GetCurrentEntity();

	if (pEntity)
	{
		if (pEntity->GetName().length())
			m_hObjectTitle->SetText(pEntity->GetClass() + ": " + pEntity->GetName());
		else
			m_hObjectTitle->SetText(pEntity->GetClass());

		m_hPropertiesPanel->SetClass("C" + pEntity->GetClass());
		m_hPropertiesPanel->SetEntity(pEntity);
		m_hPropertiesPanel->SetVisible(true);

		m_hOutputs->ClearTree();

		auto& aEntityOutputs = pEntity->GetOutputs();

		for (size_t i = 0; i < aEntityOutputs.size(); i++)
		{
			auto& oEntityOutput = aEntityOutputs[i];

			m_hOutputs->AddNode(oEntityOutput.m_sOutput + " -> " + oEntityOutput.m_sTargetName + ":" + oEntityOutput.m_sInput);
		}

		m_hOutputs->Layout();
	}

	LayoutOutput();
}

void CEditorPanel::LayoutOutput()
{
	m_hAddOutput->SetEnabled(false);
	m_hRemoveOutput->SetEnabled(false);
	m_hOutput->SetEnabled(false);
	m_hOutputEntityNameText->SetEnabled(false);
	m_hInput->SetEnabled(false);
	m_hOutputArgsText->SetEnabled(false);

	m_hOutputs->Layout_AlignTop();
	m_hOutputs->Layout_FullWidth();
	m_hOutputs->SetHeight(50);

	m_hAddOutput->SetHeight(15);
	m_hAddOutput->Layout_AlignTop(m_hOutputs);
	m_hAddOutput->Layout_Column(2, 0);
	m_hRemoveOutput->SetHeight(15);
	m_hRemoveOutput->Layout_AlignTop(m_hOutputs);
	m_hRemoveOutput->Layout_Column(2, 1);

	m_hOutput->Layout_AlignTop(m_hAddOutput, 10);
	m_hOutput->ClearSubmenus();
	m_hOutput->SetSize(100, 30);
	m_hOutput->CenterX();
	m_hOutput->SetText("Choose Output");

	m_hOutputEntityNameLabel->Layout_AlignTop(m_hOutput);
	m_hOutputEntityNameText->SetContentsChangedListener(nullptr, nullptr);
	m_hOutputEntityNameText->SetTop(m_hOutputEntityNameLabel->GetTop()+12);
	m_hOutputEntityNameText->SetText("");
	m_hOutputEntityNameText->Layout_FullWidth();

	m_hInput->Layout_AlignTop(m_hOutputEntityNameText, 10);
	m_hInput->ClearSubmenus();
	m_hInput->SetSize(100, 30);
	m_hInput->CenterX();
	m_hInput->SetText("Choose Input");

	m_hOutputArgsLabel->Layout_AlignTop(m_hInput);
	m_hOutputArgsText->SetContentsChangedListener(nullptr, nullptr);
	m_hOutputArgsText->SetTop(m_hOutputArgsLabel->GetTop()+12);
	m_hOutputArgsText->SetText("");
	m_hOutputArgsText->Layout_FullWidth();

	CLevelEntity* pEntity = GetCurrentEntity();
	if (!pEntity)
		return;

	m_hAddOutput->SetEnabled(true);
	m_hRemoveOutput->SetEnabled(true);

	CLevelEntity::CLevelEntityOutput* pOutput = GetCurrentOutput();
	if (!pOutput)
		return;

	if (pOutput->m_sTargetName.length())
		m_hOutputEntityNameText->SetText(pOutput->m_sTargetName);

	if (pOutput->m_sArgs.length())
		m_hOutputArgsText->SetText(pOutput->m_sArgs);

	if (pOutput->m_sOutput.length())
		m_hOutput->SetText(pOutput->m_sOutput);

	if (pOutput->m_sInput.length())
		m_hInput->SetText(pOutput->m_sInput);

	CEntityRegistration* pRegistration = CBaseEntity::GetRegisteredEntity("C" + pEntity->GetClass());
	TAssert(pRegistration);
	if (!pRegistration)
		return;

	m_hOutput->SetEnabled(true);
	m_hOutputEntityNameText->SetEnabled(true);
	m_hInput->SetEnabled(true);
	m_hOutputArgsText->SetEnabled(true);

	m_hOutputEntityNameText->SetContentsChangedListener(this, TargetEntityChanged);
	m_hOutputArgsText->SetContentsChangedListener(this, ArgumentsChanged);

	m_hOutput->ClearSubmenus();

	do {
		for (size_t i = 0; i < pRegistration->m_aSaveData.size(); i++)
		{
			auto& oSaveData = pRegistration->m_aSaveData[i];
			if (oSaveData.m_eType != CSaveData::DATA_OUTPUT)
				continue;

			m_hOutput->AddSubmenu(oSaveData.m_pszHandle, this, ChooseOutput);
		}

		if (!pRegistration->m_pszParentClass)
			break;

		pRegistration = CBaseEntity::GetRegisteredEntity(pRegistration->m_pszParentClass);
	} while (pRegistration);

	LayoutInput();
}

void CEditorPanel::LayoutInput()
{
	auto pOutput = GetCurrentOutput();
	if (!pOutput)
		return;

	CLevel* pLevel = LevelEditor()->GetLevel();
	if (!pLevel)
		return;

	CEntityRegistration* pRegistration = nullptr;
	if (m_hOutputEntityNameText->GetText()[0] == '*')
		pRegistration = CBaseEntity::GetRegisteredEntity("C" + m_hOutputEntityNameText->GetText().substr(1));
	else if (m_hOutputEntityNameText->GetText().length())
	{
		CLevelEntity* pTarget = nullptr;

		for (size_t i = 0; i < pLevel->GetEntityData().size(); i++)
		{
			if (!pLevel->GetEntityData()[i].GetName().length())
				continue;

			if (pLevel->GetEntityData()[i].GetName() == m_hOutputEntityNameText->GetText())
			{
				pTarget = &pLevel->GetEntityData()[i];
				break;
			}
		}

		if (pTarget)
			pRegistration = CBaseEntity::GetRegisteredEntity("C" + pTarget->GetClass());
	}

	m_hInput->ClearSubmenus();

	if (pRegistration)
	{
		do {
			for (auto it = pRegistration->m_aInputs.begin(); it != pRegistration->m_aInputs.end(); it++)
				m_hInput->AddSubmenu(it->first, this, ChooseInput);

			if (!pRegistration->m_pszParentClass)
				break;

			pRegistration = CBaseEntity::GetRegisteredEntity(pRegistration->m_pszParentClass);
		} while (pRegistration);
	}
}

void CEditorPanel::Paint(float x, float y, float w, float h)
{
	Matrix4x4 mFontProjection = Matrix4x4::ProjectOrthographic(0, glgui::RootPanel()->GetWidth(), 0, glgui::RootPanel()->GetHeight(), -1, 1);

	CLevel* pLevel = LevelEditor()->GetLevel();
	if (pLevel)
	{
		for (size_t i = 0; i < pLevel->GetEntityData().size(); i++)
		{
			CLevelEntity* pEnt = &pLevel->GetEntityData()[i];

			if (m_hEntities->GetSelectedNodeId() != i && !pEnt->GetName().length())
				continue;

			Vector vecCenter = pEnt->GetGlobalTransform().GetTranslation();
			if (!GameServer()->GetRenderer()->IsSphereInFrustum(vecCenter, pEnt->GetBoundingBox().Size().Length()/2))
				continue;

			if (pEnt->GetMaterialModel().IsValid() && !pEnt->ShouldDisableBackCulling())
			{
				Vector vecToCenter = vecCenter - GameServer()->GetRenderer()->GetCameraPosition();

				if (pEnt->ShouldRenderInverted())
				{
					if (vecToCenter.Dot(pEnt->GetGlobalTransform().GetForwardVector()) > 0)
						continue;
				}
				else
				{
					if (vecToCenter.Dot(pEnt->GetGlobalTransform().GetForwardVector()) < 0)
						continue;
				}
			}

			Vector vecScreen = GameServer()->GetRenderer()->ScreenPosition(vecCenter);

			tstring sText;
			if (pEnt->GetName().length())
				sText = pEnt->GetClass() + ": " + pEnt->GetName();
			else
				sText = pEnt->GetClass();

			float flWidth = glgui::CLabel::GetTextWidth(sText, sText.length(), "sans-serif", 10);
			float flHeight = glgui::CLabel::GetFontHeight("sans-serif", 10);
			float flAscender = glgui::CLabel::GetFontAscender("sans-serif", 10);

			if (m_hEntities->GetSelectedNodeId() == i)
				PaintRect(vecScreen.x - flWidth/2 - 3, vecScreen.y+20-flAscender - 3, flWidth + 6, flHeight + 6, Color(0, 0, 0, 150));

			CRenderingContext c(nullptr, true);

			c.SetBlend(BLEND_ALPHA);
			c.UseProgram("text");
			c.SetProjection(mFontProjection);

			if (m_hEntities->GetSelectedNodeId() == i)
				c.SetUniform("vecColor", Color(255, 0, 0, 255));
			else
				c.SetUniform("vecColor", Color(255, 255, 255, 200));

			c.Translate(Vector(vecScreen.x - flWidth/2, glgui::RootPanel()->GetBottom()-vecScreen.y-20, 0));
			c.SetUniform("bScissor", false);

			c.RenderText(sText, sText.length(), "sans-serif", 10);
		}
	}

	BaseClass::Paint(x, y, w, h);
}

CLevelEntity* CEditorPanel::GetCurrentEntity()
{
	CLevel* pLevel = LevelEditor()->GetLevel();

	if (!pLevel)
		return nullptr;

	auto& aEntities = pLevel->GetEntityData();

	if (m_hEntities->GetSelectedNodeId() >= aEntities.size())
		return nullptr;

	return &aEntities[m_hEntities->GetSelectedNodeId()];
}

CLevelEntity::CLevelEntityOutput* CEditorPanel::GetCurrentOutput()
{
	CLevelEntity* pEntity = GetCurrentEntity();

	if (!pEntity)
		return nullptr;

	auto& aEntityOutputs = pEntity->GetOutputs();

	if (m_hOutputs->GetSelectedNodeId() >= aEntityOutputs.size())
		return nullptr;

	return &aEntityOutputs[m_hOutputs->GetSelectedNodeId()];
}

void CEditorPanel::EntitySelectedCallback(const tstring& sArgs)
{
	LayoutEntity();

	LevelEditor()->EntitySelected();

	CLevel* pLevel = LevelEditor()->GetLevel();

	if (!pLevel)
		return;

	auto& aEntities = pLevel->GetEntityData();

	if (m_hEntities->GetSelectedNodeId() < aEntities.size())
	{
		Manipulator()->Activate(LevelEditor(), aEntities[m_hEntities->GetSelectedNodeId()].GetGlobalTRS(), "Entity " + sprintf("%d", m_hEntities->GetSelectedNodeId()));
	}
	else
	{
		Manipulator()->Deactivate();
	}
}

void CEditorPanel::PropertyChangedCallback(const tstring& sArgs)
{
	CLevelEntity* pEntity = GetCurrentEntity();
	if (!pEntity)
		return;

	if (pEntity)
	{
		CLevelEditor::PopulateLevelEntityFromPanel(pEntity, m_hPropertiesPanel);

		if (Manipulator()->IsActive())
			Manipulator()->SetTRS(pEntity->GetGlobalTRS());
	}
}

void CEditorPanel::OutputSelectedCallback(const tstring& sArgs)
{
	LayoutOutput();
}

void CEditorPanel::AddOutputCallback(const tstring& sArgs)
{
	CLevelEntity* pEntity = GetCurrentEntity();
	if (!pEntity)
		return;

	pEntity->GetOutputs().push_back();

	LayoutEntity();

	m_hOutputs->SetSelectedNode(pEntity->GetOutputs().size()-1);
}

void CEditorPanel::RemoveOutputCallback(const tstring& sArgs)
{
	CLevelEntity* pEntity = GetCurrentEntity();
	if (!pEntity)
		return;

	pEntity->GetOutputs().erase(pEntity->GetOutputs().begin()+m_hOutputs->GetSelectedNodeId());

	LayoutEntity();
}

void CEditorPanel::ChooseOutputCallback(const tstring& sArgs)
{
	m_hOutput->Pop(true, true);

	auto pOutput = GetCurrentOutput();
	if (!pOutput)
		return;

	tvector<tstring> asTokens;
	tstrtok(sArgs, asTokens);
	pOutput->m_sOutput = asTokens[1];
	m_hOutput->SetText(pOutput->m_sOutput);

	LayoutInput();
}

void CEditorPanel::TargetEntityChangedCallback(const tstring& sArgs)
{
	CLevel* pLevel = LevelEditor()->GetLevel();
	if (!pLevel)
		return;

	auto pOutput = GetCurrentOutput();
	if (!pOutput)
		return;

	pOutput->m_sTargetName = m_hOutputEntityNameText->GetText();

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

		asTargets.push_back(pEntity->GetName());
	}

	m_hOutputEntityNameText->SetAutoCompleteCommands(asTargets);

	LayoutInput();
}

void CEditorPanel::ChooseInputCallback(const tstring& sArgs)
{
	m_hInput->Pop(true, true);

	auto pOutput = GetCurrentOutput();
	if (!pOutput)
		return;

	tvector<tstring> asTokens;
	tstrtok(sArgs, asTokens);
	pOutput->m_sInput = asTokens[1];
	m_hInput->SetText(pOutput->m_sInput);
}

void CEditorPanel::ArgumentsChangedCallback(const tstring& sArgs)
{
	auto pOutput = GetCurrentOutput();
	if (!pOutput)
		return;

	pOutput->m_sArgs = m_hOutputArgsText->GetText();
}
