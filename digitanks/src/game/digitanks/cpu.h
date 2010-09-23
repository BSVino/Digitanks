#ifndef DT_CPU_H
#define DT_CPU_H

#include "structure.h"

class CCPU : public CSupplier
{
	REGISTER_ENTITY_CLASS(CCPU, CSupplier);

public:
	virtual void				Spawn();
	virtual void				Precache();

	virtual size_t				InitialDataStrength() { return 3200; };
	virtual size_t				BaseDataFlowPerTurn() { return 50; };
	virtual float				TotalHealth() const { return 100; };
	virtual size_t				InitialFleetPoints() const { return 8; };
	virtual size_t				InitialBandwidth() const { return 4; };
	virtual size_t				InitialPower() const { return 4; };
	virtual bool				IsDataFlowSource() { return true; };

	virtual float				GetBoundingRadius() const { return 8; };

	virtual bool				AllowControlMode(controlmode_t eMode) const;

	virtual void				SetupMenu(menumode_t eMenuMode);

	virtual bool				NeedsOrders();

	bool						IsPreviewBuildValid() const;

	Vector						GetPreviewBuild() const { return m_vecPreviewBuild; };
	virtual void				SetPreviewBuild(Vector vecPreviewBuild);
	virtual void				SetPreviewStructure(unittype_t ePreviewStructure) { m_ePreviewStructure = ePreviewStructure; };
	void						ClearPreviewBuild();

	void						BeginConstruction();
	void						CancelConstruction();
	bool						HasConstruction() { return m_hConstructing != NULL; }
	CStructure*					GetConstructing() { return m_hConstructing; }

	void						BeginProduction();
	void						CancelProduction();
	bool						IsProducing() { return m_bProducing; }

	virtual bool				HasUpdatesAvailable();
	virtual void				InstallUpdate(updatetype_t eUpdate);

	virtual void				StartTurn();

	virtual void				OnRender();
	virtual void				PostRender();

	virtual void				UpdateInfo(std::wstring& sInfo);

	virtual void				OnDeleted();

	virtual const wchar_t*		GetName() { return L"Central Processing Unit"; };
	virtual unittype_t			GetUnitType() { return STRUCTURE_CPU; };

protected:
	Vector						m_vecPreviewBuild;

	unittype_t					m_ePreviewStructure;
	CEntityHandle<CStructure>	m_hConstructing;

	bool						m_bProducing;
	size_t						m_iProduction;

	size_t						m_iFanModel;
	float						m_flFanRotationSpeed;
	float						m_flFanRotation;

	static size_t				s_iCancelIcon;
	static size_t				s_iBuildPSUIcon;
	static size_t				s_iBuildBufferIcon;
	static size_t				s_iBuildLoaderIcon;
	static size_t				s_iBuildInfantryLoaderIcon;
	static size_t				s_iBuildTankLoaderIcon;
	static size_t				s_iBuildArtilleryLoaderIcon;
	static size_t				s_iInstallIcon;
	static size_t				s_iInstallPowerIcon;
	static size_t				s_iInstallBandwidthIcon;
	static size_t				s_iInstallFleetSupplyIcon;
};

#endif