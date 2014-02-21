#include "materialeditor.h"

#include <files.h>
#include <tvector.h>

#include <glgui/rootpanel.h>
#include <glgui/menu.h>
#include <glgui/label.h>
#include <glgui/textfield.h>
#include <glgui/filedialog.h>
#include <glgui/button.h>
#include <textures/materiallibrary.h>
#include <renderer/game_renderingcontext.h>
#include <renderer/game_renderer.h>
#include <game/gameserver.h>
#include <tinker/application.h>
#include <tinker/keys.h>
#include <ui/gamewindow.h>
#include <renderer/shaders.h>
#include <datamanager/data.h>

#include "workbench.h"

CCreateMaterialPanel::CCreateMaterialPanel()
	: glgui::CMovablePanel("Create Material Tool")
{
	SetBackgroundColor(Color(0, 0, 0, 255));
	SetHeaderColor(Color(100, 100, 100, 255));
	SetBorder(glgui::CPanel::BT_SOME);

	m_pMaterialFileLabel = new glgui::CLabel("Material File:", "sans-serif", 10);
	m_pMaterialFileLabel->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pMaterialFileLabel);
	m_pMaterialFileText = new glgui::CTextField();
	m_pMaterialFileText->SetContentsChangedListener(this, MaterialChanged);
	AddControl(m_pMaterialFileText);

	m_pShader = new glgui::CMenu("Choose Shader");

	for (size_t i = 0; i < CShaderLibrary::GetNumShaders(); i++)
		m_pShader->AddSubmenu(CShaderLibrary::GetShader(i)->m_sName, this, ChooseShader);

	AddControl(m_pShader);
	m_bShaderChosen = false;

	m_pWarning = new glgui::CLabel("");
	m_pWarning->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pWarning);

	m_pCreate = new glgui::CButton("Create");
	m_pCreate->SetClickedListener(this, Create);
	AddControl(m_pCreate);
}

void CCreateMaterialPanel::Layout()
{
	m_pMaterialFileLabel->Layout_AlignTop();
	m_pMaterialFileLabel->Layout_FullWidth();
	m_pMaterialFileText->Layout_FullWidth();
	m_pMaterialFileText->CenterX();
	m_pMaterialFileText->SetTop(m_pMaterialFileLabel->GetBottom());

	m_pShader->Layout_AlignTop(m_pMaterialFileText);
	m_pShader->SetWidth(100);
	m_pShader->CenterX();
	m_pShader->SetHeight(30);

	m_pWarning->Layout_AlignTop(m_pShader);
	m_pWarning->Layout_FullWidth();
	m_pWarning->SetWrap(true);
	m_pWarning->SetBottom(GetBottom() - 60);

	m_pCreate->Layout_AlignBottom();
	m_pCreate->CenterX();

	BaseClass::Layout();

	FileNameChanged();
}

void CCreateMaterialPanel::MaterialChangedCallback(const tstring& sArgs)
{
	FileNameChanged();

	if (!m_pMaterialFileText->GetText().length())
		return;

	tvector<tstring> asExtensions;
	tvector<tstring> asExtensionsExclude;

	asExtensions.push_back(".mat");

	m_pMaterialFileText->SetAutoCompleteFiles(".", asExtensions, asExtensionsExclude);
}

tstring CCreateMaterialPanel::GetMaterialFileName()
{
	tstring sToyFile = m_pMaterialFileText->GetText();

	if (!sToyFile.endswith(".mat"))
		sToyFile.append(".mat");

	return sToyFile;
}

void CCreateMaterialPanel::FileNameChanged()
{
	m_pWarning->SetText("");

	tstring sToyFile = GetMaterialFileName();
	if (IsFile(sToyFile))
		m_pWarning->SetText("WARNING: This material file already exists. It will be overwritten when the new source file is built.");

	m_pCreate->SetVisible(false);

	if (m_pMaterialFileText->GetText().length() && m_bShaderChosen)
		m_pCreate->SetVisible(true);
}

void CCreateMaterialPanel::ChooseShaderCallback(const tstring& sArgs)
{
	tvector<tstring> asTokens;
	strtok(sArgs, asTokens);

	m_bShaderChosen = true;
	FileNameChanged();
	m_pShader->SetText(asTokens[1]);
	m_pShader->Pop(true, true);
}

void CCreateMaterialPanel::CreateCallback(const tstring& sArgs)
{
	if (!m_pMaterialFileText->GetText().length())
		return;

	CData d;
	d.AddChild("Shader", m_pShader->GetText());
	MaterialEditor()->SetMaterial(CMaterialLibrary::AddMaterial(&d, GetMaterialFileName()));

	SetVisible(false);
}

