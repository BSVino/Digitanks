#include "toyeditor.h"

#include <iostream>
#include <fstream>

#include <tinker_platform.h>
#include <files.h>
#include <tvector.h>

#include <glgui/rootpanel.h>
#include <glgui/movablepanel.h>
#include <glgui/textfield.h>
#include <glgui/button.h>
#include <glgui/menu.h>
#include <glgui/filedialog.h>
#include <glgui/tree.h>
#include <glgui/slidingpanel.h>
#include <glgui/checkbox.h>
#include <tinker/application.h>
#include <models/models.h>
#include <renderer/game_renderingcontext.h>
#include <renderer/game_renderer.h>
#include <game/gameserver.h>
#include <tinker/keys.h>
#include <ui/gamewindow.h>
#include <tools/toybuilder/geppetto.h>
#include <datamanager/dataserializer.h>
#include <textures/texturelibrary.h>
#include <textures/materiallibrary.h>
#include <toys/toy.h>
#include <modelconverter/modelconverter.h>
#include <tools/manipulator/manipulator.h>

#include "workbench.h"

using namespace glgui;

REGISTER_WORKBENCH_TOOL(ToyEditor);

CCreateToySourcePanel::CCreateToySourcePanel()
	: glgui::CMovablePanel("Create Toy Source Tool")
{
	SetBackgroundColor(Color(0, 0, 0, 255));
	SetHeaderColor(Color(100, 100, 100, 255));
	SetBorder(glgui::CPanel::BT_SOME);

	m_pToyFileLabel = new glgui::CLabel("Output Toy File:", "sans-serif", 10);
	m_pToyFileLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pToyFileLabel);
	m_pToyFileText = new glgui::CTextField();
	m_pToyFileText->SetContentsChangedListener(this, ToyChanged);
	AddControl(m_pToyFileText);

	m_pSourceFileLabel = new glgui::CLabel("Source File:", "sans-serif", 10);
	m_pSourceFileLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pSourceFileLabel);
	m_pSourceFileText = new glgui::CTextField();
	m_pSourceFileText->SetContentsChangedListener(this, SourceChanged);
	AddControl(m_pSourceFileText);

	m_pWarnings = new glgui::CLabel("");
	m_pWarnings->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pWarnings);

	m_pCreate = new glgui::CButton("Create");
	m_pCreate->SetClickedListener(this, Create);
	AddControl(m_pCreate);
}

void CCreateToySourcePanel::Layout()
{
	float flTop = 20;
	m_pToyFileLabel->SetLeft(15);
	m_pToyFileLabel->SetTop(flTop);
	m_pToyFileText->SetWidth(GetWidth()-30);
	m_pToyFileText->CenterX();
	m_pToyFileText->SetTop(flTop+12);

	flTop += 43;

	m_pSourceFileLabel->SetLeft(15);
	m_pSourceFileLabel->SetTop(flTop);
	m_pSourceFileText->SetWidth(GetWidth()-30);
	m_pSourceFileText->CenterX();
	m_pSourceFileText->SetTop(flTop+12);

	flTop += 43;

	m_pWarnings->SetLeft(15);
	m_pWarnings->SetTop(flTop);
	m_pWarnings->SetWidth(GetWidth()-30);
	m_pWarnings->CenterX();
	m_pWarnings->SetWrap(true);
	m_pWarnings->SetBottom(GetBottom() - 60);

	m_pCreate->SetTop(GetHeight() - 45);
	m_pCreate->CenterX();

	BaseClass::Layout();

	FileNamesChanged();
}

void CCreateToySourcePanel::ToyChangedCallback(const tstring& sArgs)
{
	FileNamesChanged();

	if (!m_pToyFileText->GetText().length())
		return;

	tvector<tstring> asExtensions;
	tvector<tstring> asExtensionsExclude;

	asExtensions.push_back(".toy");
	asExtensionsExclude.push_back(".mesh.toy");
	asExtensionsExclude.push_back(".phys.toy");
	asExtensionsExclude.push_back(".area.toy");

	m_pToyFileText->SetAutoCompleteFiles(".", asExtensions, asExtensionsExclude);
}

void CCreateToySourcePanel::SourceChangedCallback(const tstring& sArgs)
{
	FileNamesChanged();

	if (!m_pSourceFileText->GetText().length())
		return;

	tvector<tstring> asExtensions;
	asExtensions.push_back(".txt");

	m_pSourceFileText->SetAutoCompleteFiles("../sources", asExtensions);
}

tstring CCreateToySourcePanel::GetToyFileName()
{
	tstring sToyFile = m_pToyFileText->GetText();

	if (!sToyFile.endswith(".toy"))
		sToyFile.append(".toy");

	return sToyFile;
}

tstring CCreateToySourcePanel::GetSourceFileName()
{
	tstring sSourceFile = m_pSourceFileText->GetText();

	if (!sSourceFile.endswith(".txt"))
		sSourceFile.append(".txt");

	return "../sources/" + sSourceFile;
}

void CCreateToySourcePanel::FileNamesChanged()
{
	m_pWarnings->SetText("");

	tstring sToyFile = GetToyFileName();
	if (IsFile(sToyFile))
		m_pWarnings->SetText("WARNING: This toy file already exists. It will be overwritten when the new source file is built.");

	tstring sSourceFile = GetSourceFileName();
	if (IsFile(sSourceFile))
	{
		if (m_pWarnings->GetText().length())
			m_pWarnings->AppendText("\n\n");
		m_pWarnings->AppendText("WARNING: This source file already exists. It will be overwritten.");
	}

	m_pCreate->SetVisible(false);

	if (m_pSourceFileText->GetText().length() && m_pToyFileText->GetText().length())
		m_pCreate->SetVisible(true);
}

void CCreateToySourcePanel::CreateCallback(const tstring& sArgs)
{
	if (!m_pSourceFileText->GetText().length() || !m_pToyFileText->GetText().length())
		return;

	ToyEditor()->NewToy();
	ToyEditor()->GetToyToModify().m_sFilename = GetSourceFileName();
	ToyEditor()->GetToyToModify().m_sToyFile = GetToyFileName();

	SetVisible(false);

	ToyEditor()->Layout();
}

CChooseScenePanel::CChooseScenePanel(CSourcePanel* pSourcePanel)
	: glgui::CMovablePanel("Choose a Scene")
{
	SetBackgroundColor(Color(0, 0, 0, 255));
	SetHeaderColor(Color(100, 100, 100, 255));
	SetBorder(glgui::CPanel::BT_SOME);

	m_hSceneTree = AddControl(new glgui::CTree(CWorkbench::Get()->GetArrowTexture()));
	m_hSceneTree->SetBackgroundColor(Color(0, 0, 0, 100));
	m_hSceneTree->SetConfirmedListener(pSourcePanel, CSourcePanel::SceneAreaConfirmed);
}

void CChooseScenePanel::Layout()
{
	m_hSceneTree->Layout_AlignTop();
	m_hSceneTree->Layout_FullWidth();
	m_hSceneTree->SetHeight(GetHeight()-20);

	m_hSceneTree->ClearTree();

	auto pSceneArea = ToyEditor()->GetSourcePanel()->GetCurrentSceneArea();

	if (!pSceneArea)
		return;

	auto pConversionScene = ToyEditor()->FindLoadedSceneFromFile(pSceneArea->m_sFilename);

	for (size_t i = 0; i < pConversionScene->GetNumScenes(); i++)
		AddSceneToTree(nullptr, pConversionScene->GetScene(i));

	BaseClass::Layout();
}

void CChooseScenePanel::AddSceneToTree(CTreeNode* pParentTreeNode, CConversionSceneNode* pSceneNode)
{
	CTreeNode* pNode;
	if (pParentTreeNode)
	{
		size_t iNode = pParentTreeNode->AddNode(pSceneNode->GetName());
		pNode = pParentTreeNode->GetNode(iNode);
	}
	else
	{
		size_t iNode = m_hSceneTree->AddNode(pSceneNode->GetName());
		pNode = m_hSceneTree->GetNode(iNode);
	}

	for (size_t i = 0; i < pSceneNode->GetNumChildren(); i++)
		AddSceneToTree(pNode, pSceneNode->GetChild(i));
}

