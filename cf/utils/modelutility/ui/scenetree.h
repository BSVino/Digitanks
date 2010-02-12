#ifndef SCENETREE_H
#define SCENETREE_H

#include "modelwindow_ui.h"

class CSceneTreePanel : public CMovablePanel
{
public:
									CSceneTreePanel(CConversionScene* pScene);
									~CSceneTreePanel();

public:
	void							Layout();
	void							Paint(int x, int y, int w, int h);

	void							AddAllToTree();
	void							AddNodeToTree(modelgui::CTreeNode* pTreeNode, CConversionSceneNode* pNode);

	static void						Open(CConversionScene* pScene);
	static CSceneTreePanel*			Get();

public:
	CConversionScene*				m_pScene;

	modelgui::CTree*				m_pTree;

	static CSceneTreePanel*			s_pSceneTreePanel;
};

#endif