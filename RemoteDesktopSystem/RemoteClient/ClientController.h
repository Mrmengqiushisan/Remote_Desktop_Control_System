#pragma once
#include "ClientSocket.h"
#include "WatchDlg.h"
#include "RemoteClientDlg.h"
#include "StatusDlg.h"
#include "resource.h"
#include "CWTool.h"
#include <map>
//#define WM_SEND_PACK	(WM_USER+1)//�������ݰ�
//#define WM_SEND_DATA	(WM_USER+2)//��������
#define WM_SHOW_STATUS	(WM_USER+3)//չʾ״̬
#define WM_SHOW_WATCH	(WM_USER+4)//Զ�̼��
#define WM_SEND_MESSAGE (WM_USER+0x1000)//�Զ�����Ϣ����
class CClientController{
public:
	static CClientController* getInstance();
	int InitController();//�����߳�
	int Invoker(CWnd*& pMainWnd);//���������ڽ���
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
	//1 �鿴���̷��� 
	// 2�鿴ָ��Ŀ¼�µ��ļ� 
	// 3���ļ� 
	// 4�����ļ�
	// 5������
	// 6������Ļ����
	// 7����
	// 8����
	// 9ɾ���ļ�
	// 1981��������
	//����ֵΪ���� ���С��0 Ϊ����
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

