#ifndef LW_LEVEL_H
#define LW_LEVEL_H

#include <tstring.h>

class CLevel
{
public:
	virtual					~CLevel() {};

public:
	void					ReadFromData(const class CData* pData);
	virtual void			OnReadData(const class CData* pData);

	tstring					GetName() { return m_sName; }
	tstring					GetFile() { return m_sFile; }

	void					SetFile(tstring sFile) { m_sFile = sFile; }

protected:
	tstring					m_sName;
	tstring					m_sFile;
};

#endif
