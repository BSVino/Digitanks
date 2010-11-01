#include "register.h"

#include <sstream>
#include <time.h>

#include <strutils.h>
#include <license.h>
#include <configfile.h>
#include <mtrand.h>
#include <platform.h>

#ifdef _WIN32
#include "winsock2.h"
#include "Ws2tcpip.h"
#else
#define closesocket close
#endif

ConfigFile r( GetAppDataDirectory(L"Digitanks", L"reg.cfg") );
std::string g_sCode;
std::string g_sKey;

bool IsRegistered()
{
	std::string sKey;
	GenerateKey(g_sCode, sKey);
	return sKey == g_sKey;
}

void SaveProductCode()
{
	r.add("code", g_sCode);
	r.add("key", g_sKey);

	// Apparently you can't modify a hidden file so we need to make it normal before changing it.
#ifdef _WIN32
	SetFileAttributes(GetAppDataDirectory(L"Digitanks", L"reg.cfg").c_str(), FILE_ATTRIBUTE_NORMAL);
#endif

	do
	{
		std::ofstream o;
		o.open(GetAppDataDirectory(L"Digitanks", L"reg.cfg"), std::ios_base::out);
		o << r;
	} while (false);

#ifdef _WIN32
	SetFileAttributes(GetAppDataDirectory(L"Digitanks", L"reg.cfg").c_str(), FILE_ATTRIBUTE_HIDDEN);
#endif
}

std::string GenerateCode()
{
	mtsrand((size_t)time(NULL));

	std::string sCode;
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

void ReadProductCode()
{
	if (r.isFileValid())
	{
		if (r.keyExists("code"))
			g_sCode = r.read<std::string>("code");
		else
			g_sCode = GenerateCode();

		if (r.keyExists("key"))
			g_sKey = r.read<std::string>("key");
	}
	else
		g_sCode = GenerateCode();

	// Don't broke what ain't fixed.
	if (IsRegistered())
		return;

	SaveProductCode();
}

std::string GetProductCode()
{
	return g_sCode;
}

void SetLicenseKey(std::string sKey)
{
	g_sKey = trim(sKey);

	if (IsRegistered())
		SaveProductCode();
}

bool QueryRegistrationKey(std::wstring sKey, std::wstring& sError)
{
	int iResult;

#ifdef _WIN32
	// Initialize Winsock
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0)
	{
		sError = L"Whoops! There was some kind of problem.";
	    return false;
	}
#endif

	struct addrinfo* pResult;
	struct addrinfo oHint;
	memset(&oHint, 0, sizeof(oHint));
	oHint.ai_family = AF_UNSPEC;
	oHint.ai_socktype = SOCK_STREAM;
	oHint.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo("digitanks.com", "80", &oHint, &pResult);

	if (iResult != 0)
	{
		sError = L"Couldn't resolve hostname digitanks.com";
#ifdef _WIN32
		WSACleanup();
#endif
		return false;
	}

	int iSocket = 0;
	for (struct addrinfo *pCurrent = pResult; pCurrent != NULL; pCurrent=pCurrent->ai_next)
	{
		iSocket = socket(pCurrent->ai_family, pCurrent->ai_socktype, pCurrent->ai_protocol);
		if (iSocket == INVALID_SOCKET)
		{
			sError = L"socket() failed";
#ifdef _WIN32
			WSACleanup();
#endif
			return false;
		}

		// Connect to server.
		iResult = connect( iSocket, (struct sockaddr *)pCurrent->ai_addr, (int)pCurrent->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			closesocket(iSocket);
			iResult = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(pResult);

	if (iResult == INVALID_SOCKET)
	{
		sError = L"Couldn't connect to the server. Sorry! Try again in a bit.";
#ifdef _WIN32
		WSACleanup();
#endif
		return false;
	}

	std::wstring sContent;
	sContent.append(L"key=");
	sContent.append(sKey);

	std::wstringstream sPost;
	sPost << L"POST /reg/reg.php HTTP/1.1\n";
	sPost << L"Host: digitanks.com\n";
	sPost << L"Content-Length: " << sContent.length() << "\n";
	sPost << L"Content-Type: application/x-www-form-urlencoded\n\n";
	sPost << sContent << "\n\n";

	std::wstring sPostString = sPost.str();

	std::string sSend;
	sSend.assign(sPostString.begin(), sPostString.end());

	iResult = send( iSocket, sSend.c_str(), sSend.length(), 0 );
	if (iResult == SOCKET_ERROR)
	{
		sError = L"Couldn't send to the server. Sorry! Try again in a bit.";
#ifdef _WIN32
		WSACleanup();
#endif
		return false;
	}

	Sleep(100);

	iResult = shutdown(iSocket, SD_SEND);
	if (iResult == SOCKET_ERROR)
	{
		sError = L"Shutdown failed for some reason. Sorry! Try again in a bit.";
		closesocket(iSocket);
#ifdef _WIN32
		WSACleanup();
#endif
		return false;
	}

	std::string sResult;
	do {
		char szBuf[1000];
		iResult = recv(iSocket, szBuf, 1000, 0);
		if (iResult < 1000)
			szBuf[iResult] = '\0';
		sResult.append(szBuf);
	} while( iResult > 0 );

	std::vector<std::string> asTokens;
	strtok(sResult, asTokens);

	int iCode = -1;
	for (size_t i = 0; i < asTokens.size(); i++)
	{
		std::string sToken = asTokens[i];
		if (sToken.substr(0, 4) == "code")
		{
			std::string sCode = asTokens[i+1];
			iCode = atoi(sCode.c_str());
			break;
		}
	}

	bool bReturn = false;
	if (iCode == 0)
	{
		GenerateKey(g_sCode, g_sKey);
		SaveProductCode();

		// Register.
		sError = L"Thank you for registering Digitanks!";
		bReturn = true;
	}
	else if (iCode == 1)
		sError = L"Looks like that key has already been registered. Please contact support <support@lunarworkshop.net> to register your game.";
	else if (iCode == 2)
		sError = L"Sorry, that registration key doesn't look valid. Double check it and try again.";
	else if (iCode == -1)
		sError = L"Looks like there's something wrong with the server at the moment, try again in a bit or contact support <support@lunarworkshop.net> if it continues.";
	else
		sError = L"Looks like there's something wrong, please contact support <support@lunarworkshop.net> if it continues.";

	closesocket(iSocket);

#ifdef _WIN32
	WSACleanup();
#endif

	return bReturn;
}