CSourcePanel::CSourcePanel()
{
	SetBackgroundColor(Color(0, 0, 0, 150));
	SetBorder(glgui::CPanel::BT_SOME);

	m_hFilename = AddControl(new glgui::CLabel("", "sans-serif", 16));

	m_hToyFileLabel = AddControl(new glgui::CLabel("Output Toy File: ", "sans-serif", 10));
	m_hToyFileLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);

	m_hToyFileText = AddControl(new glgui::CTextField());
	m_hToyFileText->SetContentsChangedListener(this, ToyFileChanged);

	m_hMeshMenu = AddControl(new glgui::CMenu("Mesh: "));
	m_bMesh = true;
	m_hMeshMenu->AddSubmenu("Mesh", this, MeshSource);
	m_hMeshMenu->AddSubmenu("Material", this, MaterialSource);
	m_hMeshMenu->SetFont("sans-serif", 10);
	m_hMeshMenu->SetAlign(glgui::CLabel::TA_MIDDLECENTER);

	m_hMeshText = AddControl(new glgui::CTextField());
	m_hMeshText->SetContentsChangedListener(this, ModelChanged, "mesh");

	m_hPhysLabel = AddControl(new glgui::CLabel("Physics: ", "sans-serif", 10));
	m_hPhysLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);

	m_hPhysText = AddControl(new glgui::CTextField());
	m_hPhysText->SetContentsChangedListener(this, PhysicsChanged, "phys");

	m_hUseLocalTransformsCheck = AddControl(new glgui::CCheckBox());
	m_hUseLocalTransformsCheck->SetClickedListener(this, LocalTransformsChanged);
	m_hUseLocalTransformsCheck->SetUnclickedListener(this, LocalTransformsChanged);

	m_hUseLocalTransformsLabel = AddControl(new glgui::CLabel("Use local transforms?", "sans-serif", 10));
	m_hUseLocalTransformsLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);

	m_hSlider = AddControl(new glgui::CSlidingContainer());

	m_hPhysicsSlider = new glgui::CSlidingPanel(m_hSlider, "Physics Shapes");
	m_hAreasSlider = new glgui::CSlidingPanel(m_hSlider, "Areas");

	m_hPhysicsShapes = m_hPhysicsSlider->AddControl(new glgui::CTree());
	m_hPhysicsShapes->SetBackgroundColor(Color(0, 0, 0, 100));
	m_hPhysicsShapes->SetSelectedListener(this, PhysicsAreaSelected);

	m_hNewPhysicsShape = m_hPhysicsSlider->AddControl(new glgui::CButton("New Shape", false, "sans-serif", 10));
	m_hNewPhysicsShape->SetClickedListener(this, NewPhysicsShape);

	m_hDeletePhysicsShape = m_hPhysicsSlider->AddControl(new glgui::CButton("Delete Shape", false, "sans-serif", 10));
	m_hDeletePhysicsShape->SetClickedListener(this, DeletePhysicsShape);

	m_hAreas = m_hAreasSlider->AddControl(new glgui::CTree());
	m_hAreas->SetBackgroundColor(Color(0, 0, 0, 100));
	m_hAreas->SetSelectedListener(this, SceneAreaSelected);

	m_hNewArea = m_hAreasSlider->AddControl(new glgui::CButton("New Area", false, "sans-serif", 10));
	m_hNewArea->SetClickedListener(this, NewSceneArea);

	m_hDeleteArea = m_hAreasSlider->AddControl(new glgui::CButton("Delete Area", false, "sans-serif", 10));
	m_hDeleteArea->SetClickedListener(this, DeleteSceneArea);

	m_hAreaNameLabel = m_hAreasSlider->AddControl(new glgui::CLabel("Name: ", "sans-serif", 10));
	m_hAreaNameLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);

	m_hAreaNameText = m_hAreasSlider->AddControl(new glgui::CTextField());
	m_hAreaNameText->SetContentsChangedListener(this, SceneAreaNameChanged);

	m_hAreaSourceFileLabel = m_hAreasSlider->AddControl(new glgui::CLabel("Source File: ", "sans-serif", 10));
	m_hAreaSourceFileLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);

	m_hAreaSourceFileText = m_hAreasSlider->AddControl(new glgui::CTextField());
	m_hAreaSourceFileText->SetContentsChangedListener(this, SceneAreaMeshSourceChanged);

	m_hAreaMeshLabel = m_hAreasSlider->AddControl(new glgui::CLabel("Mesh: ", "sans-serif", 10));
	m_hAreaMeshLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);

	m_hAreaMeshText = m_hAreasSlider->AddControl(new glgui::CTextField());
	m_hAreaMeshText->SetContentsChangedListener(this, SceneAreaModelChanged, "mesh");

	m_hAreaMeshChoose = m_hAreasSlider->AddControl(new glgui::CButton("..."));
	m_hAreaMeshChoose->SetClickedListener(this, SceneAreaMeshChoose);

	m_hAreaPhysLabel = m_hAreasSlider->AddControl(new glgui::CLabel("Physics: ", "sans-serif", 10));
	m_hAreaPhysLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);

	m_hAreaPhysText = m_hAreasSlider->AddControl(new glgui::CTextField());
	m_hAreaPhysText->SetContentsChangedListener(this, SceneAreaPhysicsChanged, "phys");

	m_hAreaPhysChoose = m_hAreasSlider->AddControl(new glgui::CButton("..."));
	m_hAreaPhysChoose->SetClickedListener(this, SceneAreaPhysChoose);

	m_hSave = AddControl(new glgui::CButton("Save"));
	m_hSave->SetClickedListener(this, Save);

	m_hBuild = AddControl(new glgui::CButton("Build"));
	m_hBuild->SetClickedListener(this, Build);

	m_hSceneChooser = new CChooseScenePanel(this);
	m_hSceneChooser->SetVisible(false);
}

void CSourcePanel::SetVisible(bool bVis)
{
	if (bVis && !IsVisible())
		UpdateFields();

	BaseClass::SetVisible(bVis);
}

