#include "application.h"

#include <string>
#include <time.h>

#include <strutils.h>
#include <mtrand.h>
#include <platform.h>
#include <sockets/sockets.h>

#ifdef TINKER_NO_REGISTRATION

void CApplication::InitRegistrationFile()
{
}

bool CApplication::IsRegistered()
{
#ifdef TINKER_UNLOCKED
	return true;
#else
	return false;
#endif
}

void CApplication::SaveProductCode()
{
}

eastl::string CApplication::GenerateCode()
{
	return "00000000-0000000000000000";
}

void CApplication::ReadProductCode()
{
}

eastl::string CApplication::GetProductCode()
{
	return "00000000-0000000000000000";
}

void CApplication::SetLicenseKey(eastl::string sKey)
{
}

bool CApplication::QueryRegistrationKey(tstring sServer, tstring sURI, tstring sKey, eastl::string sProduct, tstring& sError)
{
#ifdef TINKER_UNLOCKED
	return true;
#else
	return false;
#endif
}

#else

void CApplication::InitRegistrationFile()
{
	if (m_oRegFile.isFileValid() || m_sCode.length())
		return;

	m_oRegFile = ConfigFile(GetAppDataDirectory(AppDirectory(), _T("reg.cfg")));
	ReadProductCode();
}

bool CApplication::IsRegistered()
{
	InitRegistrationFile();

	eastl::string sKey;

	if (!GenerateKey(m_sCode, sKey))
		return false;

	return sKey == m_sKey;
}

void CApplication::SaveProductCode()
{
	m_oRegFile.add(_T("code"), m_sCode.c_str());
	m_oRegFile.add(_T("key"), m_sKey.c_str());

	// Apparently you can't modify a hidden file so we need to make it normal before changing it.
#ifdef _WIN32
	SetFileAttributes(convertstring<tchar, wchar_t>(GetAppDataDirectory(AppDirectory(), _T("reg.cfg"))).c_str(), FILE_ATTRIBUTE_NORMAL);
#endif

	do
	{
		std::basic_ofstream<tchar> o;
		o.open(convertstring<tchar, char>(GetAppDataDirectory(AppDirectory(), _T("reg.cfg"))).c_str(), std::ios_base::out);
		o << m_oRegFile;
	} while (false);

#ifdef _WIN32
	SetFileAttributes(convertstring<tchar, wchar_t>(GetAppDataDirectory(AppDirectory(), _T("reg.cfg"))).c_str(), FILE_ATTRIBUTE_HIDDEN);
#endif
}

eastl::string CApplication::GenerateCode()
{
	mtsrand((size_t)time(NULL));

	eastl::string sCode;
	char szChar[2];
	szChar[1] = '\0';
	for (size_t i = 0; i < 8; i++)
	{
		szChar[0] = '0' + RandomInt(0, 9);
		sCode.append(szChar);
	}

	sCode.append("-");

	char szNumerals[] =
	{
		'0',
		'1',
		'2',
		'3',
		'4',
		'5',
		'6',
		'7',
		'8',
		'9',
		'A',
		'B',
		'C',
		'D',
		'E',
		'F',
	};

	for (size_t i = 0; i < 16; i++)
	{
		szChar[0] = szNumerals[RandomInt(0, sizeof(szNumerals)-1)];
		sCode.append(szChar);
	}

	return sCode;
}

void CApplication::ReadProductCode()
{
	if (m_oRegFile.isFileValid())
	{
		if (m_oRegFile.keyExists(_T("code")))
			m_sCode = convertstring<tchar, char>(m_oRegFile.read<tstring>(_T("code")));
		else
			m_sCode = GenerateCode();

		if (m_oRegFile.keyExists(_T("key")))
			m_sKey = convertstring<tchar, char>(m_oRegFile.read<tstring>(_T("key")));
	}
	else
		m_sCode = GenerateCode();

	// Don't broke what ain't fixed.
	if (IsRegistered())
		return;

	SaveProductCode();
}

eastl::string CApplication::GetProductCode()
{
	InitRegistrationFile();

	return m_sCode;
}

void CApplication::SetLicenseKey(eastl::string sKey)
{
	m_sKey = trim(sKey);

	if (IsRegistered())
		SaveProductCode();
}

bool CApplication::QueryRegistrationKey(tstring sServer, tstring sURI, tstring sKey, eastl::string sProduct, tstring& sError)
{
	sKey = convertstring<char, tchar>(trim(convertstring<tchar, char>(sKey)));

	CHTTPPostSocket s(convertstring<tchar, char>(sServer.c_str()).c_str());

	if (!s.IsOpen())
	{
		sError = convertstring<char, tchar>(s.GetError());
		return false;
	}

	s.AddPost("key", convertstring<tchar, char>(sKey).c_str());
	s.AddPost("product", sProduct.c_str());

	s.SendHTTP11(convertstring<tchar, char>(sURI).c_str());

//	int iOptVal = 0;
//	setsockopt(iSocket, SOL_SOCKET, SO_SNDBUF, (char*)&iOptVal, sizeof(iOptVal));

	s.ParseOutput();

	int iCode = -1;
	for (size_t i = 0; i < s.GetNumReplies(); i++)
	{
		CPostReply* pReply = s.GetReply(i);
		if (pReply->m_sKey == "code")
		{
			eastl::string sCode = pReply->m_sValue;
			iCode = atoi(sCode.c_str());
			break;
		}
	}

	s.Close();

	bool bReturn = false;
	if (iCode == 0)
	{
		GenerateKey(m_sCode, m_sKey);
		SaveProductCode();

		// Register.
		sError = _T("Thank you for registering!");
		bReturn = true;
	}
	else if (iCode == 1)
		sError = _T("Looks like that key has already been registered. Please contact support <support@lunarworkshop.com> to register your game.");
	else if (iCode == 2)
		sError = _T("Sorry, that registration key doesn't look valid. Double check it and try again.");
	else if (iCode == -1)
		sError = _T("Looks like there's something wrong with the server at the moment, try again in a bit or contact support <support@lunarworkshop.com> if it continues.");
	else
		sError = _T("Looks like there's something wrong, please contact support <support@lunarworkshop.com> if it continues.");

	return bReturn;
}


#endif
