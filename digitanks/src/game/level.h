#ifndef LW_LEVEL_H
#define LW_LEVEL_H

#include <eastl/string.h>

class CLevel
{
public:
	void					ReadFromData(const class CData* pData);
	virtual void			OnReadData(const class CData* pData);

	eastl::string			GetName() { return m_sName; }

protected:
	eastl::string			m_sName;
};

#endif
