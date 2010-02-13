#include "scenetree.h"

using namespace modelgui;

CSceneTreePanel* CSceneTreePanel::s_pSceneTreePanel = NULL;

CSceneTreePanel::CSceneTreePanel(CConversionScene* pScene)
	: CMovablePanel("Scene Tree")
{
	m_pScene = pScene;
	m_pTree = new CTree(CModelWindow::Get()->GetArrowTexture());
	AddControl(m_pTree);
}

CSceneTreePanel::~CSceneTreePanel()
{
	delete m_pTree;
}

void CSceneTreePanel::Layout()
{
	SetPos(50, GetParent()->GetHeight() - GetHeight() - 100);

	m_pTree->SetPos(5, 10);
	m_pTree->SetSize(GetWidth() - 5, GetHeight() - HEADER_HEIGHT - 20);

	m_pTree->ClearTree();

	AddAllToTree();

	CMovablePanel::Layout();
}

void CSceneTreePanel::Paint(int x, int y, int w, int h)
{
	CMovablePanel::Paint(x, y, w, h);
}

void CSceneTreePanel::AddAllToTree()
{
	size_t iMaterialsNode = m_pTree->AddNode(L"Materials");

	size_t i;
	for (i = 0; i < m_pScene->GetNumMaterials(); i++)
		m_pTree->GetNode(iMaterialsNode)->AddNode<CConversionMaterial>(m_pScene->GetMaterial(i)->GetName(), m_pScene->GetMaterial(i));

	size_t iMeshesNode = m_pTree->AddNode(L"Meshes");

	for (i = 0; i < m_pScene->GetNumMeshes(); i++)
		m_pTree->GetNode(iMeshesNode)->AddNode<CConversionMesh>(m_pScene->GetMesh(i)->GetName(), m_pScene->GetMesh(i));

	size_t iScenesNode = m_pTree->AddNode(L"Scenes");

	for (i = 0; i < m_pScene->GetNumScenes(); i++)
		AddNodeToTree(m_pTree->GetNode(iScenesNode), m_pScene->GetScene(i));
}

void CSceneTreePanel::AddNodeToTree(modelgui::CTreeNode* pTreeNode, CConversionSceneNode* pSceneNode)
{
	size_t iNode = pTreeNode->AddNode<CConversionSceneNode>(L"(Node)", pSceneNode);
	for (size_t i = 0; i < pSceneNode->GetNumChildren(); i++)
		AddNodeToTree(pTreeNode->GetNode(iNode), pSceneNode->GetChild(i));

	for (size_t m = 0; m < pSceneNode->GetNumMeshInstances(); m++)
	{
		size_t iMeshInstanceNode = pTreeNode->GetNode(iNode)->AddNode<CConversionMeshInstance>(pSceneNode->GetMeshInstance(m)->GetMesh()->GetName(), pSceneNode->GetMeshInstance(m));
		for (size_t s = 0; s < pSceneNode->GetMeshInstance(m)->GetMesh()->GetNumMaterialStubs(); s++)
		{
			size_t iMaterial = pSceneNode->GetMeshInstance(m)->GetMappedMaterial(s);
			if (!m_pScene->GetMaterial(iMaterial))
				continue;
			pTreeNode->GetNode(iNode)->GetNode(iMeshInstanceNode)->AddNode(m_pScene->GetMaterial(iMaterial)->GetName());
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
