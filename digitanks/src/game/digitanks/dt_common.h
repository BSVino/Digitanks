#ifndef DT_COMMON
#define DT_COMMON

typedef enum
{
	MODE_NONE = 0,
	MODE_MOVE,
	MODE_TURN,
	MODE_AIM,
	MODE_FIRE,
	MODE_BUILD,
} controlmode_t;

typedef enum
{
	MENUMODE_MAIN,
	MENUMODE_PROMOTE,
	MENUMODE_LOADERS,
	MENUMODE_INSTALL,
} menumode_t;

typedef enum
{
	UPDATECLASS_EMPTY = 0,
	UPDATECLASS_STRUCTURE,
	UPDATECLASS_STRUCTUREUPDATE,
} updateclass_t;

typedef enum
{
	UPDATETYPE_NONE = 0,
	UPDATETYPE_PRODUCTION,
	UPDATETYPE_BANDWIDTH,
	UPDATETYPE_FLEETSUPPLY,
} updatetype_t;

#endif
