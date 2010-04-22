#ifndef CF_SOCKETS_H
#define CF_SOCKETS_H
#ifdef _WIN32
#pragma once
#endif

#include <stdio.h>
#include <string>
#include <sstream>

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

using namespace std;

class CClientSocket
{
public:
					CClientSocket(const char* pszHostname, int iPort);
					~CClientSocket();

	virtual void	Initialize();
	virtual bool	Connect(const char* pszHostname, int iPort);

	virtual int		Send(const char* pszData, size_t iLength);
	virtual int		Send(string sData);
	virtual int		Send(const char* pszData, int iLength);
	virtual int		Recv(char* pszData, int iLength);
	virtual string	RecvAll();

	virtual void	Close();

	inline SOCKET	GetSocket() { return m_Socket; };

	inline string	GetError() { return m_sError; };

protected:
	SOCKET			m_Socket;
	string			m_sError;

#ifdef _WIN32
	WSADATA			m_WSAData;
#endif

	string			m_sHostname;
	int				m_iPort;
};

class CHTTPPostSocket : public CClientSocket
{
public:
					CHTTPPostSocket(const char* pszHostname, int iPort);

	virtual void	SendHTTP11(const char* pszPage);

	virtual void	AddPost(const char* pszKey, const char* pszValue);
	virtual void	SetPostContent(string sPostContent);

	virtual void	ParseOutput();
	virtual void	KeyValue(const char* pszKey, const char* pszValue);

protected:
	string			m_sPostContent;
};

#endif
