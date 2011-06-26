#ifndef DT_BUFFER_H
#define DT_BUFFER_H

#include "structure.h"

class CBuffer : public CSupplier
{
	REGISTER_ENTITY_CLASS(CBuffer, CSupplier);

public:
	virtual void				Spawn();
	virtual void				Precache();

	virtual bool				AllowControlMode(controlmode_t eMode) const;

	virtual void				SetupMenu(menumode_t eMenuMode);

	virtual void				UpdateInfo(tstring& sInfo);

	static size_t				InitialBufferDataStrength() { return 3500; }

	virtual size_t				InitialTurnsToConstruct() { return 1; };
	virtual size_t				InitialDataStrength() { return InitialBufferDataStrength(); };
	virtual size_t				InitialFleetPoints() const { return 2; };
	virtual float				InitialBandwidth() const { return 1; };
	virtual size_t				InitialEnergyBonus() const { return 2; };
	virtual float				InitialRechargeBonus() const { return 10.0f; };
	virtual float				TotalHealth() const { return 250; };

	virtual tstring				GetEntityName() const { return _T("Macro-Buffer"); };
	virtual unittype_t			GetUnitType() const { return STRUCTURE_BUFFER; };
};

class CMiniBuffer : public CSupplier
{
	REGISTER_ENTITY_CLASS(CMiniBuffer, CSupplier);

public:
	virtual void				Spawn();
	virtual void				Precache();

	virtual bool				AllowControlMode(controlmode_t eMode) const;

	virtual void				SetupMenu(menumode_t eMenuMode);

	virtual void				UpdateInfo(tstring& sInfo);

	virtual bool				CanStructureUpgrade();
	virtual void				UpgradeComplete();

	static size_t				InitialMiniBufferDataStrength() { return 2000; }

	virtual size_t				InitialTurnsToConstruct() { return 0; };
	resource_t					GetResourceType() { return RESOURCE_ELECTRONODE; };
	virtual size_t				InitialDataStrength() { return InitialMiniBufferDataStrength(); };
	virtual size_t				InitialFleetPoints() const { return 1; };
	virtual float				InitialBandwidth() const { return 0.5f; };
	virtual size_t				InitialEnergyBonus() const { return 1; };
	virtual float				InitialRechargeBonus() const { return 5.0f; };
	virtual float				TotalHealth() const { return 150; };

	virtual tstring				GetEntityName() const { return _T("Buffer"); };
	virtual unittype_t			GetUnitType() const { return STRUCTURE_MINIBUFFER; };
	virtual unittype_t			GetUpgradeType() const { return STRUCTURE_BUFFER; };
};

#endif
