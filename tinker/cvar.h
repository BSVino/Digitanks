#ifndef LW_TINKER_CVAR
#define LW_TINKER_CVAR

#include <EASTL/string.h>
#include <EASTL/map.h>
#include <EASTL/vector.h>

#include <common.h>
#include <tstring.h>

typedef void (*CommandCallback)(class CCommand* pCommand, eastl::vector<tstring>& asTokens, const tstring& sCommand);

class CCommand
{
public:
						CCommand(tstring sName, CommandCallback pfnCallback);
						CCommand(eastl::string sName, CommandCallback pfnCallback);

public:
	static void			Run(tstring sCommand);

	tstring		GetName() { return m_sName; };

	virtual void		MakeMePolymorphic() {};	// Can delete if another virtual function is added

	static eastl::vector<tstring> GetCommandsBeginningWith(tstring sFragment);

protected:
	tstring		m_sName;
	CommandCallback		m_pfnCallback;

	static void			RegisterCommand(CCommand* pCommand);

protected:
	static eastl::map<tstring, CCommand*>& GetCommands()
	{
		static eastl::map<tstring, CCommand*> aCommands;
		return aCommands;
	}
};

class CVar : public CCommand
{
	DECLARE_CLASS(CVar, CCommand);

public:
						CVar(tstring sName, tstring sValue);
						CVar(eastl::string sName, eastl::string sValue);

public:
	void				SetValue(tstring sValue);
	void				SetValue(int iValue);
	void				SetValue(float flValue);

	tstring		GetValue() { return m_sValue; };
	bool				GetBool();
	int					GetInt();
	float				GetFloat();

	static CVar*		FindCVar(tstring sName);

	static void			SetCVar(tstring sName, tstring sValue);
	static void			SetCVar(tstring sName, int iValue);
	static void			SetCVar(tstring sName, float flValue);

	static tstring GetCVarValue(tstring sName);
	static bool			GetCVarBool(tstring sName);
	static int			GetCVarInt(tstring sName);
	static float		GetCVarFloat(tstring sName);

	static void			SetCVar(eastl::string sName, eastl::string sValue);
	static void			SetCVar(eastl::string sName, int iValue);
	static void			SetCVar(eastl::string sName, float flValue);

	static eastl::string GetCVarValue(eastl::string sName);
	static bool			GetCVarBool(eastl::string sName);
	static int			GetCVarInt(eastl::string sName);
	static float		GetCVarFloat(eastl::string sName);

protected:
	tstring		m_sValue;
};

#endif