void CSourcePanel::Layout()
{
	float flWidth = glgui::CRootPanel::Get()->GetWidth();
	float flHeight = glgui::CRootPanel::Get()->GetHeight();

	float flMenuBarBottom = glgui::CRootPanel::Get()->GetMenuBar()->GetBottom();

	float flCurrLeft = 20;
	float flCurrTop = flMenuBarBottom + 10;

	SetDimensions(flCurrLeft, flCurrTop, 200, flHeight-30-flMenuBarBottom);

	m_hFilename->Layout_AlignTop();
	m_hFilename->Layout_FullWidth(0);

	BaseClass::Layout();

	const CToySource* pToySource = &ToyEditor()->GetToy();

	if (!pToySource->m_sFilename.length())
		return;

	LayoutFilename();

	m_hToyFileLabel->Layout_AlignTop(m_hFilename);
	m_hToyFileLabel->SetWidth(10);
	m_hToyFileLabel->SetHeight(1);
	m_hToyFileLabel->EnsureTextFits();
	m_hToyFileLabel->Layout_FullWidth();

	m_hToyFileText->Layout_FullWidth();
	m_hToyFileText->SetTop(m_hToyFileLabel->GetTop()+12);

	float flControlMargin = 12;

	m_hMeshMenu->Layout_AlignTop(m_hToyFileText, flControlMargin);
	m_hMeshMenu->SetWidth(10);
	m_hMeshMenu->SetHeight(1);
	m_hMeshMenu->EnsureTextFits();
	m_hMeshMenu->SetLeft(m_hToyFileText->GetLeft());

	m_hMeshText->Layout_FullWidth();
	m_hMeshText->SetTop(m_hMeshMenu->GetBottom());

	m_hPhysLabel->Layout_AlignTop(m_hMeshText, flControlMargin);
	m_hPhysLabel->SetWidth(10);
	m_hPhysLabel->SetHeight(1);
	m_hPhysLabel->EnsureTextFits();
	m_hPhysLabel->Layout_FullWidth();

	m_hPhysText->Layout_FullWidth();
	m_hPhysText->SetTop(m_hPhysLabel->GetTop()+12);

	m_hUseLocalTransformsCheck->Layout_AlignTop(m_hPhysText, flControlMargin);
	m_hUseLocalTransformsCheck->SetLeft(m_hPhysText->GetLeft());

	m_hUseLocalTransformsLabel->Layout_AlignTop(m_hPhysText, flControlMargin);
	m_hUseLocalTransformsLabel->SetLeft(m_hUseLocalTransformsCheck->GetRight());

	float flTempMargin = 5;
	m_hSlider->Layout_AlignTop(m_hUseLocalTransformsLabel, flTempMargin);
	m_hSlider->Layout_FullWidth(flTempMargin);

	m_hPhysicsShapes->Layout_FullWidth();
	m_hPhysicsShapes->SetTop(12);
	m_hPhysicsShapes->SetHeight(100);

	m_hPhysicsShapes->ClearTree();
	for (size_t i = 0; i < pToySource->m_aShapes.size(); i++)
		m_hPhysicsShapes->AddNode("Box");

	m_hNewPhysicsShape->SetHeight(20);
	m_hNewPhysicsShape->Layout_Column(2, 0);
	m_hNewPhysicsShape->Layout_AlignTop(m_hPhysicsShapes, 5);
	m_hDeletePhysicsShape->SetHeight(20);
	m_hDeletePhysicsShape->Layout_Column(2, 1);
	m_hDeletePhysicsShape->Layout_AlignTop(m_hPhysicsShapes, 5);
	m_hDeletePhysicsShape->SetEnabled(false);

	m_hAreas->Layout_FullWidth();
	m_hAreas->SetTop(12);
	m_hAreas->SetHeight(100);

	m_hAreas->ClearTree();
	for (size_t i = 0; i < pToySource->m_aAreas.size(); i++)
		m_hAreas->AddNode(pToySource->m_aAreas[i].m_sName);

	m_hNewArea->SetHeight(20);
	m_hNewArea->Layout_Column(2, 0);
	m_hNewArea->Layout_AlignTop(m_hAreas, 5);
	m_hDeleteArea->SetHeight(20);
	m_hDeleteArea->Layout_Column(2, 1);
	m_hDeleteArea->Layout_AlignTop(m_hAreas, 5);
	m_hDeleteArea->SetEnabled(false);

	m_hAreaNameLabel->Layout_AlignTop(m_hNewArea, flControlMargin);
	m_hAreaNameLabel->SetWidth(10);
	m_hAreaNameLabel->SetHeight(1);
	m_hAreaNameLabel->EnsureTextFits();
	m_hAreaNameLabel->Layout_FullWidth();

	m_hAreaNameText->Layout_FullWidth();
	m_hAreaNameText->SetTop(m_hAreaNameLabel->GetTop()+12);
	m_hAreaNameText->SetEnabled(!!GetCurrentSceneArea());

	m_hAreaSourceFileLabel->Layout_AlignTop(m_hAreaNameText, flControlMargin);
	m_hAreaSourceFileLabel->SetWidth(10);
	m_hAreaSourceFileLabel->SetHeight(1);
	m_hAreaSourceFileLabel->EnsureTextFits();
	m_hAreaSourceFileLabel->Layout_FullWidth();

	m_hAreaSourceFileText->Layout_FullWidth();
	m_hAreaSourceFileText->SetTop(m_hAreaSourceFileLabel->GetTop()+12);
	m_hAreaSourceFileText->SetEnabled(!!GetCurrentSceneArea());

	m_hAreaMeshLabel->Layout_AlignTop(m_hAreaSourceFileText, flControlMargin);
	m_hAreaMeshLabel->SetWidth(10);
	m_hAreaMeshLabel->SetHeight(1);
	m_hAreaMeshLabel->EnsureTextFits();
	m_hAreaPhysLabel->Layout_FullWidth();

	m_hAreaMeshChoose->SetTop(m_hAreaMeshLabel->GetTop()+12);
	m_hAreaMeshChoose->SetWidth(10);
	m_hAreaMeshChoose->EnsureTextFits();
	m_hAreaMeshChoose->SetHeight(m_hAreaMeshText->GetHeight());
	m_hAreaMeshChoose->Layout_AlignRight();
	m_hAreaMeshChoose->SetEnabled(!!GetCurrentSceneArea());

	m_hAreaMeshText->Layout_FullWidth();
	m_hAreaMeshText->SetTop(m_hAreaMeshLabel->GetTop()+12);
	m_hAreaMeshText->SetEnabled(!!GetCurrentSceneArea());
	m_hAreaMeshText->SetRight(m_hAreaMeshChoose->GetLeft() - flControlMargin);

	m_hAreaPhysLabel->Layout_AlignTop(m_hAreaMeshText, flControlMargin);
	m_hAreaPhysLabel->SetWidth(10);
	m_hAreaPhysLabel->SetHeight(1);
	m_hAreaPhysLabel->EnsureTextFits();
	m_hAreaPhysLabel->Layout_FullWidth();

	m_hAreaPhysChoose->SetTop(m_hAreaPhysLabel->GetTop()+12);
	m_hAreaPhysChoose->SetWidth(10);
	m_hAreaPhysChoose->EnsureTextFits();
	m_hAreaPhysChoose->SetHeight(m_hAreaPhysText->GetHeight());
	m_hAreaPhysChoose->Layout_AlignRight();
	m_hAreaPhysChoose->SetEnabled(!!GetCurrentSceneArea());

	m_hAreaPhysText->Layout_FullWidth();
	m_hAreaPhysText->SetTop(m_hAreaPhysLabel->GetTop()+12);
	m_hAreaPhysText->SetEnabled(!!GetCurrentSceneArea());
	m_hAreaPhysText->SetRight(m_hAreaPhysChoose->GetLeft() - flControlMargin);

	m_hSave->Layout_Column(2, 0);
	m_hSave->Layout_AlignBottom();
	m_hBuild->Layout_Column(2, 1);
	m_hBuild->Layout_AlignBottom();

	m_hSlider->SetBottom(m_hSave->GetTop() - flTempMargin);

	if (pToySource->m_aShapes.size() == 0 && pToySource->m_aAreas.size() > 0)
		m_hSlider->SetCurrent(1);
	else if (pToySource->m_aShapes.size() > 0 && pToySource->m_aAreas.size() == 0)
		m_hSlider->SetCurrent(0);

	BaseClass::Layout();

	Manipulator()->Deactivate();
}

void CSourcePanel::LayoutFilename()
{
	const CToySource* pToySource = &ToyEditor()->GetToy();

	if (!pToySource->m_sFilename.length())
		return;

	tstring sFilename = pToySource->m_sFilename;
	tstring sAbsoluteSourcePath = FindAbsolutePath("../sources/");
	tstring sAbsoluteFilename = FindAbsolutePath(sFilename);
	if (sAbsoluteFilename.find(sAbsoluteSourcePath) == 0)
		sFilename = ToForwardSlashes(sAbsoluteFilename.substr(sAbsoluteSourcePath.length()));
	m_hFilename->SetText(sFilename);
	if (!ToyEditor()->IsSaved())
		m_hFilename->AppendText(" *");
}

void CSourcePanel::UpdateFields()
{
	m_hToyFileText->SetText(ToyEditor()->GetToy().m_sToyFile);
	m_hMeshText->SetText(ToyEditor()->GetToy().m_sMesh);
	m_hPhysText->SetText(ToyEditor()->GetToy().m_sPhys);

	m_hUseLocalTransformsCheck->SetState(ToyEditor()->GetToy().m_bUseLocalTransforms, false);

	tstring sMesh = ToyEditor()->GetToy().m_sMesh;
	if (sMesh.length() >= 4)
	{
		tstring sExtension = sMesh.substr(sMesh.length()-4);
		if (sExtension == ".mat")
		{
			m_bMesh = false;
			m_hMeshMenu->SetText("Material: ");
		}
		else
		{
			m_bMesh = true;
			m_hMeshMenu->SetText("Mesh: ");
		}
	}

	auto pSceneArea = GetCurrentSceneArea();

	m_hDeleteArea->SetEnabled(!!pSceneArea);
	m_hAreaNameText->SetEnabled(!!pSceneArea);
	m_hAreaSourceFileText->SetEnabled(!!pSceneArea);
	m_hAreaMeshText->SetEnabled(!!pSceneArea);
	m_hAreaPhysText->SetEnabled(!!pSceneArea);
	m_hAreaMeshChoose->SetEnabled(!!pSceneArea);
	m_hAreaPhysChoose->SetEnabled(!!pSceneArea);

	if (pSceneArea)
	{
		m_hAreaNameText->SetText(pSceneArea->m_sName);
		m_hAreaSourceFileText->SetText(pSceneArea->m_sFilename);
		m_hAreaMeshText->SetText(pSceneArea->m_sMesh);
		m_hAreaPhysText->SetText(pSceneArea->m_sPhys);
	}
	else
	{
		m_hAreaNameText->SetText("");
		m_hAreaSourceFileText->SetText("");
		m_hAreaMeshText->SetText("");
		m_hAreaPhysText->SetText("");
	}
}

