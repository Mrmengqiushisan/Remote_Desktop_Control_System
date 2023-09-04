#pragma once
#include "pch.h"
#include "framework.h"
#include "CWTool.h"
#define BUFFER_SIZE 409600

class CPacket {
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}
	CPacket(const CPacket& pack) {
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		sSum = pack.sSum;
		strData = pack.strData;
	}
	CPacket& operator=(const CPacket& pack) {
		if (this != &pack) {
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			sSum = pack.sSum;
			strData = pack.strData;
		}
		return *this;
	}
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {
		sHead = 0xFEFF;
		nLength = (DWORD)(nSize + 4);
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else strData.clear();
		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++)
			sSum += BYTE(pData[j]) & 0xFF;
	}
	//解包
	CPacket(const BYTE* pData, size_t& nSize) :CPacket() {
		size_t i = 0;
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i = i + 2;
				break;
			}
		}
		//检查包长度
		if (i + 4 + 2 + 2 > nSize) {
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize) {
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 4);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i); i += 2;
		WORD sum{ 0 };
		for (size_t j = 0; j < strData.size(); j++) {
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {
			nSize = i;
			return;
		}
		nSize = 0;
	}
	WORD GetCmd() { return sCmd; }
	std::string GetStrData() { return strData; }
	int Size() {
		return nLength + 6;
	}
	const char* Data() {
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead;							pData += 2;
		*(DWORD*)(pData) = nLength;						pData += 4;
		*(WORD*)pData = sCmd;							pData += 2;
		memcpy(pData, strData.c_str(), strData.size());	pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}
	~CPacket() {}
private:
	WORD		sHead;		//固定长度为 FE FF
	DWORD		nLength;	//包长度
	WORD		sCmd;		//控制命令
	WORD		sSum;		//和校验
	std::string	strData;	//包数据
	std::string strOut;		//整个包的数据
};

typedef struct file_info {
	BOOL isInvalid;         //是否有效
	BOOL isDirectory;       //是否为目录0否1是
	BOOL hasNext;           //是否还有后续文件0没有1有
	char szFileName[MAX_PATH]{}; //文件名
	file_info() {
		isInvalid = FALSE;
		isDirectory = -1;
		hasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
}FILEINFO, * PFILEINFO;

typedef struct MouseEvent {
	WORD nAction;//点击，移动，双击
	WORD nButton;//左键，右键，中键
	POINT ptXY;	 //坐标
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
}MOUSEEV, * PMOUSEEV;

class CServerSocket{
public:
	static CServerSocket* getInstance() {
		if (m_instace == NULL) {
			m_instace = new CServerSocket();
		}
		return m_instace;
	}
	bool InitSocket() {
		if (m_sock == INVALID_SOCKET) {
			TRACE("creating socket failed,failed num=%d\n", GetLastError());
			return false;
		}
		sockaddr_in server_addr;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.S_un.S_addr = inet_addr("0.0.0.0");
		server_addr.sin_port = htons(9527);
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
	bool GetFilePath(std::string& strPath) {
		if (m_packet.GetCmd() >= 2 && m_packet.GetCmd() <= 4 || m_packet.GetCmd() == 9) {
			strPath = m_packet.GetStrData();
			return true;
		}
		return false;
	}
	bool GetMouseEvent(MOUSEEV& mouse) {
		if (m_packet.GetCmd() == 5) {
			memcpy(&mouse, m_packet.GetStrData().c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}
	CPacket& GetPacket() {
		return m_packet;
	}
	void CloseClient() {
		closesocket(m_client);
		m_client = INVALID_SOCKET;
	}
private:
	SOCKET	m_sock;
	SOCKET	m_client;
	CPacket m_packet;
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
