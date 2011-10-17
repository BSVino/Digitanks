#ifndef LW_DATA_H
#define LW_DATA_H

#include <EASTL/vector.h>

#include <vector.h>
#include <tstring.h>

class CData
{
public:
							CData();
							CData(tstring sKey, tstring sValue);
							~CData();

public:
	CData*					AddChild(tstring sKey);
	CData*					AddChild(tstring sKey, tstring sValue);

	CData*					GetParent() const { return m_pParent; }

	size_t					FindChildIndex(tstring sKey);
	CData*					FindChild(tstring sKey);

	tstring					GetKey() const { return m_sKey; }
	tstring					GetValueTString() const { return m_sValue; }
	eastl::string			GetValueString() const { return convertstring<tchar, char>(m_sValue); }
	bool					GetValueBool() const;
	int						GetValueInt() const;
	size_t					GetValueUInt() const;
	float					GetValueFloat() const;
	Vector2D				GetValueVector2D() const;
	EAngle					GetValueEAngle() const;

	void					SetKey(tstring sKey) { m_sKey = sKey; }
	void					SetValue(tstring sValue) { m_sValue = sValue; }
	void					SetValue(bool);
	void					SetValue(int);
	void					SetValue(size_t);
	void					SetValue(float);
	void					SetValue(Vector2D);
	void					SetValue(EAngle);

	size_t					GetNumChildren() const { return m_apChildren.size(); }
	CData*					GetChild(size_t i) const { return m_apChildren[i]; }

protected:
	CData*					m_pParent;

	tstring					m_sKey;
	tstring					m_sValue;
	eastl::vector<CData*>	m_apChildren;
};

#endif