void CSourcePanel::SetupChooser(CControl<CTextField> hControl)
{
	m_hChoosingTextField = hControl;
	m_hSceneChooser->SetVisible(true);

	FRect rChooseButton = m_hChoosingTextField->GetAbsDimensions();
	m_hSceneChooser->SetPos(rChooseButton.x + rChooseButton.w + 10, rChooseButton.y);

	if (m_hSceneChooser->GetBottom() > glgui::RootPanel()->GetHeight())
		m_hSceneChooser->SetTop(glgui::RootPanel()->GetHeight() - m_hSceneChooser->GetHeight());
}

void CSourcePanel::SetModelSourcesAutoComplete(glgui::CTextField* pField)
{
	tvector<tstring> asExtensions = CModelConverter::GetReadFormats();
	asExtensions.push_back(".png");

	pField->SetAutoCompleteFiles(GetDirectory(ToyEditor()->GetToy().m_sFilename), asExtensions);
}

CToySource::CSceneArea* CSourcePanel::GetCurrentSceneArea() const
{
	size_t iArea = m_hAreas->GetSelectedNodeId();

	if (iArea >= ToyEditor()->GetToyToModify().m_aAreas.size())
		return nullptr;

	return &ToyEditor()->GetToyToModify().m_aAreas[iArea];
}

void CSourcePanel::ToyFileChangedCallback(const tstring& sArgs)
{
	ToyEditor()->GetToyToModify().m_sToyFile = m_hToyFileText->GetText();

	if (!m_hToyFileText->GetText().length())
		return;

	tvector<tstring> asExtensions;
	tvector<tstring> asExtensionsExclude;

	asExtensions.push_back(".toy");
	asExtensionsExclude.push_back(".mesh.toy");
	asExtensionsExclude.push_back(".phys.toy");
	asExtensionsExclude.push_back(".area.toy");

	m_hToyFileText->SetAutoCompleteFiles(".", asExtensions, asExtensionsExclude);
}

void CSourcePanel::MeshSourceCallback(const tstring& sArgs)
{
	m_hMeshMenu->SetText("Mesh:");
	m_bMesh = true;
	m_hMeshMenu->CloseMenu();
	Layout();
}

void CSourcePanel::MaterialSourceCallback(const tstring& sArgs)
{
	m_hMeshMenu->SetText("Material:");
	m_bMesh = false;
	m_hMeshMenu->CloseMenu();
	Layout();
}

void CSourcePanel::ModelChangedCallback(const tstring& sArgs)
{
	ToyEditor()->GetToyToModify().m_sMesh = m_hMeshText->GetText();

	if (m_bMesh)
		SetModelSourcesAutoComplete(m_hMeshText);
	else
	{
		tvector<tstring> asExtensions;
		asExtensions.push_back(".mat");

		m_hMeshText->SetAutoCompleteFiles(FindAbsolutePath("."), asExtensions);
	}

	ToyEditor()->Layout();
}

void CSourcePanel::PhysicsChangedCallback(const tstring& sArgs)
{
	ToyEditor()->GetToyToModify().m_sPhys = m_hPhysText->GetText();
	SetModelSourcesAutoComplete(m_hPhysText);

	ToyEditor()->Layout();
}

void CSourcePanel::LocalTransformsChangedCallback(const tstring& sArgs)
{
	ToyEditor()->GetToyToModify().m_bUseLocalTransforms = m_hUseLocalTransformsCheck->GetState();

	ToyEditor()->ReloadPreview();

	ToyEditor()->Layout();
}

void CSourcePanel::PhysicsAreaSelectedCallback(const tstring& sArgs)
{
	if (m_hPhysicsShapes->GetSelectedNodeId() < ToyEditor()->GetToy().m_aShapes.size())
	{
		Manipulator()->Activate(ToyEditor(), ToyEditor()->GetToy().m_aShapes[m_hPhysicsShapes->GetSelectedNodeId()].m_trsTransform, "PhysicsShape " + sprintf("%d", m_hPhysicsShapes->GetSelectedNodeId()));
		m_hDeletePhysicsShape->SetEnabled(true);
	}
	else
	{
		Manipulator()->Deactivate();
		m_hDeletePhysicsShape->SetEnabled(false);
	}
}

void CSourcePanel::NewPhysicsShapeCallback(const tstring& sArgs)
{
	auto& oPhysicsShape = ToyEditor()->GetToyToModify().m_aShapes.push_back();

	Layout();
}

void CSourcePanel::DeletePhysicsShapeCallback(const tstring& sArgs)
{
	if (m_hPhysicsShapes->GetSelectedNodeId() >= ToyEditor()->GetToy().m_aShapes.size())
		return;

	size_t iSelected = m_hPhysicsShapes->GetSelectedNodeId();	// Grab before GetToyToModify() since a Layout() is called which resecs the shapes list.
	auto pToy = &ToyEditor()->GetToyToModify();
	pToy->m_aShapes.erase(pToy->m_aShapes.begin()+iSelected);

	Layout();
}

void CSourcePanel::SceneAreaSelectedCallback(const tstring& sArgs)
{
	UpdateFields();
}

void CSourcePanel::SceneAreaMeshChooseCallback(const tstring& sArgs)
{
	SetupChooser(m_hAreaMeshText);
}

void CSourcePanel::SceneAreaPhysChooseCallback(const tstring& sArgs)
{
	SetupChooser(m_hAreaPhysText);
}

void CSourcePanel::NewSceneAreaCallback(const tstring& sArgs)
{
	auto& oSceneArea = ToyEditor()->GetToyToModify().m_aAreas.push_back();

	oSceneArea.m_sName = "New Area";

	if (m_hMeshText->GetText().length())
		oSceneArea.m_sFilename = m_hMeshText->GetText();
	else
	{
		// Look for a scene area that has a file already, copy it to this one for convenience.
		for (size_t i = 0; i < ToyEditor()->GetToyToModify().m_aAreas.size(); i++)
		{
			if (ToyEditor()->GetToyToModify().m_aAreas[i].m_sMesh.length())
			{
				oSceneArea.m_sFilename = ToyEditor()->GetToyToModify().m_aAreas[i].m_sFilename;
				break;
			}
		}
	}

	Layout();

	m_hAreas->SetSelectedNode(ToyEditor()->GetToyToModify().m_aAreas.size()-1);
}

void CSourcePanel::DeleteSceneAreaCallback(const tstring& sArgs)
{
	if (m_hAreas->GetSelectedNodeId() >= ToyEditor()->GetToy().m_aAreas.size())
		return;

	size_t iSelected = m_hAreas->GetSelectedNodeId(); // Grab before GetToyToModify() since a Layout() is called which resets the shapes list.
	auto pToy = &ToyEditor()->GetToyToModify();
	pToy->m_aAreas.erase(pToy->m_aAreas.begin()+iSelected);

	Layout();
}

void CSourcePanel::SceneAreaNameChangedCallback(const tstring& sArgs)
{
	CToySource::CSceneArea* pSceneArea = GetCurrentSceneArea();

	if (m_hAreaNameText->GetText().length())
		TAssert(pSceneArea);

	if (!pSceneArea)
		return;

	pSceneArea->m_sName = m_hAreaNameText->GetText();

	m_hAreas->GetNodeFromAllNodes(m_hAreas->GetSelectedNodeId())->SetText(pSceneArea->m_sName);
}

void CSourcePanel::SceneAreaMeshSourceChangedCallback(const tstring& sArgs)
{
	CToySource::CSceneArea* pSceneArea = GetCurrentSceneArea();

	if (m_hAreaSourceFileText->GetText().length())
		TAssert(pSceneArea);

	if (!pSceneArea)
		return;

	pSceneArea->m_sFilename = m_hAreaSourceFileText->GetText();

	SetModelSourcesAutoComplete(m_hAreaSourceFileText);

	ToyEditor()->ReloadModels();
}

void CSourcePanel::SceneAreaModelChangedCallback(const tstring& sArgs)
{
	CToySource::CSceneArea* pSceneArea = GetCurrentSceneArea();

	if (m_hAreaMeshText->GetText().length())
		TAssert(pSceneArea);

	if (!pSceneArea)
		return;

	pSceneArea->m_sMesh = m_hAreaMeshText->GetText();
}

void CSourcePanel::SceneAreaPhysicsChangedCallback(const tstring& sArgs)
{
	CToySource::CSceneArea* pSceneArea = GetCurrentSceneArea();

	if (m_hAreaPhysText->GetText().length())
		TAssert(pSceneArea);

	if (!pSceneArea)
		return;

	pSceneArea->m_sPhys = m_hAreaPhysText->GetText();
}

