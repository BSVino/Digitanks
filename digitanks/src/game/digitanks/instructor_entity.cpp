#include "instructor_entity.h"

#include <ui/digitankswindow.h>
#include <ui/instructor.h>

REGISTER_ENTITY(CInstructorEntity);

NETVAR_TABLE_BEGIN(CInstructorEntity);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CInstructorEntity);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CInstructorEntity);
	INPUT_DEFINE(DisplayLesson);
INPUTS_TABLE_END();

void CInstructorEntity::Spawn()
{
	BaseClass::Spawn();

	SetName("instructor");
}

void CInstructorEntity::DisplayLesson(const eastl::vector<eastl::string16>& sArgs)
{
	if (sArgs.size() == 0)
	{
		TMsg("DisplayLesson called without a lesson name argument.\n");
		return;
	}

	DigitanksWindow()->GetInstructor()->DisplayTutorial(convertstring<char16_t, char>(sArgs[0]));
}