CMaterialPanel::CMaterialPanel()
{
	m_pName = new glgui::CLabel("", "sans-serif", 20);
	AddControl(m_pName);

	m_pShader = new glgui::CMenu("Choose Shader");

	for (size_t i = 0; i < CShaderLibrary::GetNumShaders(); i++)
		m_pShader->AddSubmenu(CShaderLibrary::GetShader(i)->m_sName, this, ChooseShader);

	AddControl(m_pShader);

	m_pParameterPanel = new glgui::CPanel();
	AddControl(m_pParameterPanel);
	m_pParameterPanel->SetVerticalScrollBarEnabled(true);
	m_pParameterPanel->SetScissoring(true);

	m_pSave = new glgui::CButton("Save");
	m_pSave->SetClickedListener(this, Save);
	AddControl(m_pSave);
}

void CMaterialPanel::Layout()
{
	float flWidth = glgui::CRootPanel::Get()->GetWidth();
	float flHeight = glgui::CRootPanel::Get()->GetHeight();

	float flMenuBarBottom = glgui::CRootPanel::Get()->GetMenuBar()->GetBottom();

	float flCurrLeft = 20;
	float flCurrTop = flMenuBarBottom + 10;

	SetDimensions(flCurrLeft, flCurrTop, 200, flHeight-30-flMenuBarBottom);

	m_pName->Layout_AlignTop();
	m_pName->Layout_FullWidth();

	CMaterialHandle hMaterial = MaterialEditor()->GetMaterialPreview();
	if (hMaterial)
	{
		tstring sFilename = hMaterial->m_sFile;
		tstring sAbsoluteGamePath = FindAbsolutePath(".");
		tstring sAbsoluteFilename = FindAbsolutePath(sFilename);
		if (sAbsoluteFilename.find(sAbsoluteGamePath) == 0)
			sFilename = ToForwardSlashes(sAbsoluteFilename.substr(sAbsoluteGamePath.length()));
		m_pName->SetText(sFilename);
	}
	else
		m_pName->SetText("");

	m_pShader->SetVisible(hMaterial.IsValid());
	m_pShader->SetSize(100, 30);
	m_pShader->Layout_AlignTop(m_pName);
	m_pShader->CenterX();
	if (hMaterial)
		m_pShader->SetText(hMaterial->m_sShader);

	m_pSave->SetVisible(hMaterial.IsValid());
	m_pSave->Layout_AlignBottom();
	m_pSave->CenterX();

	m_pParameterPanel->Layout_AlignTop(m_pShader);
	m_pParameterPanel->Layout_FullWidth(0);
	m_pParameterPanel->SetBottom(m_pSave->GetTop() - GetDefaultMargin());

	TAssert(m_apParameterLabels.size() == m_apParameterOptions.size());
	for (size_t i = 0; i < m_apParameterLabels.size(); i++)
	{
		m_pParameterPanel->RemoveControl(m_apParameterLabels[i]);
		m_pParameterPanel->RemoveControl(m_apParameterOptions[i]);

		delete m_apParameterLabels[i];
		delete m_apParameterOptions[i];
	}
	m_apParameterLabels.clear();
	m_apParameterOptions.clear();
	m_asParameterNames.clear();

	if (hMaterial)
	{
		CShader* pShader = CShaderLibrary::GetShader(hMaterial->m_sShader);
		TAssert(pShader);
		if (pShader)
		{
			glgui::CBaseControl* pLast = nullptr;
			for (auto it = pShader->m_aParameters.begin(); it != pShader->m_aParameters.end(); it++)
			{
				CShader::CParameter* pParameter = &it->second;

				m_asParameterNames.push_back(pParameter->m_sName);

				m_apParameterLabels.push_back(new glgui::CLabel(pParameter->m_sName + ": ", "sans-serif", 10));
				m_apParameterLabels.back()->SetAlign(glgui::CLabel::TA_TOPLEFT);
				m_pParameterPanel->AddControl(m_apParameterLabels.back());
				m_apParameterLabels.back()->Layout_FullWidth();
				m_apParameterLabels.back()->Layout_AlignTop(pLast);

				glgui::CTextField* pTextField = new glgui::CTextField();
				m_apParameterOptions.push_back(pTextField);
				m_pParameterPanel->AddControl(pTextField);
				pTextField->SetTop(m_apParameterLabels.back()->GetTop()+12);
				pTextField->Layout_FullWidth();

				bool bTexture = false;
				for (size_t i = 0; i < pParameter->m_aActions.size(); i++)
				{
					if (pParameter->m_aActions[i].m_bTexture)
					{
						bTexture = true;
						break;
					}
				}

				if (bTexture)
					pTextField->SetContentsChangedListener(this, TextureParameterChanged, sprintf("%d", m_asParameterNames.size()-1));
				else
					pTextField->SetContentsChangedListener(this, ParameterChanged);

				pLast = pTextField;

				if (pShader->m_aParameters[pParameter->m_sName].m_sBlend == "[value]")
				{
					pTextField->SetText(hMaterial->m_sBlend);
				}
				else if (pShader->m_aParameters[pParameter->m_sName].m_aActions.size())
				{
					for (size_t i = 0; i < hMaterial->m_aParameters.size(); i++)
					{
						if (hMaterial->m_aParameters[i].m_sName == pParameter->m_sName)
						{
							pTextField->SetText(hMaterial->m_aParameters[i].m_sValue);
							break;
						}
					}
				}
			}
		}
	}

	BaseClass::Layout();
}