void CSourcePanel::SaveCallback(const tstring& sArgs)
{
	ToyEditor()->GetToy().Save();
}

void CSourcePanel::BuildCallback(const tstring& sArgs)
{
	ToyEditor()->GetToy().Build();
}

void CSourcePanel::SceneAreaConfirmedCallback(const tstring& sArgs)
{
	auto pTreeNode = m_hSceneChooser->m_hSceneTree->GetNodeFromAllNodes(stoi(sArgs));
	if (pTreeNode)
		m_hChoosingTextField->SetText(pTreeNode->GetText());

	m_hSceneChooser->SetVisible(false);
}

CToyEditor* CToyEditor::s_pToyEditor = nullptr;

CToyEditor::CToyEditor()
{
	s_pToyEditor = this;

	m_pCreateToySourcePanel = new CCreateToySourcePanel();
	m_pCreateToySourcePanel->Layout();
	m_pCreateToySourcePanel->Center();
	m_pCreateToySourcePanel->SetVisible(false);

	m_pSourcePanel = new CSourcePanel();
	m_pSourcePanel->SetVisible(false);
	glgui::CRootPanel::Get()->AddControl(m_pSourcePanel);

	m_iMeshPreview = ~0;
	m_iPhysPreview = ~0;

	m_bRotatingPreview = false;
	m_angPreview = EAngle(-20, 20, 0);
	m_bDollyingPreview = false;

	m_bSaved = false;
}

CToyEditor::~CToyEditor()
{
	delete m_pCreateToySourcePanel;
}

void CToyEditor::Activate()
{
	Layout();

	BaseClass::Activate();
}

void CToyEditor::Deactivate()
{
	BaseClass::Deactivate();

	m_pCreateToySourcePanel->SetVisible(false);
	m_pSourcePanel->SetVisible(false);
}

void CToyEditor::Layout()
{
	m_pCreateToySourcePanel->SetVisible(false);
	m_pSourcePanel->SetVisible(false);

	if (!m_oToySource.m_sFilename.length())
		m_pCreateToySourcePanel->SetVisible(true);
	else
		m_pSourcePanel->SetVisible(true);

	SetupMenu();

	ReloadModels();
}

void CToyEditor::ReloadModels()
{
	bool bGenPreviewDistance = false;

	if (!CModelLibrary::GetModel(m_iMeshPreview))
		m_iMeshPreview = ~0;

	tstring sMesh = FindAbsolutePath(GetDirectory(GetToy().m_sFilename) + "/" + GetToy().m_sMesh);
	if (IsFile(sMesh))
	{
		if (m_iMeshPreview != ~0)
		{
			CModel* pMesh = CModelLibrary::GetModel(m_iMeshPreview);
			if (m_bReloadPreview || sMesh != FindAbsolutePath(pMesh->m_sFilename))
			{
				CModelLibrary::ReleaseModel(m_iMeshPreview);
				CModelLibrary::ClearUnreferenced();

				if (GetToy().m_bUseLocalTransforms)
					CModelLibrary::LoadNextSceneWithLocalTransforms();

				m_iMeshPreview = CModelLibrary::AddModel(sMesh);

				bGenPreviewDistance = true;
				m_bReloadPreview = false;
			}
		}
		else
		{
			if (GetToy().m_bUseLocalTransforms)
				CModelLibrary::LoadNextSceneWithLocalTransforms();

			m_iMeshPreview = CModelLibrary::AddModel(sMesh);

			bGenPreviewDistance = true;
			m_bReloadPreview = false;
		}
	}
	else
	{
		if (m_iMeshPreview != ~0)
		{
			CModelLibrary::ReleaseModel(m_iMeshPreview);
			CModelLibrary::ClearUnreferenced();
			m_iMeshPreview = ~0;
		}
	}

	// Enclose this next code section in a scope so that the it variable here doesn't affect later code.
	{
		// Mark all scenes as not necessary.
		for (auto it = m_aFileScenes.begin(); it != m_aFileScenes.end(); it++)
			it->second.bMark = false;
	}

	for (size_t i = 0; i < GetToy().m_aAreas.size(); i++)
	{
		const auto& oArea = GetToy().m_aAreas[i];
		auto it = m_aFileScenes.find(oArea.m_sFilename);

		if (it == m_aFileScenes.end())
		{
			// Couldn't find it. We make one!
			CSceneAreaModelData oModelData;

			oModelData.pScene = std::shared_ptr<CConversionScene>(new CConversionScene());
			CModelConverter c(oModelData.pScene.get());
			if (!c.ReadModel(GetDirectory(GetToy().m_sFilename) + "/" + oArea.m_sFilename))
				continue;

			for (size_t j = 0; j < oModelData.pScene->GetNumMeshes(); j++)
				oModelData.aiModels.push_back(CModelLibrary::AddModel(oModelData.pScene.get(), j));

			for (size_t j = 0; j < oModelData.pScene->GetNumMaterials(); j++)
			{
				auto pMaterial = oModelData.pScene->GetMaterial(j);

				CTextureHandle hDiffuse = CTextureLibrary::AddTexture(pMaterial->GetDiffuseTexture());

				CMaterialHandle hMaterial = CMaterialLibrary::CreateBlankMaterial(oArea.m_sFilename + " : " + pMaterial->GetName());

				hMaterial->SetShader("model");

				if (hDiffuse.IsValid())
					hMaterial->SetParameter("DiffuseTexture", hDiffuse);

				hMaterial->SetParameter("Diffuse", pMaterial->m_vecDiffuse);

				// Hold a handle to the materials so they don't get removed next time the list is cleared.
				oModelData.ahMaterials.push_back(hMaterial);
			}

			oModelData.bMark = true;

			m_aFileScenes[oArea.m_sFilename] = oModelData;
			bGenPreviewDistance = true;
		}
		else
		{
			it->second.bMark = true;
		}
	}

	auto it = m_aFileScenes.begin();

	// Any scene still marked as not necessary gets del taco'd.
	while (it != m_aFileScenes.end())
	{
		if (!it->second.bMark)
		{
			for (size_t i = 0; i < it->second.aiModels.size(); i++)
				CModelLibrary::ReleaseModel(it->second.aiModels[i]);

			m_aFileScenes.erase(it++);
		}
		else
			++it;
	}

	if (bGenPreviewDistance)
	{
		m_flPreviewDistance = 0;

		if (m_iMeshPreview != ~0)
			m_flPreviewDistance = std::max(CModelLibrary::GetModel(m_iMeshPreview)->m_aabbVisBoundingBox.Size().Length()*2, m_flPreviewDistance);

		for (auto it = m_aFileScenes.begin(); it != m_aFileScenes.end(); it++)
		{
			for (size_t i = 0; i < it->second.aiModels.size(); i++)
			{
				CModel* pModel = CModelLibrary::GetModel(it->second.aiModels[i]);
				if (pModel)
					m_flPreviewDistance = std::max(pModel->m_aabbVisBoundingBox.Size().Length()*2, m_flPreviewDistance);
			}
		}
	}

	m_hMaterialPreview.Reset();
	tstring sMaterial = GetToy().m_sMesh;
	if (IsFile(sMaterial))
	{
		// Don't bother with clearing old ones, they'll get flushed eventually.
		CMaterialHandle hMaterialPreview = CMaterialLibrary::AddMaterial(sMaterial);

		if (hMaterialPreview.IsValid())
		{
			m_hMaterialPreview = hMaterialPreview;

			if (m_hMaterialPreview->m_ahTextures.size())
			{
				CTextureHandle hBaseTexture = m_hMaterialPreview->m_ahTextures[0];

				m_flPreviewDistance = (float)(hBaseTexture->m_iHeight + hBaseTexture->m_iWidth)/hMaterialPreview->m_iTexelsPerMeter;
			}
		}
	}

	tstring sPhys = FindAbsolutePath(GetDirectory(GetToy().m_sFilename) + "/" + GetToy().m_sPhys);
	if (IsFile(sPhys))
	{
		if (m_iPhysPreview != ~0)
		{
			CModel* pPhys = CModelLibrary::GetModel(m_iPhysPreview);
			if (sPhys != FindAbsolutePath(pPhys->m_sFilename))
			{
				CModelLibrary::ReleaseModel(m_iPhysPreview);
				CModelLibrary::ClearUnreferenced();
				m_iPhysPreview = CModelLibrary::AddModel(sPhys);
			}
		}
		else
			m_iPhysPreview = CModelLibrary::AddModel(sPhys);
	}
	else
	{
		if (m_iPhysPreview != ~0)
		{
			CModelLibrary::ReleaseModel(m_iPhysPreview);
			CModelLibrary::ClearUnreferenced();
		}
		m_iPhysPreview = ~0;
	}
}

