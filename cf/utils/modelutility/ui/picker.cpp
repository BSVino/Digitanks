#include "picker.h"

#include <assert.h>

#include "modelwindow.h"
#include <modelconverter/convmesh.h>

using namespace modelgui;

CPicker::CPicker(char* pszName, IEventListener* pCallback, IEventListener::Callback pfnCallback)
	: CMovablePanel(pszName)
{
	m_pCallback = pCallback;
	m_pfnCallback = pfnCallback;

	m_bPopulated = false;

	m_pTree = new CTree(CModelWindow::Get()->GetArrowTexture(), CModelWindow::Get()->GetEditTexture(), CModelWindow::Get()->GetVisibilityTexture());
	m_pTree->SetBackgroundColor(g_clrBox);
	m_pTree->SetSelectedListener(this, Selected);
	AddControl(m_pTree);

	Open();
}

void CPicker::Layout()
{
	m_pTree->SetPos(10, 25);
	m_pTree->SetSize(GetWidth()-20, GetHeight()-35);

	CMovablePanel::Layout();
}

void CPicker::Think()
{
	if (!m_bPopulated)
	{
		m_pTree->ClearTree();
		PopulateTree();
		Layout();
	}

	m_bPopulated = true;

	CMovablePanel::Think();
}

void CPicker::SelectedCallback()
{
	CTreeNode* pSelectedNode = m_pTree->GetSelectedNode();
	if (pSelectedNode)
	{
		NodeSelected(pSelectedNode);
		m_pfnCallback(m_pCallback);
	}
}

void CPicker::Open()
{
	SetVisible(true);
	Layout();
}

void CPicker::Close()
{
}

CMeshInstancePicker::CMeshInstancePicker(IEventListener* pCallback, IEventListener::Callback pfnCallback)
	: CPicker("Pick a mesh", pCallback, pfnCallback)
{
	m_pPickedMeshInstance = NULL;
}

void CMeshInstancePicker::PopulateTree()
{
	CConversionScene* pScene = CModelWindow::Get()->GetScene();

	for (size_t i = 0; i < pScene->GetNumScenes(); i++)
		PopulateTreeNode(NULL, pScene->GetScene(i));
}

void CMeshInstancePicker::PopulateTreeNode(modelgui::CTreeNode* pTreeNode, CConversionSceneNode* pSceneNode)
{
	if (!pSceneNode->GetNumChildren() && !pSceneNode->GetNumMeshInstances())
		return;

	size_t iNode;
	modelgui::CTreeNode* pChildNode;
	if (pTreeNode)
	{
		iNode = pTreeNode->AddNode<CConversionSceneNode>(pSceneNode->GetName(), pSceneNode);
		pChildNode = pTreeNode->GetNode(iNode);
	}
	else
	{
		iNode = m_pTree->AddNode<CConversionSceneNode>(pSceneNode->GetName(), pSceneNode);
		pChildNode = m_pTree->GetNode(iNode);
	}

	for (size_t i = 0; i < pSceneNode->GetNumChildren(); i++)
		PopulateTreeNode(pChildNode, pSceneNode->GetChild(i));

	// Child never got any nodes, remove it and get on with our lives.
	if (!pChildNode->GetNumNodes() && !pSceneNode->GetNumMeshInstances())
	{
		if (pTreeNode)
			pTreeNode->RemoveNode(pChildNode);
		else
			m_pTree->RemoveNode(pChildNode);
		return;
	}

	for (size_t m = 0; m < pSceneNode->GetNumMeshInstances(); m++)
	{
		size_t iMeshInstanceNode = pChildNode->AddNode<CConversionMeshInstance>(pSceneNode->GetMeshInstance(m)->GetMesh()->GetName(), pSceneNode->GetMeshInstance(m));
		CTreeNode* pMeshInstanceNode = pChildNode->GetNode(iMeshInstanceNode);
		pMeshInstanceNode->SetIcon(CModelWindow::Get()->GetMeshesNodeTexture());
	}
}

void CMeshInstancePicker::NodeSelected(CTreeNode* pNode)
{
	CTreeNodeObject<CConversionMeshInstance>* pMeshNode = dynamic_cast<CTreeNodeObject<CConversionMeshInstance>*>(pNode);

	if (!pMeshNode)
		return;

	m_pPickedMeshInstance = pMeshNode->GetObject();
}
