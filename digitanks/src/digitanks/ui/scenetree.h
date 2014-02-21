#ifndef DT_SCENETREE_H
#define DT_SCENETREE_H

#include <common.h>
#include <glgui/glgui.h>

#include <dt_common.h>
#include <selectable.h>

class CSceneTreeNode : public glgui::CTreeNode
{
	DECLARE_CLASS(CSceneTreeNode, glgui::CTreeNode);

public:
									CSceneTreeNode(glgui::CTreeNode* pParent, glgui::CTree* pTree);

public:
	virtual void					Delete() { delete this; };

	virtual void					Paint(float x, float y, float w, float h, bool bFloating);

	virtual int						GetNodeHeight() { return 30; };
	virtual int						GetNodeSpacing() { return 10; };
};

class CSceneTreeUnit : public CSceneTreeNode
{
	DECLARE_CLASS(CSceneTreeUnit, CSceneTreeNode);

public:
									CSceneTreeUnit(CEntityHandle<CSelectable> hEntity, glgui::CTreeNode* pParent, glgui::CTree* pTree);

public:
	virtual void					Delete() { delete this; };

	virtual void					Paint(float x, float y, float w, float h, bool bFloating);

	virtual CSelectable*			GetEntity() { return m_hEntity; };

	virtual void					Selected();

	virtual bool					IsCursorListener() {return true;};	// Don't show mouse cursor when hovering a unit button.

protected:
	CEntityHandle<CSelectable>		m_hEntity;
};

class CSceneTreeGroup : public CSceneTreeNode
{
	DECLARE_CLASS(CSceneTreeGroup, CSceneTreeNode);

public:
									CSceneTreeGroup(unittype_t eUnit, glgui::CTree* pTree);

public:
	virtual void					Delete() { delete this; };

	virtual void					Paint(float x, float y, float w, float h, bool bFloating);

	virtual unittype_t				GetUnitType() { return m_eUnit; };

	void							GetUnitDimensions(CDigitanksEntity* pEntity, int& x, int& y, int& w, int& h);

protected:
	unittype_t						m_eUnit;
};

class CSceneTree : public glgui::CTree, public glgui::IEventListener
{
	DECLARE_CLASS(CSceneTree, glgui::CTree);

public:
									CSceneTree();

public:
	virtual void					Delete() { delete this; };

	virtual void					Layout();
	virtual void					BuildTree(bool bForce = false);

	virtual CSceneTreeGroup*		GetUnitNode(unittype_t eUnit);

	virtual void					Paint(float x, float y, float w, float h);

	virtual void					OnAddEntityToTeam(class CDigitanksTeam* pTeam, class CBaseEntity* pEntity);
	virtual void					OnRemoveEntityFromTeam(class CDigitanksTeam* pTeam, class CBaseEntity* pEntity);
	virtual void					OnTeamMembersUpdated();

	void							GetUnitDimensions(CDigitanksEntity* pEntity, int& x, int& y, int& w, int& h);

protected:
	CEntityHandle<CDigitanksTeam>	m_hTeam;
};

#endif