void CToyEditor::SetupMenu()
{
	GetFileMenu()->ClearSubmenus();

	GetFileMenu()->AddSubmenu("New", this, NewToy);
	GetFileMenu()->AddSubmenu("Open", this, ChooseToy);

	if (GetToy().m_sFilename.length())
	{
		GetFileMenu()->AddSubmenu("Save", this, SaveToy);
		GetFileMenu()->AddSubmenu("Build", this, BuildToy);
	}
}

void CToyEditor::RenderScene()
{
	{
		CRenderingContext c(GameServer()->GetRenderer(), true);

		if (!GameServer()->GetRenderer()->IsDrawingBackground())
			GameServer()->GetRenderer()->DrawBackground(&c);

		c.UseProgram("model");

		c.SetUniform("bDiffuse", false);
		c.SetBlend(BLEND_ALPHA);

		c.SetUniform("vecDiffuse", Vector4D(1, 1, 1, 1));

		c.SetUniform("vecColor", Vector4D(0.7f, 0.2f, 0.2f, 0.7f));
		c.BeginRenderLines();
			c.Vertex(Vector(-10000, 0, 0));
			c.Vertex(Vector(10000, 0, 0));
		c.EndRender();

		c.SetUniform("vecColor", Vector4D(0.2f, 0.7f, 0.2f, 0.7f));
		c.BeginRenderLines();
			c.Vertex(Vector(0, -10000, 0));
			c.Vertex(Vector(0, 10000, 0));
		c.EndRender();

		c.SetUniform("vecColor", Vector4D(0.2f, 0.2f, 0.7f, 0.7f));
		c.BeginRenderLines();
			c.Vertex(Vector(0, 0, -10000));
			c.Vertex(Vector(0, 0, 10000));
		c.EndRender();

		c.SetUniform("vecColor", Vector4D(1.0f, 1.0f, 1.0f, 0.2f));

		int i;

		Vector vecStartX(-10, -10, 0);
		Vector vecEndX(-10, 10, 0);
		Vector vecStartZ(-10, -10, 0);
		Vector vecEndZ(10, -10, 0);

		c.BeginRenderLines();
		for (i = 0; i <= 20; i++)
		{
			if (i != 10)
			{
				c.Vertex(vecStartX);
				c.Vertex(vecEndX);
				c.Vertex(vecStartZ);
				c.Vertex(vecEndZ);
			}

			vecStartX.x += 1;
			vecEndX.x += 1;
			vecStartZ.y += 1;
			vecEndZ.y += 1;
		}
		c.EndRender();

		c.SetUniform("vecColor", Vector4D(1.0f, 1.0f, 1.0f, 1.0f));
	}

	GameServer()->GetRenderer()->SetRenderingTransparent(false);

	if (m_iMeshPreview != ~0)
		TAssert(CModelLibrary::GetModel(m_iMeshPreview));

	if (m_iMeshPreview != ~0 && CModelLibrary::GetModel(m_iMeshPreview))
	{
		CGameRenderingContext c(GameServer()->GetRenderer(), true);

		if (!c.GetActiveFrameBuffer())
			c.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

		c.SetColor(Color(255, 255, 255));

		c.RenderModel(m_iMeshPreview);
	}

	if (m_hMaterialPreview.IsValid())
	{
		CGameRenderingContext c(GameServer()->GetRenderer(), true);

		if (!c.GetActiveFrameBuffer())
			c.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

		c.SetColor(Color(255, 255, 255));

		c.RenderMaterialModel(m_hMaterialPreview);
	}

	GameServer()->GetRenderer()->SetRenderingTransparent(true);

	if (m_iPhysPreview != ~0 && CModelLibrary::GetModel(m_iPhysPreview))
	{
		CGameRenderingContext c(GameServer()->GetRenderer(), true);

		if (!c.GetActiveFrameBuffer())
			c.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

		c.ClearDepth();

		float flAlpha = 0.3f;
		if (m_iMeshPreview == ~0 && m_hMaterialPreview.IsValid() == 0)
			flAlpha = 1.0f;

		c.SetColor(Color(0, 100, 155, (int)(255*flAlpha)));
		c.SetAlpha(flAlpha);
		if (flAlpha < 1)
			c.SetBlend(BLEND_ALPHA);

		c.RenderModel(m_iPhysPreview);
	}

	for (size_t i = 0; i < GetToy().m_aAreas.size(); i++)
	{
		const auto& oSceneArea = GetToy().m_aAreas[i];

		if (!oSceneArea.m_sMesh.length())
			continue;

		const tstring& sFilename = oSceneArea.m_sFilename;
		const auto& it = m_aFileScenes.find(sFilename);

		if (it == m_aFileScenes.end())
			continue;

		const auto& oAreaData = it->second;

		bool bSelected = false;
		auto pSceneArea = m_pSourcePanel->GetCurrentSceneArea();
		if (pSceneArea)
		{
			if (pSceneArea->m_sFilename == sFilename && pSceneArea->m_sMesh == oSceneArea.m_sMesh)
				bSelected = true;
		}

		CConversionSceneNode* pMeshNode = oAreaData.pScene->FindSceneNode(oSceneArea.m_sMesh);

		if (pMeshNode)
		{
			class CDrawStack
			{
			public:
				CConversionSceneNode* pMeshNode;
				Matrix4x4             mTransform;
			};

			tvector<CDrawStack> aDrawStack;
			aDrawStack.push_back();
			aDrawStack.back().mTransform = Matrix4x4(Vector(1, 0, 0), Vector(0, 0, 1), Vector(0, -1, 0));  // Make Z up.
			aDrawStack.back().pMeshNode = pMeshNode;

			CDrawStack oCurrent;
			while (aDrawStack.size())
			{
				oCurrent = aDrawStack.back();
				aDrawStack.pop_back();

				Matrix4x4 mTransformations = oCurrent.mTransform;

				if (!GetToy().m_bUseLocalTransforms)
					mTransformations = oCurrent.mTransform * oCurrent.pMeshNode->m_mTransformations;

				for (size_t j = 0; j < oCurrent.pMeshNode->GetNumChildren(); j++)
				{
					aDrawStack.push_back();
					aDrawStack.back().mTransform = mTransformations;
					aDrawStack.back().pMeshNode = oCurrent.pMeshNode->GetChild(j);
				}

				for (size_t m = 0; m < oCurrent.pMeshNode->GetNumMeshInstances(); m++)
				{
					const auto pMeshInstance = oCurrent.pMeshNode->GetMeshInstance(m);
					size_t iModel = oAreaData.aiModels[pMeshInstance->m_iMesh];
					CModel* pModel = CModelLibrary::GetModel(iModel);

					if (!pModel)
						continue;

					CGameRenderingContext c(GameServer()->GetRenderer(), true);

					c.UseProgram("model");

					if (!c.GetActiveFrameBuffer())
						c.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

					c.LoadTransform(mTransformations);

					for (size_t k = 0; k < pModel->m_aiVertexBuffers.size(); k++)
					{
						auto pMaterial = oAreaData.pScene->GetMaterial(pMeshInstance->GetMappedMaterial(k)->m_iMaterial);
						tstring sMaterialName = pMaterial->GetName();

						CMaterialHandle hMaterial = CMaterialLibrary::FindAsset(sFilename + " : " + sMaterialName);

						if (hMaterial.IsValid())
						{
							c.UseMaterial(hMaterial);
						}
						else
						{
							c.SetColor(Color(150, 150, 150));
							c.SetUniform("vecColor", Vector4D(0.6f, 0.6f, 0.6f, 1));
							c.SetUniform("bDiffuse", false);
							c.SetUniform("vecDiffuse", Vector4D(1, 1, 1, 1));
						}

						if (bSelected)
							c.SetColor(Color(255, 0, 0));

						c.RenderModel(pModel, k);
					}
				}
			}
		}

		if (pMeshNode && CModelLibrary::GetModel(m_iMeshPreview))
		{
			CGameRenderingContext c(GameServer()->GetRenderer(), true);

			if (!c.GetActiveFrameBuffer())
				c.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

			c.SetColor(Color(255, 255, 255));

			c.RenderModel(m_iMeshPreview);
		}

		if (m_hMaterialPreview.IsValid())
		{
			CGameRenderingContext c(GameServer()->GetRenderer(), true);

			if (!c.GetActiveFrameBuffer())
				c.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

			c.SetColor(Color(255, 255, 255));

			c.RenderMaterialModel(m_hMaterialPreview);
		}

		GameServer()->GetRenderer()->SetRenderingTransparent(true);

		if (m_iPhysPreview != ~0 && CModelLibrary::GetModel(m_iPhysPreview))
		{
			CGameRenderingContext c(GameServer()->GetRenderer(), true);

			if (!c.GetActiveFrameBuffer())
				c.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

			c.ClearDepth();

			float flAlpha = 0.3f;
			if (m_iMeshPreview == ~0 && m_hMaterialPreview.IsValid() == 0)
				flAlpha = 1.0f;

			c.SetColor(Color(0, 100, 155, (int)(255*flAlpha)));
			c.SetAlpha(flAlpha);
			if (flAlpha < 1)
				c.SetBlend(BLEND_ALPHA);

			c.RenderModel(m_iPhysPreview);
		}
	}

	for (size_t i = 0; i < GetToy().m_aShapes.size(); i++)
	{
		CGameRenderingContext c(GameServer()->GetRenderer(), true);

		if (!c.GetActiveFrameBuffer())
			c.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

		c.ClearDepth();

		c.UseProgram("model");
		c.SetUniform("bDiffuse", false);
		c.SetUniform("vecDiffuse", Vector4D(1, 1, 1, 1));

		float flAlpha = 0.2f;
		if (m_pSourcePanel->m_hPhysicsShapes->GetSelectedNodeId() == i)
			flAlpha = 0.8f;
		if (m_iMeshPreview == ~0 && m_hMaterialPreview.IsValid() == 0)
			flAlpha = 1.0f;
		if (flAlpha < 1)
			c.SetBlend(BLEND_ALPHA);

		if (m_pSourcePanel->m_hPhysicsShapes->GetSelectedNodeId() == i)
			c.SetUniform("vecColor", Color(255, 50, 100, (char)(255*flAlpha)));
		else
			c.SetUniform("vecColor", Color(0, 100, 200, (char)(255*flAlpha)));

		if (m_pSourcePanel->m_hPhysicsShapes->GetSelectedNodeId() == i)
			c.Transform(Manipulator()->GetTransform());
		else
			c.Transform(GetToy().m_aShapes[i].m_trsTransform.GetMatrix4x4());

		c.RenderWireBox(CToy::s_aabbBoxDimensions);

		// Reset the uniforms so other stuff doesn't get this ugly color.
		c.SetUniform("bDiffuse", true);
		c.SetUniform("vecColor", Color(255, 255, 255, 255));
	}
}

