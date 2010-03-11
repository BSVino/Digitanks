#include "scenetree.h"

#include <GL/freeglut.h>

using namespace modelgui;

CSceneTreePanel* CSceneTreePanel::s_pSceneTreePanel = NULL;

CSceneTreePanel::CSceneTreePanel(CConversionScene* pScene)
	: CMovablePanel("Scene Tree")
{
	m_pScene = pScene;
	m_pTree = new CTree(CModelWindow::Get()->GetArrowTexture(), CModelWindow::Get()->GetEditTexture(), CModelWindow::Get()->GetVisibilityTexture());
	AddControl(m_pTree);

	HasCloseButton(false);
	SetClearBackground(true);

	// Infinite height so that scene objects are always clickable.
	SetSize(GetWidth(), 10000);
	SetPos(50, 100);

	m_pMaterialEditor = NULL;
}

CSceneTreePanel::~CSceneTreePanel()
{
	delete m_pTree;

	if (m_pMaterialEditor)
		delete m_pMaterialEditor;
}

void CSceneTreePanel::Layout()
{
	m_pTree->SetPos(5, HEADER_HEIGHT);
	m_pTree->SetSize(GetWidth() - 5, GetHeight() - HEADER_HEIGHT - 20);

	CMovablePanel::Layout();
}

void CSceneTreePanel::UpdateTree()
{
	m_pTree->ClearTree();

	AddAllToTree();

	Layout();
}

void CSceneTreePanel::Paint(int x, int y, int w, int h)
{
	CMovablePanel::Paint(x, y, w, h);
}

void CSceneTreePanel::AddAllToTree()
{
	size_t iMaterialsNode = m_pTree->AddNode(L"Materials");
	CTreeNode* pMaterialsNode = m_pTree->GetNode(iMaterialsNode);
	pMaterialsNode->SetIcon(CModelWindow::Get()->GetMaterialsNodeTexture());

	size_t i;
	for (i = 0; i < m_pScene->GetNumMaterials(); i++)
	{
		size_t iMaterialNode = pMaterialsNode->AddNode<CConversionMaterial>(m_pScene->GetMaterial(i)->GetName(), m_pScene->GetMaterial(i));
		CTreeNode* pMaterialNode = pMaterialsNode->GetNode(iMaterialNode);
		pMaterialNode->AddVisibilityButton();
		dynamic_cast<CTreeNodeObject<CConversionMaterial>*>(pMaterialNode)->AddEditButton(::OpenMaterialEditor);
	}

	// Don't overload the screen.
	if (pMaterialsNode->m_apNodes.size() > 10)
		pMaterialsNode->SetExpanded(false);

	size_t iMeshesNode = m_pTree->AddNode(L"Meshes");
	CTreeNode* pMeshesNode = m_pTree->GetNode(iMeshesNode);
	pMeshesNode->SetIcon(CModelWindow::Get()->GetMeshesNodeTexture());

	for (i = 0; i < m_pScene->GetNumMeshes(); i++)
		pMeshesNode->AddNode<CConversionMesh>(m_pScene->GetMesh(i)->GetName(), m_pScene->GetMesh(i));

	if (pMeshesNode->m_apNodes.size() > 10)
		pMeshesNode->SetExpanded(false);

	size_t iScenesNode = m_pTree->AddNode(L"Scenes");
	CTreeNode* pScenesNode = m_pTree->GetNode(iScenesNode);
	pScenesNode->SetIcon(CModelWindow::Get()->GetScenesNodeTexture());

	for (i = 0; i < m_pScene->GetNumScenes(); i++)
		AddNodeToTree(pScenesNode, m_pScene->GetScene(i));

	if (pScenesNode->m_apNodes.size() > 10)
		pScenesNode->SetExpanded(false);
}

void CSceneTreePanel::AddNodeToTree(modelgui::CTreeNode* pTreeNode, CConversionSceneNode* pSceneNode)
{
	size_t iNode = pTreeNode->AddNode<CConversionSceneNode>(pSceneNode->GetName(), pSceneNode);
	for (size_t i = 0; i < pSceneNode->GetNumChildren(); i++)
		AddNodeToTree(pTreeNode->GetNode(iNode), pSceneNode->GetChild(i));

	for (size_t m = 0; m < pSceneNode->GetNumMeshInstances(); m++)
	{
		size_t iMeshInstanceNode = pTreeNode->GetNode(iNode)->AddNode<CConversionMeshInstance>(pSceneNode->GetMeshInstance(m)->GetMesh()->GetName(), pSceneNode->GetMeshInstance(m));
		CTreeNode* pMeshInstanceNode = pTreeNode->GetNode(iNode)->GetNode(iMeshInstanceNode);
		pMeshInstanceNode->SetIcon(CModelWindow::Get()->GetMeshesNodeTexture());
		pMeshInstanceNode->AddVisibilityButton();

		for (size_t s = 0; s < pSceneNode->GetMeshInstance(m)->GetMesh()->GetNumMaterialStubs(); s++)
		{
			CConversionMaterialMap* pMaterialMap = pSceneNode->GetMeshInstance(m)->GetMappedMaterial(s);

			if (!pMaterialMap)
				continue;

			if (!m_pScene->GetMaterial(pMaterialMap->m_iMaterial))
				continue;

			size_t iMapNode = pMeshInstanceNode->AddNode<CConversionMaterialMap>(m_pScene->GetMaterial(pMaterialMap->m_iMaterial)->GetName(), pMaterialMap);
			CTreeNode* pMeshMapNode = pMeshInstanceNode->GetNode(iMapNode);
			pMeshMapNode->AddVisibilityButton();
		}
	}
}

