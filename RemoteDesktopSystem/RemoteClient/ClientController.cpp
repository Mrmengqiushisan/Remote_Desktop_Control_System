#include "pch.h"
#include "ClientController.h"

std::map<UINT, CClientController::MSGFUNC>CClientController::m_mapFunc;

CClientController* CClientController::getInstance(){
    if (m_instance == NULL) {
        m_instance = new CClientController();
        struct {
            UINT nMsg;
            MSGFUNC func;
        }MsgFuncs[]{
            //{WM_SEND_PACK,&CClientController::OnSendPack},
            //{WM_SEND_DATA,&CClientController::OnSendData},
            {WM_SHOW_STATUS,&CClientController::OnShowStatus},
            {WM_SHOW_WATCH,&CClientController::OnShowWatch},
            {(UINT)-1,NULL}
        };
        for (int i = 0; MsgFuncs[i].func != NULL; i++)
            m_mapFunc[MsgFuncs[i].nMsg] = MsgFuncs[i].func;
    }
    return m_instance;
}

int CClientController::InitController(){
    m_hThread = (HANDLE)_beginthreadex(NULL, 0, &CClientController::threadEntry, this, 0, &m_hThreadID);
    m_statusDlg.Create(IDD_DIG_STATUS, &m_remoteDlg);
    return 0;
}

int CClientController::Invoker(CWnd*& pMainWnd){
    pMainWnd = &m_remoteDlg;
    return m_remoteDlg.DoModal();
}

LRESULT CClientController::SendMessage(MSG msg){
    HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hEvent == INVALID_HANDLE_VALUE)return -2;
    MSGINFO info(msg);
    PostThreadMessage(m_hThreadID, WM_SEND_MESSAGE, (WPARAM)&info, (LPARAM)&hEvent);
    WaitForSingleObject(hEvent, INFINITE);
    CloseHandle(hEvent);
    return info.result;
}

int CClientController::SendCommandPacket(int nCmd, bool bAutoClose, BYTE* pData, size_t nlength,std::list<CPacket>* plstPacks) {
    TRACE("cmd: %d %s start %lld \r\n", nCmd, __FUNCTION__, GetTickCount64());
    CClientSocket* pClient = CClientSocket::getInstance();
	//if (pClient->InitSocket() == false)return false;
    HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    std::list<CPacket>lstPacks;
    if (plstPacks == nullptr)plstPacks = &lstPacks;
    pClient->SendPacket(CPacket(nCmd, pData, nlength, hEvent), lstPacks);
    CloseHandle(hEvent);//回收时间句柄，防止资源耗尽
    //pClient->Send(CPacket(nCmd, pData, nlength, hEvent));
	//int cmd = DealCommand();
	/*TRACE("ack:%d\r\n", cmd);
	if (bAutoClose)CloseSocket();*/
    if (plstPacks->size() > 0) {
        if (plstPacks->size() == 1)return plstPacks->front().sCmd;
        TRACE("%s start %lld \r\n", __FUNCTION__, GetTickCount64());
    }
    TRACE("%s start %lld \r\n", __FUNCTION__, GetTickCount64());
    return -1;
}

int CClientController::DownFile(CString strPath) {
	CFileDialog dlg(FALSE, NULL, strPath, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, NULL, &m_remoteDlg);
	if (dlg.DoModal() == IDOK) {
		m_strRemote = strPath;
		m_strLocal = dlg.GetPathName();
		m_hThreadDownLoad = (HANDLE)_beginthread(&CClientController::threadDownLoadFileEntry, 0, this);
		if (WaitForSingleObject(m_hThreadDownLoad, 0) != WAIT_TIMEOUT) {
			AfxMessageBox(_T("下载文件线程开启无效！！！"));
			return -1;
		}
		m_remoteDlg.BeginWaitCursor();
		m_statusDlg.m_info.SetWindowText(_T("命令正在执行中"));
	}
}

void CClientController::StartWatchScreen(){
    m_isClosed = false;
    m_hThreadWatch = (HANDLE)_beginthread(CClientController::threadWatchScreenEntry, 0, this);
    m_watchDlg.DoModal();
    m_isClosed = true;
    WaitForSingleObject(m_hThreadWatch, 500);
}