void CToyEditor::NewToy()
{
	m_oToySource = CToySource();
	MarkUnsaved();
}

CToySource& CToyEditor::GetToyToModify()
{
	MarkUnsaved();
	return m_oToySource;
}

void CToyEditor::MarkUnsaved()
{
	m_bSaved = false;

	// Don't call Layout, it will invalidate all sorts of important stuff.
	m_pSourcePanel->LayoutFilename();
}

void CToyEditor::MarkSaved()
{
	m_bSaved = true;
	m_pSourcePanel->Layout();
}

void CToyEditor::NewToyCallback(const tstring& sArgs)
{
	m_pCreateToySourcePanel->SetVisible(true);
}

void CToyEditor::SaveToyCallback(const tstring& sArgs)
{
	m_oToySource.Save();
}

void CToyEditor::ChooseToyCallback(const tstring& sArgs)
{
	glgui::CFileDialog::ShowOpenDialog("../sources", ".txt", this, OpenToy);
}

void CToyEditor::OpenToyCallback(const tstring& sArgs)
{
	tstring sGamePath = GetRelativePath(sArgs, ".");

	m_bReloadPreview = true;

	m_oToySource = CToySource();
	m_oToySource.Open(sGamePath);

	m_pSourcePanel->Layout();
	m_pSourcePanel->UpdateFields();
	Layout();
}

void CToyEditor::BuildToyCallback(const tstring& sArgs)
{
	m_oToySource.Build();
}

bool CToyEditor::KeyPress(int c)
{
	// ; because my dvorak to qwerty key mapper works against me when the game is open, oh well.
	if ((c == 'S' || c == ';') && Application()->IsCtrlDown())
	{
		m_oToySource.Save();

		return true;
	}

	return false;
}

bool CToyEditor::MouseInput(int iButton, tinker_mouse_state_t iState)
{
	if (iButton == TINKER_KEY_MOUSE_LEFT)
	{
		m_bRotatingPreview = (iState == TINKER_MOUSE_PRESSED);
		return true;
	}

	if (iButton == TINKER_KEY_MOUSE_RIGHT)
	{
		m_bDollyingPreview = (iState == TINKER_MOUSE_PRESSED);
		return true;
	}

	return false;
}

void CToyEditor::MouseMotion(int x, int y)
{
	if (m_bRotatingPreview)
	{
		int lx, ly;
		if (GameWindow()->GetLastMouse(lx, ly))
		{
			m_angPreview.y -= (float)(x-lx);
			m_angPreview.p -= (float)(y-ly);

			if (m_angPreview.p > 89)
				m_angPreview.p = 89;
			else if (m_angPreview.p < -89)
				m_angPreview.p = -89;
		}
	}

	if (m_bDollyingPreview)
	{
		int lx, ly;
		if (GameWindow()->GetLastMouse(lx, ly))
			m_flPreviewDistance += (float)(y-ly);

		m_flPreviewDistance = std::max(m_flPreviewDistance, 1.0f);
	}
}

void CToyEditor::MouseWheel(int x, int y)
{
	if (y > 0)
	{
		for (int i = 0; i < y; i++)
			m_flPreviewDistance *= 0.9f;
	}
	else if (y < 0)
	{
		for (int i = 0; i < -y; i++)
			m_flPreviewDistance *= 1.1f;
	}
}

TVector CToyEditor::GetCameraPosition()
{
	CModel* pMesh = CModelLibrary::GetModel(m_iMeshPreview);

	Vector vecPreviewAngle = AngleVector(m_angPreview)*m_flPreviewDistance;
	if (!pMesh)
	{
		CModel* pPhys = CModelLibrary::GetModel(m_iPhysPreview);
		if (!pPhys)
			return Vector(0, 0, 0) - vecPreviewAngle;

		return pPhys->m_aabbVisBoundingBox.Center() - vecPreviewAngle;
	}

	return pMesh->m_aabbVisBoundingBox.Center() - vecPreviewAngle;
}

Vector CToyEditor::GetCameraDirection()
{
	return AngleVector(m_angPreview);
}

void CToyEditor::ManipulatorUpdated(const tstring& sArguments)
{
	// Grab this before GetToyToModify since that does a layout and clobbers the list.
	size_t iSelected = m_pSourcePanel->m_hPhysicsShapes->GetSelectedNodeId();

	tvector<tstring> asTokens;
	strtok(sArguments, asTokens);
	TAssert(asTokens.size() == 2);
	TAssert(stoi(asTokens[1]) == iSelected);
	TAssert(iSelected != ~0);

	if (iSelected >= ToyEditor()->GetToy().m_aShapes.size())
		return;

	ToyEditor()->GetToyToModify().m_aShapes[iSelected].m_trsTransform = Manipulator()->GetTRS();
}

void CToyEditor::DuplicateMove(const tstring& sArguments)
{
	size_t iSelected = m_pSourcePanel->m_hPhysicsShapes->GetSelectedNodeId();

	if (iSelected <= ToyEditor()->GetToy().m_aShapes.size())
		return;

	ToyEditor()->GetToyToModify().m_aShapes.push_back(ToyEditor()->GetToy().m_aShapes[iSelected]);

	Layout();

	m_pSourcePanel->m_hPhysicsShapes->SetSelectedNode(ToyEditor()->GetToy().m_aShapes.size()-1);
}

CConversionScene* CToyEditor::FindLoadedSceneFromFile(const tstring& sFile)
{
	auto it = m_aFileScenes.find(sFile);
	if (it == m_aFileScenes.end())
		return nullptr;

	return it->second.pScene.get();
}

CToySource::CToySource()
{
	Clear();
}

void CToySource::Clear()
{
	m_sFilename = "";
	m_sToyFile = "";
	m_sMesh = "";
	m_sPhys = "";

	m_aShapes.clear();

	m_bUseLocalTransforms = true; // Local by default.
	m_flNeighborDistance = 1;

	m_aAreas.clear();
}

