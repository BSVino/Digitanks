#ifndef NETWORKEDEFFECT_H
#define NETWORKEDEFFECT_H

#include <EASTL/string.h>
#include <vector.h>

class CNetworkedEffect
{
public:
	static void AddInstance(const eastl::string16& sName, Vector vecPosition, EAngle angDirection = EAngle());
};

#endif
