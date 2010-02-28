#include "scenetree.h"

using namespace modelgui;

CSceneTreePanel* CSceneTreePanel::s_pSceneTreePanel = NULL;

CSceneTreePanel::CSceneTreePanel(CConversionScene* pScene)
	: CMovablePanel("Scene Tree")
{
	m_pScene = pScene;
	m_pTree = new CTree(CModelWindow::Get()->GetArrowTexture(), CModelWindow::Get()->GetVisibilityTexture());
	AddControl(m_pTree);

	SetCloseButtonMinimize(true);
	SetClearBackground(true);

	// Infinite height so that scene objects are always clickable.
	SetSize(GetWidth(), 10000);
	SetPos(50, 100);
}

CSceneTreePanel::~CSceneTreePanel()
{
	delete m_pTree;
}

void CSceneTreePanel::Layout()
{
	m_pTree->SetPos(5, 10);
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