void CToySource::Save() const
{
	if (!m_sFilename.length())
		return;

	CreateDirectory(GetDirectory(m_sFilename));

	std::basic_ofstream<tchar> f(m_sFilename.c_str());

	TAssert(f.is_open());
	if (!f.is_open())
		return;

	tstring sMessage = "// Generated by the Tinker Engine\n// Feel free to modify\n\n";
	f.write(sMessage.data(), sMessage.length());

	tstring sGame = "Game: " + GetRelativePath(".", GetDirectory(m_sFilename)) + "\n";
	f.write(sGame.data(), sGame.length());

	tstring sOutput = "Output: " + m_sToyFile + "\n\n";
	f.write(sOutput.data(), sOutput.length());

	if (m_sMesh.length())
	{
		tstring sGame = "Mesh: " + m_sMesh + "\n";
		f.write(sGame.data(), sGame.length());
	}

	if (m_sPhys.length())
	{
		tstring sGame = "Physics: " + m_sPhys + "\n";
		f.write(sGame.data(), sGame.length());
	}

	if (m_bUseLocalTransforms)
	{
		tstring sData = "UseLocalTransforms\n";
		f.write(sData.data(), sData.length());
	}
	else
	{
		tstring sData = "UseGlobalTransforms\n";
		f.write(sData.data(), sData.length());
	}

	if (m_aShapes.size())
	{
		tstring sShapes = "PhysicsShapes:\n{\n";
		f.write(sShapes.data(), sShapes.length());

		tstring sFormat = "\t// Format is translation (x y z), rotation in euler (p y r), scaling (x y z): x y z p y r x y z\n";
		f.write(sFormat.data(), sFormat.length());

		for (size_t i = 0; i < m_aShapes.size(); i++)
		{
			tstring sDimensions =
				pretty_float(m_aShapes[i].m_trsTransform.m_vecTranslation.x) + " " +
				pretty_float(m_aShapes[i].m_trsTransform.m_vecTranslation.y) + " " + 
				pretty_float(m_aShapes[i].m_trsTransform.m_vecTranslation.z) + " " +
				pretty_float(m_aShapes[i].m_trsTransform.m_angRotation.p) + " " +
				pretty_float(m_aShapes[i].m_trsTransform.m_angRotation.y) + " " +
				pretty_float(m_aShapes[i].m_trsTransform.m_angRotation.r) + " " +
				pretty_float(m_aShapes[i].m_trsTransform.m_vecScaling.x) + " " +
				pretty_float(m_aShapes[i].m_trsTransform.m_vecScaling.y) + " " +
				pretty_float(m_aShapes[i].m_trsTransform.m_vecScaling.z);

			tstring sShape = "\tBox: " + sDimensions + "\n";
			f.write(sShape.data(), sShape.length());
		}

		sShapes = "}\n";
		f.write(sShapes.data(), sShapes.length());
	}

	if (m_aAreas.size())
	{
		tstring sSceneAreas = "SceneAreas:\n{\n";
		f.write(sSceneAreas.data(), sSceneAreas.length());

		tstring sNeighborDistance = tstring("\tNeighborDistance: ") + pretty_float(m_flNeighborDistance) + "\n";
		f.write(sNeighborDistance.data(), sNeighborDistance.length());

		tstring sTransforms = "\tUseGlobalTransforms\n";
		if (m_bUseLocalTransforms)
			sTransforms = "\tUseLocalTransforms\n";
		f.write(sTransforms.data(), sTransforms.length());

		for (size_t i = 0; i < m_aAreas.size(); i++)
		{
			tstring sArea = tstring("\n\tArea: ") + m_aAreas[i].m_sName + "\n\t{\n";
			f.write(sArea.data(), sArea.length());

			tstring sFile = tstring("\t\tFile: ") + m_aAreas[i].m_sFilename + "\n";
			f.write(sFile.data(), sFile.length());

			tstring sMesh = tstring("\t\tMesh: ") + m_aAreas[i].m_sMesh + "\n";
			f.write(sMesh.data(), sMesh.length());

			tstring sPhysics = tstring("\t\tPhysics: ") + m_aAreas[i].m_sPhys + "\n";
			f.write(sPhysics.data(), sPhysics.length());

			tstring sCurly = "\t}\n";
			f.write(sCurly.data(), sCurly.length());
		}

		tstring sCurly = "}\n";
		f.write(sCurly.data(), sCurly.length());
	}

	ToyEditor()->MarkSaved();
}

void CToySource::Build() const
{
	Save();

	CGeppetto g(true, FindAbsolutePath(GetDirectory(m_sFilename)));

	bool bSuccess = g.BuildFromInputScript(FindAbsolutePath(m_sFilename));

	if (bSuccess)
	{
		if (CModelLibrary::FindModel(m_sToyFile) != ~0)
		{
			CModel* pModel = CModelLibrary::GetModel(CModelLibrary::FindModel(m_sToyFile));
			TAssert(pModel);
			if (pModel)
			{
				for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
				{
					CBaseEntity* pEntity = CBaseEntity::GetEntity(i);
					if (!pEntity)
						continue;

					if (pEntity->GetModel() == pModel)
						pEntity->RemoveFromPhysics();
				}

				pModel->Reload();
			}
		}
		else
			CModelLibrary::AddModel(m_sToyFile);
	}
}

void CToySource::Open(const tstring& sFile)
{
	Clear();

	std::basic_ifstream<tchar> f((sFile).c_str());
	if (!f.is_open())
	{
		TError("Could not read input script '" + sFile + "'\n");
		return;
	}

	m_sFilename = sFile;

	std::shared_ptr<CData> pData(new CData());
	CDataSerializer::Read(f, pData.get());

	CData* pOutput = pData->FindChild("Output");
	CData* pSceneAreas = pData->FindChild("SceneAreas");
	CData* pMesh = pData->FindChild("Mesh");
	CData* pPhysics = pData->FindChild("Physics");
	CData* pPhysicsShapes = pData->FindChild("PhysicsShapes");

	if (pOutput)
		m_sToyFile = pOutput->GetValueString();
	else
		m_sToyFile = "";

	if (pMesh)
		m_sMesh = pMesh->GetValueString();
	else
		m_sMesh = "";

	if (pPhysics)
		m_sPhys = pPhysics->GetValueString();
	else
		m_sPhys = "";

	CData* pGlobalTransforms = pData->FindChild("UseGlobalTransforms");
	m_bUseLocalTransforms = !pGlobalTransforms;

	if (pPhysicsShapes)
	{
		for (size_t i = 0; i < pPhysicsShapes->GetNumChildren(); i++)
		{
			CData* pChild = pPhysicsShapes->GetChild(i);

			TAssert(pChild->GetKey() == "Box");
			if (pChild->GetKey() != "Box")
				continue;

			tvector<tstring> asTokens;
			strtok(pChild->GetValueString(), asTokens);
			TAssert(asTokens.size() == 9);
			if (asTokens.size() != 9)
				continue;

			CPhysicsShape& oShape = m_aShapes.push_back();

			oShape.m_trsTransform = pChild->GetValueTRS();
		}
	}

	if (pSceneAreas)
	{
		for (size_t i = 0; i < pSceneAreas->GetNumChildren(); i++)
		{
			CData* pChild = pSceneAreas->GetChild(i);

			if (pChild->GetKey() == "NeighborDistance")
			{
				m_flNeighborDistance = pChild->GetValueFloat();
				continue;
			}

			if (pChild->GetKey() == "UseGlobalTransforms")
			{
				m_bUseLocalTransforms = false;
				continue;
			}

			if (pChild->GetKey() == "UseLocalTransforms")
			{
				m_bUseLocalTransforms = true;
				continue;
			}

			TAssert(pChild->GetKey() == "Area");
			if (pChild->GetKey() != "Area")
				continue;

			auto& oArea = m_aAreas.push_back();

			oArea.m_sName = pChild->GetValueString();

			CData* pFile = pChild->FindChild("File");
			CData* pMesh = pChild->FindChild("Mesh");
			CData* pPhysics = pChild->FindChild("Physics");

			if (pFile)
				oArea.m_sFilename = pFile->GetValueString();
			else
				oArea.m_sFilename = "";

			if (pMesh)
				oArea.m_sMesh = pMesh->GetValueString();
			else
				oArea.m_sMesh = "";

			if (pPhysics)
				oArea.m_sPhys = pPhysics->GetValueString();
			else
				oArea.m_sPhys = "";
		}
	}

	ToyEditor()->MarkSaved();
}
