#ifndef LW_TINKER_CVAR
#define LW_TINKER_CVAR

#include <common.h>
#include <EASTL/string.h>
#include <EASTL/map.h>
#include <EASTL/vector.h>

typedef void (*CommandCallback)(class CCommand* pCommand, eastl::vector<eastl::string16>& asTokens);

class CCommand
{
public:
						CCommand(eastl::string16 sName, CommandCallback pfnCallback);
						CCommand(eastl::string sName, CommandCallback pfnCallback);

public:
	static void			Run(eastl::string16 sCommand);

	eastl::string16		GetName() { return m_sName; };

	virtual void		MakeMePolymorphic() {};	// Can delete if another virtual function is added

protected:
	eastl::string16		m_sName;
	CommandCallback		m_pfnCallback;

	static void			RegisterCommand(CCommand* pCommand);

protected:
	static eastl::map<eastl::string16, CCommand*>& GetCommands()
	{
		static eastl::map<eastl::string16, CCommand*> aCommands;
		return aCommands;
	}
};

class CVar : public CCommand
{
	DECLARE_CLASS(CVar, CCommand);

public:
						CVar(eastl::string16 sName, eastl::string16 sValue);
						CVar(eastl::string sName, eastl::string sValue);

public:
	void				SetValue(eastl::string16 sValue);

	eastl::string16		GetValue() { return m_sValue; };
	bool				GetBool();
	int					GetInt();
	float				GetFloat();

protected:
	eastl::string16		m_sValue;
};

#endif