void CMaterialPanel::ChooseShaderCallback(const tstring& sArgs)
{
	tvector<tstring> asTokens;
	strtok(sArgs, asTokens);

	m_pShader->SetText(asTokens[1]);
	m_pShader->Pop(true, true);

	const_cast<CMaterial*>(MaterialEditor()->GetMaterialPreview().GetAsset())->m_sShader = asTokens[1];

	Layout();
}

void CMaterialPanel::TextureParameterChangedCallback(const tstring& sArgs)
{
	size_t i = stoi(sArgs);

	TAssert(i < m_apParameterOptions.size());
	if (i >= m_apParameterOptions.size())
		return;

	tvector<tstring> asExtensions;
	asExtensions.push_back(".png");

	m_apParameterOptions[i]->SetAutoCompleteFiles(FindAbsolutePath("."), asExtensions);

	ParameterChangedCallback(sArgs);

	MaterialEditor()->ResetPreviewDistance();
}

void CMaterialPanel::ParameterChangedCallback(const tstring& sArgs)
{
	CMaterialHandle hMaterial = MaterialEditor()->GetMaterialPreview();
	TAssert(hMaterial);
	if (!hMaterial)
		return;

	CMaterial* pMaterial = const_cast<CMaterial*>(MaterialEditor()->GetMaterialPreview().GetAsset());
	CShader* pShader = CShaderLibrary::GetShader(pMaterial->m_sShader);

	TAssert(pShader);
	if (!pShader)
		return;

	pMaterial->m_aParameters.clear();
	for (size_t i = 0; i < m_asParameterNames.size(); i++)
	{
		if (!m_apParameterOptions[i]->GetText().length())
			continue;

		auto& oShaderPar = pShader->m_aParameters[m_asParameterNames[i]];
		if (oShaderPar.m_sBlend.length())
		{
			pMaterial->m_sBlend = oShaderPar.m_sBlend;
			if (pMaterial->m_sBlend == "[value]")
				pMaterial->m_sBlend = m_apParameterOptions[i]->GetText();
			if (pMaterial->m_sBlend != "alpha" && pMaterial->m_sBlend != "additive")
				pMaterial->m_sBlend = "";
			continue;
		}

		size_t iParameter = pMaterial->FindParameter(m_asParameterNames[i], true);
		CMaterial::CParameter* pParameter = &pMaterial->m_aParameters[iParameter];

		pParameter->m_sName = m_asParameterNames[i];

		pMaterial->FillParameter(iParameter, m_apParameterOptions[i]->GetText(), pShader);
	}
}

void CMaterialPanel::SaveCallback(const tstring& sArgs)
{
	MaterialEditor()->GetMaterialPreview()->Save();
	const_cast<CMaterial*>(MaterialEditor()->GetMaterialPreview().GetAsset())->Reload();
}

REGISTER_WORKBENCH_TOOL(MaterialEditor);

CMaterialEditor* CMaterialEditor::s_pMaterialEditor = nullptr;
	
CMaterialEditor::CMaterialEditor()
{
	s_pMaterialEditor = this;

	m_pCreateMaterialPanel = new CCreateMaterialPanel();
	m_pCreateMaterialPanel->Layout();
	m_pCreateMaterialPanel->Center();
	m_pCreateMaterialPanel->SetVisible(false);

	m_pMaterialPanel = new CMaterialPanel();
	m_pMaterialPanel->SetVisible(false);
	m_pMaterialPanel->SetBackgroundColor(Color(0, 0, 0, 150));
	m_pMaterialPanel->SetBorder(glgui::CPanel::BT_SOME);
	glgui::CRootPanel::Get()->AddControl(m_pMaterialPanel);

	m_bRotatingPreview = false;
	m_flPreviewDistance = 0;
}

CMaterialEditor::~CMaterialEditor()
{
	glgui::CRootPanel::Get()->RemoveControl(m_pMaterialPanel);
	delete m_pMaterialPanel;

	delete m_pCreateMaterialPanel;
}

