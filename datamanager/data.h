#ifndef LW_DATA_H
#define LW_DATA_H

#include <eastl/string.h>
#include <eastl/vector.h>

#include <vector.h>

class CData
{
public:
							CData();
							CData(eastl::string sKey, eastl::string sValue);
							~CData();

public:
	CData*					AddChild(eastl::string sKey);
	CData*					AddChild(eastl::string sKey, eastl::string sValue);

	CData*					GetParent() const { return m_pParent; }

	size_t					FindChildIndex(eastl::string sKey);
	CData*					FindChild(eastl::string sKey);

	eastl::string			GetKey() const { return m_sKey; }
	eastl::string			GetValueString() const { return m_sValue; }
	int						GetValueInt() const;
	size_t					GetValueUInt() const;
	float					GetValueFloat() const;
	Vector2D				GetValueVector2D() const;

	size_t					GetNumChildren() const { return m_apChildren.size(); }
	CData*					GetChild(size_t i) const { return m_apChildren[i]; }

protected:
	CData*					m_pParent;

	eastl::string			m_sKey;
	eastl::string			m_sValue;
	eastl::vector<CData*>	m_apChildren;
};

#endif
