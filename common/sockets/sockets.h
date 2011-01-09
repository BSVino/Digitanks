#ifndef LW_SOCKETS_H
#define LW_SOCKETS_H
#ifdef _WIN32
#pragma once
#endif

#include <stdio.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/wait.h>
#include <sys/socket.h>
#include <wctype.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#endif // _WIN32

#ifndef _WIN32
#define SOCKET int
#define SOCKADDR struct sockaddr
#define SOCKET_ERROR -1
#endif

class CClientSocket
{
public:
					CClientSocket(const char* pszHostname, int iPort);
					~CClientSocket();

	virtual void	Initialize();
	virtual bool	Connect(const char* pszHostname, int iPort);

	virtual bool	IsOpen() { return m_bOpen; };

	virtual int		Send(const char* pszData, size_t iLength);
	virtual int		Send(eastl::string sData);
	virtual int		Send(const char* pszData, int iLength);
	virtual int		Recv(char* pszData, int iLength);
	virtual eastl::string	RecvAll();

	virtual void	Close();

	inline SOCKET	GetSocket() { return m_iSocket; };

	inline eastl::string	GetError() { return m_sError; };

protected:
	SOCKET			m_iSocket;
	eastl::string	m_sError;

	bool			m_bOpen;

#ifdef _WIN32
	WSADATA			m_WSAData;
#endif

	eastl::string	m_sHostname;
	int				m_iPort;
};

class CPostReply
{
public:
	CPostReply(eastl::string sKey, eastl::string sValue)
	{
		m_sKey = sKey;
		m_sValue = sValue;
	}

public:
	eastl::string	m_sKey;
	eastl::string	m_sValue;
};

class CHTTPPostSocket : public CClientSocket
{
public:
					CHTTPPostSocket(const char* pszHostname, int iPort = 80);

	virtual void	SendHTTP11(const char* pszPage);

	virtual void	AddPost(const char* pszKey, const eastl::string&);
	virtual void	SetPostContent(eastl::string sPostContent);

	virtual void	ParseOutput();
	virtual void	KeyValue(const char* pszKey, const char* pszValue);

	size_t			GetNumReplies() { return m_aKeys.size(); }
	CPostReply*		GetReply(size_t i) { return &m_aKeys[i]; }

protected:
	eastl::string	m_sPostContent;

	eastl::vector<CPostReply>	m_aKeys;
};

#endif