void CClientController::threadDownLoadFile()
{
    FILE* file = fopen(m_strLocal, "wb+");
    if (file == NULL) {
		AfxMessageBox(_T("本体没有权限保存该文件,或者无法创建文件！！！"));
		m_statusDlg.ShowWindow(SW_HIDE);
		m_remoteDlg.EndWaitCursor();
		return;
    }
    CClientSocket* pClient = CClientSocket::getInstance();
    do {
        int ret = SendCommandPacket(4, false, (BYTE*)(LPCTSTR)m_strRemote, m_strRemote.GetLength());
        long long length = *(long long*)CClientSocket::getInstance()->GetPacket().strData.c_str();
        if (length == 0) {
			AfxMessageBox("文件长度为零或者无法读取文件！！！");
			break;
        }
        long long nCount = 0;
        while (nCount < length) {
            ret = pClient->DealCommand();
            if (ret < 0) {
				AfxMessageBox("传输失败！！！");
				TRACE("传输失败：ret = % d\r\n", ret);
				break;
            }
            fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), file);
            nCount += pClient->GetPacket().strData.size();
        }
    } while (false);
    fclose(file);
    CloseSocket();
	m_statusDlg.ShowWindow(SW_HIDE);
	m_remoteDlg.EndWaitCursor();
	m_remoteDlg.MessageBox(_T("下载完成！"), _T("完成"));
}

void CClientController::threadWatchScreen()
{
    Sleep(50);
    ULONGLONG tick = GetTickCount64();
    while (!m_isClosed) {
		if (GetTickCount64() - tick <= 100)
			continue;
		tick = GetTickCount64();
        if (m_watchDlg.isFull() == false) {
            std::list<CPacket>lstPacks;
            int ret = SendCommandPacket(6, true, NULL, 0, &lstPacks);
            if (ret == 6) {
                if (CWTool::ByteToImage(m_watchDlg.GetImage(), lstPacks.front().strData) == 0) {
					TRACE("成功设置了图片！%08X\r\n", (HBITMAP)m_watchDlg.GetImage());
					TRACE("和校验：%04x\r\n", lstPacks.front().sSum);
                    m_watchDlg.SetImageStatus(true);
                }
				else 
					TRACE("获取图片失败！ret=%d\r\n", ret);
            }
        }
        Sleep(1);
    }
}

void CClientController::threadDownLoadFileEntry(void* arg)
{
    CClientController* thiz = (CClientController*)arg;
    thiz->threadDownLoadFile();
    _endthread();
}

void CClientController::threadWatchScreenEntry(void* arg){
    CClientController* thiz = (CClientController*)arg;
    thiz->threadWatchScreen();
    _endthread();
}

CClientController::CClientController() :m_statusDlg(&m_remoteDlg), m_watchDlg(&m_remoteDlg) {
	m_hThread = INVALID_HANDLE_VALUE;
	m_hThreadDownLoad = INVALID_HANDLE_VALUE;
	m_hThreadWatch = INVALID_HANDLE_VALUE;
	m_hThreadID = -1;
	m_isClosed = true;
}

void CClientController::threadFunc(){
    MSG msg;
    while (::GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_SEND_MESSAGE) {
            MSGINFO* pmsg = (MSGINFO*)msg.wParam;
            HANDLE hEvent = (HANDLE)msg.lParam;
            auto it = m_mapFunc.find(msg.message);
            if (it != m_mapFunc.end())
                pmsg->result = (this->*it->second)(pmsg->msg.message, pmsg->msg.wParam, pmsg->msg.lParam);
            else
                pmsg->result = -1;
            SetEvent(hEvent);
        }
        else {
            std::map<UINT, MSGFUNC>::iterator it = m_mapFunc.find(msg.message);
            if (it != m_mapFunc.end()) {
                (this->*it->second)(msg.message, msg.wParam, msg.lParam);
            }
        }
    }
}

unsigned int _stdcall CClientController::threadEntry(void* arg){
    CClientController* thiz = (CClientController*)arg;
    thiz->threadFunc();
    _endthreadex(0);
    return 0;
}

//LRESULT CClientController::OnSendPack(UINT msg, WPARAM wParam, LPARAM lParam)
//{
//	CClientSocket* pClient = CClientSocket::getInstance();
//	CPacket* pPacket = (CPacket*)wParam;
//	return pClient->Send(*pPacket);
//}

//LRESULT CClientController::OnSendData(UINT nMsg, WPARAM wParam, LPARAM lParam)
//{
//    CClientSocket* pClient = CClientSocket::getInstance();
//    const char* buffer = (const char*)wParam;
//    return pClient->Send(buffer, (int)lParam);
//}

LRESULT CClientController::OnShowStatus(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    return m_statusDlg.ShowWindow(SW_SHOW);
}

LRESULT CClientController::OnShowWatch(UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    return m_watchDlg.DoModal();
}