void CMaterialEditor::Activate()
{
	if (!m_hMaterial)
		m_pCreateMaterialPanel->SetVisible(true);
	m_pMaterialPanel->SetVisible(true);

	GetFileMenu()->AddSubmenu("New", this, NewMaterial);
	GetFileMenu()->AddSubmenu("Open", this, ChooseMaterial);
	GetFileMenu()->AddSubmenu("Save", this, SaveMaterial);

	BaseClass::Activate();
}

void CMaterialEditor::Deactivate()
{
	BaseClass::Deactivate();

	m_pCreateMaterialPanel->SetVisible(false);
	m_pMaterialPanel->SetVisible(false);
}

void CMaterialEditor::RenderScene()
{
	if (!m_hMaterial)
		return;

	CGameRenderingContext c(GameServer()->GetRenderer(), true);

	if (!c.GetActiveFrameBuffer())
		c.UseFrameBuffer(GameServer()->GetRenderer()->GetSceneBuffer());

	c.ClearColor(Color(2, 5, 10));

	c.RenderMaterialModel(m_hMaterial);

	if (!m_hMaterial->m_ahTextures.size())
		return;

	CTextureHandle hBaseTexture = m_hMaterial->m_ahTextures[0];

	if (!hBaseTexture)
		return;

	Vector vecUp = Vector(0, 0, 0.5f) * ((float)hBaseTexture->m_iHeight/(float)m_hMaterial->m_iTexelsPerMeter);
	Vector vecLeft = Vector(0, 0.5f, 0) * ((float)hBaseTexture->m_iWidth/(float)m_hMaterial->m_iTexelsPerMeter);

	float flAlpha = 0.3f;
	if (Vector(GetCameraDirection()).Dot(Vector(1, 0, 0)) > 0)
		flAlpha = 0.1f;

	c.UseProgram("model");
	c.SetUniform("bDiffuse", false);
	c.SetBlend(BLEND_ALPHA);
	c.SetUniform("vecDiffuse", Vector4D(1, 1, 1, 1));
	c.SetUniform("vecColor", Color(155, 40, 20, (char)(255*flAlpha)));
	c.RenderWireBox(AABB(-vecUp-vecLeft, vecUp+vecLeft));
}

void CMaterialEditor::NewMaterialCallback(const tstring& sArgs)
{
	m_pCreateMaterialPanel->SetVisible(true);
}

void CMaterialEditor::SaveMaterialCallback(const tstring& sArgs)
{
	if (m_hMaterial)
		m_hMaterial->Save();
}

void CMaterialEditor::ChooseMaterialCallback(const tstring& sArgs)
{
	m_pCreateMaterialPanel->SetVisible(false);

	glgui::CFileDialog::ShowOpenDialog(".", ".mat", this, OpenMaterial);
}

void CMaterialEditor::OpenMaterialCallback(const tstring& sArgs)
{
	tstring sGamePath = GetRelativePath(sArgs, ".");

	m_hMaterial = CMaterialLibrary::AddMaterial(sGamePath);

	m_pMaterialPanel->Layout();

	ResetPreviewDistance();
}

void CMaterialEditor::ResetPreviewDistance()
{
	if (m_hMaterial->m_ahTextures.size())
	{
		CTextureHandle hBaseTexture = m_hMaterial->m_ahTextures[0];

		if (hBaseTexture)
			m_flPreviewDistance = (float)(hBaseTexture->m_iHeight + hBaseTexture->m_iWidth)/m_hMaterial->m_iTexelsPerMeter;
	}
}

bool CMaterialEditor::KeyPress(int c)
{
	// ; because my dvorak to qwerty key mapper works against me when the game is open, oh well.
	if ((c == 'S' || c == ';') && Application()->IsCtrlDown())
	{
		if (m_hMaterial)
			m_hMaterial->Save();

		return true;
	}

	return false;
}

bool CMaterialEditor::MouseInput(int iButton, tinker_mouse_state_t iState)
{
	if (iButton == TINKER_KEY_MOUSE_LEFT)
	{
		m_bRotatingPreview = (iState == TINKER_MOUSE_PRESSED);
		return true;
	}

	return false;
}

void CMaterialEditor::MouseMotion(int x, int y)
{
	if (m_bRotatingPreview)
	{
		int lx, ly;
		if (GameWindow()->GetLastMouse(lx, ly))
		{
			m_angPreview.y += (float)(x-lx);
			m_angPreview.p -= (float)(y-ly);
		}
	}
}

void CMaterialEditor::MouseWheel(int x, int y)
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

TVector CMaterialEditor::GetCameraPosition()
{
	Vector vecPreviewAngle = AngleVector(m_angPreview)*m_flPreviewDistance;
	return -vecPreviewAngle;
}

Vector CMaterialEditor::GetCameraDirection()
{
	return AngleVector(m_angPreview);
}

void CMaterialEditor::SetMaterial(CMaterialHandle hMaterial)
{
	m_hMaterial = hMaterial;
	m_pMaterialPanel->Layout();
}
