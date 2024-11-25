#pragma once
#include "ClientSocket.h"
#include "WatchDlg.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include "resource.h"
#include "CWTool.h"
#include <map>
//#define WM_SEND_PACK	(WM_USER+1)//发送数据包
//#define WM_SEND_DATA	(WM_USER+2)//发送数据
#define WM_SHOW_STATUS	(WM_USER+3)//展示状态
#define WM_SHOW_WATCH	(WM_USER+4)//远程监控
#define WM_SEND_MESSAGE (WM_USER+0x1000)//自定义消息处理
class CClientController{
public:
	static CClientController* getInstance();
	int InitController();//开启线程
	int Invoker(CWnd*& pMainWnd);//唤醒主窗口界面
	LRESULT SendMessage(MSG msg);
	inline void UpdateAddress(int nIP, int nPort) {
		CClientSocket::getInstance()->UpDateAddress(nIP, nPort);
	}
	int DealCommand() {
		return CClientSocket::getInstance()->DealCommand();
	}
	void CloseSocket() {
		return CClientSocket::getInstance()->CloseSocket();
	}
	/*bool SendPacket(const CPacket& pack) {
		CClientSocket* pClient = CClientSocket::getInstance();
		if (pClient->InitSocket() == false)return false;
		pClient->Send(pack);
	}*/
	//1 查看磁盘分区 
	// 2查看指定目录下的文件 
	// 3打开文件 
	// 4下载文件
	// 5鼠标操作
	// 6发送屏幕内容
	// 7锁机
	// 8解锁
	// 9删除文件
	// 1981测试连接
	//返回值为命令 如果小于0 为错误
	int SendCommandPacket(int nCmd, bool bAutoClose = true, BYTE* pData = nullptr, size_t nlength = 0, std::list<CPacket>* plstPacks = NULL);
	int GetImage(CImage& image) {
		CClientSocket* pClient = CClientSocket::getInstance();
		return CWTool::ByteToImage(image, pClient->GetPacket().strData);
	}
	int DownFile(CString strPath);
	void StartWatchScreen();
protected:
	void threadDownLoadFile();
	void threadWatchScreen();
	static void threadDownLoadFileEntry(void* arg);
	static void threadWatchScreenEntry(void* arg);
	CClientController();
	~CClientController() {
		WaitForSingleObject(m_hThread, 100);
	}
	void threadFunc();
	static unsigned int _stdcall threadEntry(void* arg);
	static void releaseInstance() {
		if (m_instance != NULL) {
			CClientController* temp = m_instance;
			delete m_instance;
			temp = NULL;
		}
	}
	//LRESULT OnSendPack(UINT msg, WPARAM wParam, LPARAM lParam);
	//LRESULT OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnShowWatch(UINT nMsg, WPARAM wParam, LPARAM lParam);
private:
	typedef struct MsgInfo {
		MSG		msg;
		LRESULT result;
		MsgInfo(MSG m) {
			result = 0;
			msg = m;
		}
		MsgInfo(const MsgInfo& m) {
			result = m.result;
			memcpy(&msg, &m.msg, sizeof(MSG));
		}
		MsgInfo& operator=(const MsgInfo& m) {
			if (this != &m) {
				result = m.result;
				msg = m.msg;
			}
			return *this;
		}
	}MSGINFO;
	typedef LRESULT(CClientController::* MSGFUNC)(UINT nMsg, WPARAM wParam, LPARAM lParam);
	static std::map<UINT, MSGFUNC>		m_mapFunc;
	inline static CClientController*	m_instance = NULL;
	unsigned int						m_hThreadID;
	bool								m_isClosed;
	CWatchDlg							m_watchDlg;
	CRemoteClientDlg					m_remoteDlg;
	CStatusDlg							m_statusDlg;
	HANDLE								m_hThread;
	HANDLE								m_hThreadDownLoad;
	HANDLE								m_hThreadWatch;
	CString								m_strRemote;
	CString								m_strLocal;
	class CHelper {
		CHelper() {
			CClientController::getInstance();
		}
		~CHelper() {
			CClientController::releaseInstance();
		}
	};
	static CHelper m_helper;
};

