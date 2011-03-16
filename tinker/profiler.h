#ifndef TINKER_PROFILER_H
#define TINKER_PROFILER_H

#include <EASTL/string.h>
#include <EASTL/vector.h>
#include <EASTL/map.h>

#define TPROF(name) CProfileScope _TProf(name);

class CProfileScope
{
public:
								CProfileScope(const eastl::string& sName);
								~CProfileScope();

public:
	eastl::string				GetName() { return m_sName; };

protected:
	eastl::string				m_sName;
};

class CPerfBlock
{
public:
								CPerfBlock(const eastl::string& sName, CPerfBlock* pParent);

public:
	CPerfBlock*					GetParent() { return m_pParent; };

	CPerfBlock*					GetChild(const eastl::string& sName);
	CPerfBlock*					AddChild(const eastl::string& sName);

	void						BeginFrame();

	void						BlockStarted();
	void						BlockEnded();

	eastl::string				GetName() { return m_sName; };
	float						GetTime() { return m_flTime; };

public:
	CPerfBlock*					m_pParent;

	eastl::string				m_sName;
	float						m_flTime;

	float						m_flTimeBlockStarted;

	eastl::map<eastl::string, CPerfBlock*>	m_apPerfBlocks;
};

class CProfiler
{
public:
	static void					BeginFrame();

	static void					PushScope(CProfileScope* pScope);
	static void					PopScope(CProfileScope* pScope);

	static void					Render();

	static bool					IsProfiling() { return s_bProfiling; };

protected:
	static void					PopAllScopes();

	static void					Render(CPerfBlock* pBlock, int& iLeft, int& iTop);

protected:
	static CPerfBlock*			s_pBottomBlock;
	static CPerfBlock*			s_pTopBlock;

	static bool					s_bProfiling;
};

#endif
