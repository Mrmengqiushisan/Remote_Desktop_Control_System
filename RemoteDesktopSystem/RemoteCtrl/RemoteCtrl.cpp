// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "CServerSocket.h"
#include <direct.h>
#include <stdio.h>
#include <io.h>
#include <atlimage.h>
#include "CWTool.h"
#include "LockDialog.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#pragma warning(disable:4996)

using namespace std;

// 唯一的应用程序对象

CWinApp theApp;

int MakeDriverInfo() {
    std::string result;
    for (int i = 1; i <= 26; i++) {
        if (_chdrive(i) == 0) {
            if (result.size() > 0)result += ',';
            result += 'A' + i - 1;
        }
    }
	result += ',';
    CPacket packet(1, (BYTE*)result.c_str(), result.size());
    CWTool::Dump((BYTE*)result.c_str(), result.size());
    CServerSocket::getInstance()->Send(packet);
    return 0;
}

int MakeDriectoryInfo() {
    std::string strPath;
	if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
		OutputDebugString(_T("当前的命令，不是获取文件列表，命令解析错误！！"));
		return -1;
	}
    if (_chdir(strPath.c_str()) != 0) {
        FILEINFO finfo;
        finfo.hasNext = FALSE;
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		CServerSocket::getInstance()->Send(pack);
		OutputDebugString(_T("没有权限访问目录！！"));
		return -2;
    }
    _finddata_t fdata;////查找文件索引
	intptr_t hfind = _findfirst("*", &fdata);//这行单独写出来比较好
	if (hfind == -1) {
		OutputDebugString(_T("没有找到任何文件！！"));
		FILEINFO finfo;
		finfo.hasNext = FALSE;
		CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
		CServerSocket::getInstance()->Send(pack);
		return -3;
    }
	int servercount{ 0 };
    do {
        FILEINFO finfo;
        finfo.isDirectory = (fdata.attrib & _A_SUBDIR) != 0;
		memcpy(finfo.szFileName, fdata.name, sizeof(fdata.name));
		TRACE("%s \r\n", finfo.szFileName);
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
		servercount++;
    } while (!_findnext(hfind, &fdata));
	TRACE("servercount:%d\r\n", servercount);
	//发送信息到控制端
	FILEINFO finfo;
	finfo.hasNext = FALSE;
	CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
	CServerSocket::getInstance()->Send(pack);
	return 0;
}
//打开文件
int RunFile() {
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    CPacket packet(3, NULL, 0);
    CServerSocket::getInstance()->Send(packet);
    return 0;
}
//下载文件
int DownLoadFile() {
	std::string strPath;
	CServerSocket::getInstance()->GetFilePath(strPath);
	long long data{ 0 };
    FILE* pFile = NULL;
    errno_t ret = fopen_s(&pFile, strPath.c_str(), "rb");
    if (ret != 0) {
        CPacket packet(4, (BYTE*)&data, 8);
        CServerSocket::getInstance()->Send(packet);
        return -1;
    }
    if (pFile != NULL) {
		fseek(pFile, 0, SEEK_END);
        data = _ftelli64(pFile);
        CPacket head(4, (BYTE*)&data, 8);
        CServerSocket::getInstance()->Send(head);
        fseek(pFile, 0, SEEK_SET);
        char buffer[1024]{};
        size_t rLen = 0;
        do {
            rLen = fread(buffer, 1, sizeof(buffer), pFile);
            CPacket pack(4, (BYTE*)buffer, rLen);
            CServerSocket::getInstance()->Send(pack);
        } while (rLen >= 1024);
        fclose(pFile);
    }
    CPacket pack(4, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;

}

int MouseEvent() {
    MOUSEEV mouse;
    if (CServerSocket::getInstance()->GetMouseEvent(mouse)) {
		DWORD nFlags = 0;
		switch (mouse.nButton)
		{
		case 0://左键
			nFlags = 1;
			break;
		case 1://右键
			nFlags = 2;
			break;
		case 2://中建
			nFlags = 4;
			break;
		case 4://没有按键
			nFlags = 8;
			break;
		default:
			break;
		}
        if (nFlags != 8)SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
		switch (mouse.nAction)
		{
		case 0://单击
			nFlags |= 0x10;
			break;
		case 1://双击
			nFlags |= 0x20;
			break;
		case 2://按下
			nFlags |= 0x40;
			break;
		case 3://放开
			nFlags |= 0x80;
			break;
		default:
			break;
		}
		TRACE("mouse move: %08x x: %d y: %d\r\n", nFlags, mouse.ptXY.x, mouse.ptXY.y);
        switch (nFlags){
		case 0x21://左键双击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x11://左键单击
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x41://左键按下
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x81://左键放开
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x22://右键双击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x12://右键单击
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x42://右键按下
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x82://右键放开
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x24://中键双击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x14://中键单击
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x44://中键按下
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x84://中键放开
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x08://单纯的鼠标移动
			mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
			break;
        default:
            break;
        }
		CPacket pack(4, NULL, 0);
		CServerSocket::getInstance()->Send(pack);
	}
	else {
		OutputDebugString(_T("获取鼠标操作参数失败！！"));
		return -1;
	}
	return 0;
}

int DeleteLoaclFile() {
	std::string strPath;
	CServerSocket::getInstance()->GetFilePath(strPath);
	TCHAR sPath[MAX_PATH] = _T("");
	MultiByteToWideChar(CP_ACP, 0, strPath.c_str(), strPath.size(), sPath, sizeof(sPath) / sizeof(TCHAR));//将多字节转换为宽字节
	DeleteFileA(strPath.c_str());
	CPacket pack(9, NULL, 0);
	int ret = CServerSocket::getInstance()->Send(pack);
	TRACE("send ret=%d\r\n", ret);
	return 0;
}

int SendScreen() {
	CImage screen;
	HDC hScreen = ::GetDC(NULL);
	int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);
	int nWidth = GetDeviceCaps(hScreen, HORZRES);
	int nHeight = GetDeviceCaps(hScreen, VERTRES);
	screen.Create(nWidth, nHeight, nBitPerPixel);
	BitBlt(screen.GetDC(), 0, 0, 1920, 1020, hScreen, 0, 0, SRCCOPY);
	ReleaseDC(NULL, hScreen);
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
	if (hMem == NULL)return -1;
	IStream* pStream = NULL;
	HRESULT res = CreateStreamOnHGlobal(hMem, TRUE, &pStream);
	if (res == S_OK) {
		screen.Save(pStream, Gdiplus::ImageFormatPNG);
		//screen.Save(_T("test2023.tiff"), Gdiplus::ImageFormatTIFF);
		//screen.Save(_T("test2023.jpg"), Gdiplus::ImageFormatJPEG);
		LARGE_INTEGER bg{ 0 };
		pStream->Seek(bg, STREAM_SEEK_SET, NULL);
		PBYTE pData = (PBYTE)GlobalLock(hMem);
		SIZE_T nSize = GlobalSize(hMem);
		CPacket pack(6, pData, nSize);
		CServerSocket::getInstance()->Send(pack);
		GlobalUnlock(hMem);
	}
	pStream->Release();
	GlobalFree(hMem);
	screen.ReleaseDC();
	return 0;
}
CLockDialog dlg;
unsigned threadId{ 0 };
unsigned int _stdcall threadLockDlg(void* arg) {
	TRACE("%s[%d]:%d\r\n", __FUNCTION__, __LINE__, GetCurrentThreadId());
	dlg.Create(IDD_DIALOG_INFO, NULL);
	dlg.ShowWindow(SW_SHOW);
	CRect rect;
	rect.left = 0; rect.top = 0;
	rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
	rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
	rect.bottom += 20;
	TRACE("right=%d,bottom=%d\r\n", rect.right, rect.bottom);
	dlg.MoveWindow(rect);
	//窗口置顶
	dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	ShowCursor(false);//将鼠标设置为不可见的
	::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);//将任务栏隐藏
	//设置鼠标范围
	rect.right = rect.left + 1;
	rect.bottom = rect.top + 1;
	ClipCursor(rect);
	MSG msg;//消息循环
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_KEYDOWN) {
			TRACE("msg:%08X wparam:%08X lparam:%08X\r\n", msg.message, msg.wParam, msg.lParam);
			if (msg.wParam == 0x41)
				break;
		}
	}
	//恢复鼠标和任务栏
	ShowCursor(true);
	::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
	dlg.DestroyWindow();
	_endthreadex(0);
	return 0;
}
int LockMachine() {
	//m_hWnd用于表示窗口的句柄
	if (dlg.m_hWnd == NULL || dlg.m_hWnd == INVALID_HANDLE_VALUE) {
		_beginthreadex(NULL, 0, threadLockDlg, NULL, 0, &threadId);
		TRACE("threadId=%d\r\n", threadId);
	}
	CPacket pack(7, NULL, 0);
	CServerSocket::getInstance()->Send(pack);
	return 0;
}
int UnLockMachine() {
	PostThreadMessage(threadId, WM_KEYDOWN, 0x41, 0x01E0001);
	CPacket pack(8, nullptr, 0);
	CServerSocket::getInstance()->Send(pack);
	return 0;
}
int TestConnect() {
	CPacket pack(1981, NULL, 0);
	int ret = CServerSocket::getInstance()->Send(pack);
	TRACE("Send ret=%d\r\n", ret);
	return 0;
}
int ExcuteCommand(int nCmd) {
	int ret = 0;
	switch (nCmd)
	{
	case 1://查看磁盘分区
		ret = MakeDriverInfo();
		break;
	case 2://查看指定目录下的文件
		ret = MakeDriectoryInfo();
		break;
	case 3://打开文件
		ret = RunFile();
		break;
	case 4://下载文件
		ret = DownLoadFile();
		break;
	case 5://鼠标操作
		ret = MouseEvent();
		break;
	case 6://发送屏幕内容==>发送屏幕的截图
		ret = SendScreen();
		break;
	case 7://锁机
		ret = LockMachine();
		break;
	case 8://解锁
		ret = UnLockMachine();
		break;
	case 9:
		ret = DeleteLoaclFile();
		break;
	case 1981:
		ret = TestConnect();
		break;
	default:
		break;
	}
	return 0;
	/*Sleep(5000);
			UnLockMachine();
			TRACE("m_hwnd=%08X\r\n", dlg.m_hWnd);
			while (dlg.m_hWnd != nullptr) {
				Sleep(10);
			}*/
}
int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            CServerSocket* pserver = CServerSocket::getInstance();
            int count{ 0 };
            if (pserver->InitSocket() == false) {
				MessageBox(NULL, _T("网络初始化异常，未能成功初始化，请检查网络状态"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
				exit(0);
            }
            while (CServerSocket::getInstance() != NULL) {
                if (pserver->AcceptClient() == false) {
                    if (count >= 3) {
						MessageBox(NULL, _T("多次无法接入用户，结束程序"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
						exit(0);
                    }
					MessageBox(NULL, _T("无法正常接入用户，自动重试"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
					count++;
                }
				TRACE("AcceptClient return true\r\n");
                int ret = pserver->DealCommand();
				TRACE("DealCommand :%d\r\n", ret);
				if (ret > 0) {
					ret = ExcuteCommand(ret);
					if (ret != 0) {
						TRACE("执行命令失败:%d,ret=%d\r\n", pserver->GetPacket().GetCmd(), ret);
					}
					pserver->CloseClient();
					TRACE("Command has Done\r\n");
				}
            }
            
        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
