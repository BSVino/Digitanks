#include "general.h"

#include "intro_window.h"
#include "general_window.h"

REGISTER_ENTITY(CIntroGeneral);

NETVAR_TABLE_BEGIN(CIntroGeneral);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CIntroGeneral);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CIntroGeneral);
	INPUT_DEFINE(Deploy);
INPUTS_TABLE_END();

void CIntroGeneral::Deploy(const eastl::vector<eastl::string16>& sArgs)
{
	IntroWindow()->GetGeneralWindow()->Deploy();
}