void OpenMaterialEditor(CConversionMaterial* pMaterial)
{
	CSceneTreePanel::Get()->OpenMaterialEditor(pMaterial);
}

void CSceneTreePanel::OpenMaterialEditor(CConversionMaterial* pMaterial)
{
	if (m_pMaterialEditor)
		delete m_pMaterialEditor;

	m_pMaterialEditor = new CMaterialEditor(pMaterial, this);

	if (!m_pMaterialEditor)
		return;

	m_pMaterialEditor->SetVisible(true);
	m_pMaterialEditor->Layout();
}

void CSceneTreePanel::Open(CConversionScene* pScene)
{
	CSceneTreePanel* pPanel = Get();

	if (!pPanel)
		pPanel = s_pSceneTreePanel = new CSceneTreePanel(pScene);

	if (!pPanel)
		return;

	pPanel->SetVisible(true);
	pPanel->Layout();
}

CSceneTreePanel* CSceneTreePanel::Get()
{
	return s_pSceneTreePanel;
}

CMaterialEditor::CMaterialEditor(CConversionMaterial* pMaterial, CSceneTreePanel* pSceneTree)
	: CMovablePanel("Material Properties")
{
	m_pMaterial = pMaterial;
	m_pSceneTree = pSceneTree;

	m_pScene = CModelWindow::Get()->GetScene();

	for (size_t i = 0; i < m_pScene->GetNumMaterials(); i++)
	{
		if (m_pScene->GetMaterial(i) == m_pMaterial)
		{
			m_iMaterial = i;
			break;
		}
	}

	int x, y;
	m_pSceneTree->GetAbsPos(x, y);

	SetPos(x + m_pSceneTree->GetWidth(), y);
	SetSize(500, 80);

	m_pName->AppendText(L" - ");
	m_pName->AppendText(pMaterial->GetName().c_str());

	m_pDiffuseLabel = new CLabel(0, 0, 1, 1, "Diffuse map: ");
	AddControl(m_pDiffuseLabel);
	m_pDiffuseFile = new CLabel(0, 0, 1, 1, "");
	m_pDiffuseFile->SetAlign(CLabel::TA_LEFTCENTER);
	m_pDiffuseFile->SetWrap(false);
	AddControl(m_pDiffuseFile);
	m_pDiffuseButton = new CButton(0, 0, 70, 20, "Change...");
	m_pDiffuseButton->SetClickedListener(this, ChooseDiffuse);
	AddControl(m_pDiffuseButton);

	Layout();
}

void CMaterialEditor::Layout()
{
	int iHeight = HEADER_HEIGHT+10;

	m_pDiffuseLabel->SetPos(10, iHeight);
	m_pDiffuseLabel->EnsureTextFits();

	int x, y;
	m_pDiffuseLabel->GetPos(x, y);
	int iDiffuseRight = x + m_pDiffuseLabel->GetWidth();

	m_pDiffuseFile->SetPos(iDiffuseRight, iHeight);
	m_pDiffuseFile->SetText(m_pMaterial->GetTexture().c_str());
	m_pDiffuseFile->SetSize(0, 0);
	m_pDiffuseFile->EnsureTextFits();
	if (m_pDiffuseFile->GetWidth() + m_pDiffuseLabel->GetWidth() + 10 > GetWidth())
		m_pDiffuseFile->SetSize(GetWidth() - m_pDiffuseLabel->GetWidth() - 10, m_pDiffuseFile->GetHeight());

	iHeight += y;

	m_pDiffuseButton->SetPos(GetWidth() - m_pDiffuseButton->GetWidth() - 10, iHeight);

	CMovablePanel::Layout();
}

void CMaterialEditor::ChooseDiffuseCallback()
{
	wchar_t* pszOpen = CModelWindow::OpenFileDialog(L"All *.bmp;*.jpg;*.png;*.tga;*.psd;*.gif;*.tif\0*.bmp;*.jpg;*.png;*.tga;*.psd;*.gif;*.tif\0");

	if (!pszOpen)
		return;

	size_t iTexture = CModelWindow::LoadTextureIntoGL(std::wstring(pszOpen));

	if (!iTexture)
		return;

	CMaterial* pMaterial = &(*CModelWindow::Get()->GetMaterials())[m_iMaterial];

	if (pMaterial->m_iBase)
		glDeleteTextures(1, &pMaterial->m_iBase);

	pMaterial->m_iBase = iTexture;

	m_pMaterial->m_sTexture = pszOpen;

	Layout();
}
