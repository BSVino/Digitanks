#ifndef DT_BUFFER_H
#define DT_BUFFER_H

#include "structure.h"

class CBuffer : public CSupplier
{
	REGISTER_ENTITY_CLASS(CBuffer, CSupplier);

public:
	virtual void				Spawn();
	virtual void				Precache();

	virtual void				SetupMenu(menumode_t eMenuMode);

	virtual void				UpdateInfo(eastl::string16& sInfo);

	virtual size_t				GetTurnsToConstruct();
	virtual size_t				InitialDataStrength() { return 1250; };
	virtual size_t				InitialFleetPoints() const { return 2; };
	virtual size_t				InitialBandwidth() const { return 0; };
	virtual size_t				InitialEnergyBonus() const { return 2; };
	virtual float				InitialRechargeBonus() const { return 1.0f; };
	virtual float				TotalHealth() const { return 25; };

	virtual eastl::string16		GetName() { return L"Buffer"; };
	virtual unittype_t			GetUnitType() const { return STRUCTURE_BUFFER; };

protected:
	static size_t				s_iCancelIcon;
	static size_t				s_iInstallIcon;
	static size_t				s_iInstallBandwidthIcon;
	static size_t				s_iInstallFleetSupplyIcon;
	static size_t				s_iInstallEnergyBonusIcon;
	static size_t				s_iInstallRechargeBonusIcon;
};

class CMiniBuffer : public CSupplier
{
	REGISTER_ENTITY_CLASS(CMiniBuffer, CSupplier);

public:
	virtual void				Spawn();
	virtual void				Precache();

	virtual void				SetupMenu(menumode_t eMenuMode);

	virtual void				UpdateInfo(eastl::string16& sInfo);

	virtual bool				CanStructureUpgrade();
	virtual void				UpgradeComplete();

	virtual size_t				GetTurnsToConstruct() { return 1; };
	resource_t					GetResourceType() { return RESOURCE_ELECTRONODE; };
	virtual size_t				InitialDataStrength() { return 450; };
	virtual size_t				InitialFleetPoints() const { return 1; };
	virtual size_t				InitialBandwidth() const { return 0; };
	virtual size_t				InitialEnergyBonus() const { return 1; };
	virtual float				InitialRechargeBonus() const { return 0.5f; };
	virtual float				TotalHealth() const { return 15; };

	virtual eastl::string16		GetName() { return L"Mini-Buffer"; };
	virtual unittype_t			GetUnitType() const { return STRUCTURE_MINIBUFFER; };
	virtual unittype_t			GetUpgradeType() const { return STRUCTURE_BUFFER; };

protected:
	static size_t				s_iUpgradeIcon;
	static size_t				s_iCancelIcon;
};

#endif