#ifndef LW_LEVEL_H
#define LW_LEVEL_H

#include <eastl/string.h>

class CLevel
{
public:
	virtual					~CLevel() {};

public:
	void					ReadFromData(const class CData* pData);
	virtual void			OnReadData(const class CData* pData);

	eastl::string			GetName() { return m_sName; }
	eastl::string16			GetFile() { return m_sFile; }

	void					SetFile(eastl::string16 sFile) { m_sFile = sFile; }

protected:
	eastl::string			m_sName;
	eastl::string16			m_sFile;
};

#endif
