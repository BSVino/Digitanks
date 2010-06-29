#ifndef DT_STRUCTURE_H
#define DT_STRUCTURE_H

#include "selectable.h"
#include "supplyline.h"

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
	virtual void				StartTurn();
	virtual void				PostStartTurn() {};

	virtual void				SetSupplier(class CSupplier* pSupplier);
	virtual class CSupplier*	GetSupplier() { return m_hSupplier; };

	virtual void				ModifyContext(class CRenderingContext* pContext);

protected:
	bool						m_bConstructing;
	size_t						m_iTurnsToConstruct;

	CEntityHandle<CSupplier>		m_hSupplier;
	CEntityHandle<CSupplyLine>		m_hSupplyLine;
};

class CSupplier : public CStructure
{
	REGISTER_ENTITY_CLASS(CSupplier, CStructure);

public:
	virtual void				Spawn();

	virtual size_t				InitialDataStrength() { return 100; };
	virtual size_t				BaseDataFlowPerTurn() { return 20; };
	virtual float				GetDataFlowRate();
	float						GetDataFlowRadius();
	float						GetDataFlow(Vector vecPoint);
	static float				GetDataFlow(Vector vecPoint, CTeam* pTeam, CSupplier* pIgnore = NULL);
	void						CalculateDataFlow();
	void						GiveDataStrength(size_t iStrength) { m_iDataStrength += iStrength; };

	virtual void				PostStartTurn();

	virtual void				PostRender();

	void						UpdateTendrils();

	void						AddChild(CStructure* pChild);

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
