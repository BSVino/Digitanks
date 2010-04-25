#ifndef DT_BASEENTITY_H
#define DT_BASEENTITY_H

#include <vector.h>

class CBaseEntity
{
public:
	Vector					GetOrigin() const { return m_vecOrigin; };
	void					SetOrigin(const Vector& vecOrigin) { m_vecOrigin = vecOrigin; };

protected:
	Vector					m_vecOrigin;
};

#endif