#pragma once
#include "pch.h"
#include "framework.h"
#include "CWTool.h"
#include "Packet.h"
#include <list>
#define BUFFER_SIZE 409600
typedef void(*SOCKET_CALLBACK)(void*, int, std::list<CPacket>&, CPacket&);
class CServerSocket{
public:
	static CServerSocket* getInstance() {
		if (m_instace == NULL) {
			m_instace = new CServerSocket();
		}
		return m_instace;
	}
	int Run(SOCKET_CALLBACK callback, void* arg, short port = 9527) {
		bool ret = InitSocket(port);
		if (ret == false)return -1;
		std::list<CPacket> lstPacket;
		m_callback = callback;
		m_arg = arg;
		int count = 0;
		while (true) {
			if (AcceptClient() == false) {
				if (count >= 3)return -2;
				count++;
			}
			int ret = DealCommand();
			if (ret > 0) {
				m_callback(m_arg, ret, lstPacket, m_packet);
				while (lstPacket.size() > 0) {
					Send(lstPacket.front());
					lstPacket.pop_front();
				}
			}
			CloseClient();
		}
		return 0;
	}
protected:
	bool InitSocket(short port) {
		if (m_sock == INVALID_SOCKET) {
			TRACE("creating socket failed,failed num=%d\n", GetLastError());
			return false;
		}
		sockaddr_in server_addr;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//inet_addr("0.0.0.0")
		server_addr.sin_port = htons(port);
		if (bind(m_sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
			TRACE("bind failed,failed num=%d", GetLastError());
			return false;
		}
		TRACE("bind return true\r\n");
		if (listen(m_sock, 5) == SOCKET_ERROR) {
			TRACE("listen failed,failed num=%d", GetLastError());
			return false;
		}
		TRACE("listen return true\r\n");
		return true;
	}
	bool  AcceptClient() {
		TRACE("enter AcceptClient\r\n");
		sockaddr_in client_addr;
		int cliadrLen = sizeof(client_addr);
		m_client = accept(m_sock, (sockaddr*)&client_addr, &cliadrLen);
		TRACE("m_client= %d\r\n", m_client);
		if (m_client == SOCKET_ERROR) {
			TRACE("accept failed,failed num=%d", GetLastError());
			return false;
		}
		/*recv(client_sock, buffer, sizeof(buffer), 0);
		send(client_sock, buffer, sizeof(buffer), 0);*/
		return true;
	}
	int DealCommand() {
		if (m_client == -1)return false;
		char* buffer = new char[BUFFER_SIZE];
		if (buffer == nullptr) {
			TRACE("Run out of memory\r\n ");
			return -2;
		}
		memset(buffer, 0, sizeof(buffer));
		size_t index{ 0 };
		while (true) {
			size_t len = recv(m_client, buffer + index, BUFFER_SIZE - index, 0);
			if (len <= 0) {
				TRACE("recv faliled,Failed num=%d\n", GetLastError());
				delete[] buffer;
				return -1;
			}
			TRACE("recv:%d\r\n", len);
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, len);
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE - len);
				index -= len;
				delete[]buffer;
				return m_packet.GetCmd();
			}
		}
		delete[]buffer;
		TRACE("command failed\r\n");
		return -1;
	}
	bool Send(const char* pData, int nSize) {
		if (m_client == -1)return FALSE;
		//CWTool::Dump((BYTE*)pData, nSize);
		return send(m_client, pData, nSize, 0) > 0;
	}
	bool Send(CPacket& pack) {
		if (m_client == -1)return FALSE;
		//CWTool::Dump((BYTE*)pack.Data(), pack.Size());
		return send(m_client, pack.Data(), pack.Size(), 0) > 0;
	}
	void CloseClient() {
		if (m_client != INVALID_SOCKET) {
			closesocket(m_client);
			m_client = INVALID_SOCKET;
		}
	}
private:
	void*			m_arg;
	SOCKET			m_sock;
	SOCKET			m_client;
	CPacket			m_packet;
	SOCKET_CALLBACK m_callback;
	CServerSocket() :m_sock(INVALID_SOCKET), m_client(INVALID_SOCKET) {
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("无法初始化套接字环境,请检查网络设置"), _T("初始化错误"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		TRACE("InitSocketEnv return true\r\n");
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
		if (m_sock == INVALID_SOCKET)
			TRACE("Socket Invalid_socket\r\n");
		TRACE("m_sock:%d\r\n", m_sock);
		TRACE("Socket return true\r\n");
	}
	CServerSocket(const CServerSocket& s) {
		m_sock = s.m_sock;
		m_client = s.m_client;
	}
	CServerSocket& operator=(const CServerSocket& s) {
		if (this != &s) {
			m_sock = s.m_sock;
			m_client = s.m_client;
		}
		return *this;
	}
	bool InitSockEnv() {
		WSAData data;
		int err = WSAStartup(MAKEWORD(2, 2), &data);
		if (err != 0)return FALSE;
		if (LOBYTE(data.wVersion) != 2 || HIBYTE(data.wVersion) != 2) {
			WSACleanup();
			return FALSE;
		}
		return TRUE;
	}
	~CServerSocket() {
		closesocket(m_sock);
		WSACleanup();
	}
	static CServerSocket* m_instace;
	static void ReleaseInstance() {
		if (m_instace != NULL) {
			CServerSocket* temp = m_instace;
			m_instace = NULL;
			delete temp;
		}
	}
	class CHelper {
	public:
		CHelper() {
			CServerSocket::getInstance();
		}
		~CHelper(){
			CServerSocket::ReleaseInstance();
		}
	};
	static CHelper m_chelper;
};
extern CServerSocket* pserver;
