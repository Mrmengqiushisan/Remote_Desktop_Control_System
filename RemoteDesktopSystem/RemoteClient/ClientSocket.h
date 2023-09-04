#pragma once
#include"pch.h"
#include"framework.h"
#include<string>
#include "CWTool.h"
#include <vector>
#define  BUFFER_SIZE 409600
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
			sSum += BYTE(strData[j]) & 0xFF;
	}
	CPacket& operator=(const CPacket& pack) {
		if (this == &pack)return *this;
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		sSum = pack.sSum;
		strData = pack.strData;
		return *this;
	}

	CPacket(const BYTE* pData, size_t& nSize) :CPacket() {
		size_t i = 0;
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;
				break;
			}
		}
		//nlength scmd ssum
		if (i + 4 + 2 + 2 > nSize) {
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i);	i += 4;
		if (nLength + i > nSize) {//包没有完全接收到
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i);		i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i);		i += 2;
		WORD sum{ 0 };
		for (size_t j = 0; j < strData.size(); j++)
			sum += BYTE(strData[j]) & 0xFF;
		if (sum == sSum) {
			nSize = i;
			return;
		}
		nSize = 0;
	}
	int Size() {//报数据的大小
		return nLength + 6;
	}
	const char* Data() {
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead;		pData += 2;
		*(DWORD*)(pData) = nLength;	pData += 4;
		*(WORD*)pData = sCmd;		pData += 2;
		memcpy(pData, strData.c_str(), strData.size());	pData += strData.size();
		*(WORD*)pData = sSum;
		return strOut.c_str();
	}
	~CPacket() {}
public:
	WORD		sHead;		//固定为FE FF
	DWORD		nLength;	//包长度(从控制命令开始，到和校验结束)
	WORD		sCmd;		//控制命令
	std::string strData;	//包数据
	WORD		sSum;		//和校验
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
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//点击，移动，双击
	WORD nButton;//左键，右键，中键
	POINT ptXY;	 //坐标
}MOUSEEV, * PMOUSEEV;

class CClientSocket{
public:
	static CClientSocket* getInstance() {//静态函数没有this指针，所以无法访问成员变量
		if (m_instance == NULL) {
			m_instance = new CClientSocket();
		}
		return m_instance;
	}
	bool InitSocket(int nIP, int nPort) {
		if (m_sock != INVALID_SOCKET)CloseSocket();
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
		if (m_sock == INVALID_SOCKET) {
			TRACE("creating socket failed,failed num=%d\n", GetLastError());
			return false;
		}
		sockaddr_in servadr;
		memset(&servadr, 0, sizeof(servadr));
		servadr.sin_family = AF_INET;
		servadr.sin_addr.S_un.S_addr = htonl(nIP);
		servadr.sin_port = htons(nPort);
		if (servadr.sin_addr.S_un.S_addr == INADDR_NONE) {
			AfxMessageBox("指定的IP地址不存在");
			return false;
		}
		int ret = connect(m_sock, (sockaddr*)&servadr, sizeof(sockaddr));
		if (ret == -1) {
			AfxMessageBox("连接失败!");
			TRACE("连接失败，%d.%s\r\n", WSAGetLastError(),CWTool::GetErrorInfo(WSAGetLastError()).c_str());
		}
		TRACE("connect return true\r\n");
		return true;
	}
	int DealCommand() {
		if (m_sock == -1)return -1;
		char* buffer = m_buffer.data();
		static size_t index{ 0 };
		while (true) {
			size_t len = recv(m_sock, buffer + index, (int)(BUFFER_SIZE - index), 0);
			if (len <= 0 && index == 0) {
				TRACE("recv faliled,Failed num=%d\n", GetLastError());
				return -1;
			}
			index += len;
			len = index;
			m_packet = CPacket((BYTE*)buffer, (size_t)len);
			if (len > 0) {
				memmove(buffer, buffer + len, index - len);
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}
	bool Send(const char* pData, int nSize) {
		if (m_sock == -1)return FALSE;
		return send(m_sock, pData, nSize, 0) > 0;
	}
	bool Send(CPacket& pack) {
		TRACE("m_sock= %d\r\n", m_sock);
		if (m_sock == -1)return FALSE;
		return send(m_sock, pack.Data(), pack.Size(), 0) > 0;
	}
	bool GetFilePath(std::string& strPath) {
		if (m_packet.sCmd >= 2 && m_packet.sCmd <= 4) {
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}
	//鼠标移动，左键，右键
	bool GetMouseEvent(MOUSEEV& mouse) {
		if (m_packet.sCmd == 5) {
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;
		}
		return false;
	}
	CPacket& GetPacket() {
		return m_packet;
	}
	void CloseSocket() {
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}
private:
	std::vector<char> m_buffer;
	SOCKET m_sock;
	CPacket m_packet;
	inline static CClientSocket* m_instance = NULL;
	CClientSocket& operator=(const CClientSocket&) {}//赋值运算符
	CClientSocket(const CClientSocket& ss) {//拷贝构造函数
		m_sock = ss.m_sock;
	}
	CClientSocket() :m_sock(INVALID_SOCKET) {
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("无法初始化套接字环境,请检查网络设置"), _T("初始化错误"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		TRACE("InitSockEnv Client return true\r\n");
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
		m_buffer.resize(BUFFER_SIZE);
		memset((void*)m_buffer.data(), 0, BUFFER_SIZE);//不能放在dealcommand中做这个事情 这意味下一次文件的接受会出问题，因为会把数据清理掉
		if (m_sock == INVALID_SOCKET)
			TRACE("socket invalid\r\n");
		TRACE("m_sock:%d\r\n", m_sock);
	}
	~CClientSocket() {
		closesocket(m_sock);
		WSACleanup();
	}
	bool InitSockEnv() {
		WSAData data;
		int err;
		err = WSAStartup(MAKEWORD(1, 1), &data);//TODO:返回值处理
		if (err != 0)return FALSE;
		return TRUE;
	}
	static void releaseInstance() {
		if (m_instance != NULL) {
			CClientSocket* temp = m_instance;
			m_instance = NULL;
			delete temp;
		}
	}
	class CHelper {
	public:
		CHelper() {
			CClientSocket::getInstance();
		}
		~CHelper() {
			CClientSocket::releaseInstance();
		}
	};
	static CHelper m_helper;

};

