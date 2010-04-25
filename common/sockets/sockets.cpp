#include "sockets.h"

#include "strutils.h"

CClientSocket::CClientSocket(const char* pszHostname, int iPort)
{
	m_sHostname = pszHostname;
	m_iPort = iPort;

	Initialize();

	if (!Connect(m_sHostname.c_str(), m_iPort))
		Close();
}

CClientSocket::~CClientSocket()
{
	Close();
}

void CClientSocket::Initialize()
{
	m_Socket = -1;
	m_sError = "";

#ifdef _WIN32
    if(WSAStartup(MAKEWORD(2,0), &m_WSAData) !=0)
    {
		m_sError = "WSAStartup failed.";
        return;
    }
#endif

	m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}

bool CClientSocket::Connect(const char* pszHostname, int iPort)
{
	struct sockaddr_in srv;
	struct addrinfo hints;
	struct addrinfo *remoteHost;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM; // TCP

	ostringstream ssPort;
	ssPort << iPort;

	int iResult = getaddrinfo(pszHostname, ssPort.str().c_str(), &hints, &remoteHost);
	if ( iResult != 0 )
	{
		m_sError = "getaddrinfo() failed.";
		return false;
	}

	if (connect(GetSocket(), remoteHost->ai_addr, sizeof(srv)) == SOCKET_ERROR)
	{
#ifdef _WIN32
		int iError = WSAGetLastError();
		WSACleanup();
		m_sError = "connect() failed.";
#endif
		return false;
	}

	return true;
}

int CClientSocket::Send(const char* pszData, size_t iLength)
{
	return Send(pszData, (int)iLength);
}

int CClientSocket::Send(string sData)
{
	return (Send(sData.c_str(), sData.length()));
}

int CClientSocket::Send(const char* pszData, int iLength)
{
	return (send(GetSocket(), pszData, iLength, 0));
}

int CClientSocket::Recv(char* pszData, int iLength)
{
	return recv(GetSocket(), pszData, iLength, 0);
}

string CClientSocket::RecvAll()
{
	ostringstream ssOutput;
	char szBuffer[1028];
	int iResult;

	while ((iResult = Recv(szBuffer, sizeof(szBuffer))) > 0)
	{
		ssOutput << szBuffer;
	}
	return ssOutput.str();
}

void CClientSocket::Close()
{
#ifdef _WIN32
	closesocket(GetSocket());
#else
	shutdown(GetSocket(), SHUT_RDWR);
	close(GetSocket());
#endif

	m_Socket = -1;
}

CHTTPPostSocket::CHTTPPostSocket(const char* pszHostname, int iPort) : CClientSocket(pszHostname, iPort)
{
}

void CHTTPPostSocket::AddPost(const char* pszKey, const char* pszValue)
{
	string sPostContent = m_sPostContent;

	if (sPostContent.length())
		sPostContent += "&";

	sPostContent += pszKey;
	sPostContent += "=";
	sPostContent += pszValue;

	SetPostContent(sPostContent);
}

void CHTTPPostSocket::SendHTTP11(const char* pszPage)
{
	ostringstream ssOutput;

	ssOutput << "POST " << pszPage << " HTTP/1.1" << endl;
	ssOutput << "Host: " << m_sHostname << endl;
	ssOutput << "Content-Length: " << m_sPostContent.length() << endl;
	ssOutput << "Content-Type: application/x-www-form-urlencoded" << endl << endl;
	Send(ssOutput.str());

	Send(m_sPostContent);
}

void CHTTPPostSocket::SetPostContent(string sPostContent)
{
	m_sPostContent = sPostContent;
}

void CHTTPPostSocket::ParseOutput()
{
	string sReturn = RecvAll();
	bool bHTTPOver = false;
	vector<string> vTokens;

	strtok(sReturn, vTokens, "\n");
	for (unsigned int i = 1; i < vTokens.size(); i++)
	{
		if (!vTokens[i].length())
		{
			bHTTPOver = true;
			continue;
		}

		if (!bHTTPOver)
			continue;

		string::size_type iColon = vTokens[i].find(":");
		KeyValue(vTokens[i].substr(0, iColon).c_str(), vTokens[i].substr(iColon+2).c_str());
	}
}

void CHTTPPostSocket::KeyValue(const char* pszKey, const char* pszValue)
{
}
