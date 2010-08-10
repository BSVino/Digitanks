#ifndef DT_BUFFER_H
#define DT_BUFFER_H

#include "structure.h"

class CBuffer : public CSupplier
{
	REGISTER_ENTITY_CLASS(CBuffer, CSupplier);

public:
	virtual void				Spawn();
	virtual void				Precache();

	virtual size_t				InitialDataStrength() { return 300; };
	virtual size_t				InitialFleetPoints() const { return 4; };
	virtual float				TotalHealth() const { return 25; };

	virtual void				UpdateInfo(std::string& sInfo);

	virtual const char*			GetName() { return "Buffer"; };
	virtual unittype_t			GetUnitType() { return STRUCTURE_BUFFER; };
	virtual size_t				ConstructionCost() const { return 20; };
};

#endif