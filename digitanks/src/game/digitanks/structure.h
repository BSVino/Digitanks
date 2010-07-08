#ifndef DT_STRUCTURE_H
#define DT_STRUCTURE_H

#include "selectable.h"
#include "supplyline.h"

typedef enum
{
	STRUCTURE_BUFFER,
	STRUCTURE_PSU,
	STRUCTURE_INFANTRYLOADER,
	STRUCTURE_TANKLOADER,
} structure_t;

typedef enum
{
	RESOURCE_ELECTRONODE,
	RESOURCE_BITWELL,
} resource_t;

class CStructure : public CSelectable
{
	REGISTER_ENTITY_CLASS(CStructure, CSelectable);

public:
								CStructure();

public:
	virtual float				GetBoundingRadius() const { return 5; };

	void						BeginConstruction(size_t iTurnsToConstruct);
	size_t						GetTurnsToConstruct() { return m_iTurnsToConstruct; };
	bool						IsConstructing() { return m_bConstructing; };

	virtual void				PreStartTurn();

	virtual void				SetSupplier(class CSupplier* pSupplier);
	virtual class CSupplier*	GetSupplier() { return m_hSupplier; };

	virtual void				ModifyContext(class CRenderingContext* pContext);

	virtual void				UpdateInfo(std::string& sInfo) {};

	virtual float				VisibleRange() const { return 50; };

	// AI stuff
	void						AddDefender(class CDigitank* pTank);
	size_t						GetNumLivingDefenders();

protected:
	bool						m_bConstructing;
	size_t						m_iTurnsToConstruct;

	CEntityHandle<CSupplier>		m_hSupplier;
	CEntityHandle<CSupplyLine>		m_hSupplyLine;

	typedef struct
	{
		CEntityHandle<CDigitank>	m_hDefender;
		float						m_flPosition;
	} defender_t;

	// AI stuff
	std::vector<defender_t>		m_aoDefenders;
};

class CSupplier : public CStructure
{
	REGISTER_ENTITY_CLASS(CSupplier, CStructure);

public:
	virtual void				Spawn();

	virtual size_t				InitialDataStrength() { return 100; };
	virtual size_t				BaseDataFlowPerTurn() { return 20; };
	virtual float				GetDataFlowRate();
	float						GetDataFlowRadius() const;
	float						GetDataFlow(Vector vecPoint);
	static float				GetDataFlow(Vector vecPoint, CTeam* pTeam, CSupplier* pIgnore = NULL);
	void						CalculateDataFlow();
	void						GiveDataStrength(size_t iStrength) { m_iDataStrength += iStrength; };

	float						GetChildEfficiency();

	virtual void				PostStartTurn();

	virtual void				PostRender();

	void						UpdateTendrils();

	void						AddChild(CStructure* pChild);
	size_t						GetNumChildren() { return m_ahChildren.size(); };
	CStructure*					GetChild(size_t i) { return m_ahChildren[i]; };

	virtual float				VisibleRange() const;

	static CSupplier*			FindClosestSupplier(CBaseEntity* pUnit);
	static CSupplier*			FindClosestSupplier(Vector vecPoint, class CTeam* pTeam);

protected:
	size_t						m_iDataStrength;
	float						m_flBonusDataFlow;

	class CTendril
	{
	public:
		float					m_flLength;
		Vector					m_vecEndPoint;
	};

	std::vector<CTendril>		m_aTendrils;
	std::vector<CEntityHandle<CStructure> >	m_ahChildren;
};

#endif
