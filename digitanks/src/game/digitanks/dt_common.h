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
} menumode_t;

#endif
