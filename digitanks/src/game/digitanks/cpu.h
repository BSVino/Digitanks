#ifndef DT_CPU_H
#define DT_CPU_H

#include "structure.h"

class CCPU : public CSupplier
{
	REGISTER_ENTITY_CLASS(CCPU, CSupplier);

public:
								CCPU();

public:
	virtual size_t				InitialDataStrength() { return 3200; };
	virtual size_t				BaseDataFlowPerTurn() { return 50; };

	virtual float				GetBoundingRadius() const { return 8; };

	virtual bool				OnControlModeChange(controlmode_t eOldMode, controlmode_t eNewMode);

	virtual void				SetupMenu(menumode_t eMenuMode);

	bool						IsPreviewBuildValid() const;

	Vector						GetPreviewBuild() const { return m_vecPreviewBuild; };
	virtual void				SetPreviewBuild(Vector vecPreviewBuild);
	virtual void				SetPreviewStructure(structure_t ePreviewStructure) { m_ePreviewStructure = ePreviewStructure; };
	void						ClearPreviewBuild();

	void						BeginConstruction();
	void						CancelConstruction();
	bool						HasConstruction() { return m_hConstructing != NULL; }
	CStructure*					GetConstructing() { return m_hConstructing; }

	virtual void				StartTurn();
	virtual void				PostStartTurn();

	virtual void				OnRender();
	virtual void				PostRender();

	virtual void				UpdateInfo(std::string& sInfo);

protected:
	Vector						m_vecPreviewBuild;

	structure_t					m_ePreviewStructure;
	CEntityHandle<CStructure>	m_hConstructing;
};

#endif