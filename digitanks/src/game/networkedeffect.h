#ifndef NETWORKEDEFFECT_H
#define NETWORKEDEFFECT_H

#include <vector.h>
#include <tstring.h>

class CNetworkedEffect
{
public:
	static void AddInstance(const tstring& sName, Vector vecPosition, EAngle angDirection = EAngle());
};

#endif
