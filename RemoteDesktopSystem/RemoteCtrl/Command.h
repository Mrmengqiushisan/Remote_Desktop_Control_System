#pragma once
#pragma warning(disable:4996)
#include<map>
#include<atlimage.h>
#include<direct.h>
#include"CWTool.h"
#include<stdio.h>
#include<io.h>
#include"LockDialog.h"
#include"Resource.h"
#include"CServerSocket.h"
#include "Packet.h"
#include <list>
class Command
{
public:
	Command();
	~Command() = default;
	int ExcuteCommand(int nCmd, std::list<CPacket>&, CPacket&);
	static void RunCommand(void* arg, int status, std::list<CPacket>& lstPacket, CPacket& inPacket) {
		Command* thiz = (Command*)arg;
		if (status > 0) {
			int ret = thiz->ExcuteCommand(status, lstPacket, inPacket);
			if(ret!=0)TRACE("ִ������ʧ��:%d,ret=%d\r\n", status, ret);
		}
		else {
			MessageBox(NULL, _T("�޷����������û�����������"), _T("�û�����ʧ�ܣ�"), MB_OK | MB_ICONERROR);
		}
	}
private:
	typedef int(Command::* CMDFUNC)(std::list<CPacket>&, CPacket&);
	std::map<int, CMDFUNC>m_mapFunction;//������ŵ����ܵĴ���
	CLockDialog dlg;
	unsigned threadId;
protected:
	int MakeDriverInfo(std::list<CPacket>& lstPacket, CPacket& inPacket) {
		std::string result;
		for (int i = 1; i <= 26; i++) {
			if (_chdrive(i) == 0) {
				if (result.size() > 0)result += ',';
				result += 'A' + i - 1;
			}
		}
		result += ',';
		lstPacket.push_back(CPacket(1, (BYTE*)result.c_str(), result.size()));
		return 0;
	}

	int MakeDriectoryInfo(std::list<CPacket>& lstPacket, CPacket& inPacket) {
		std::string strPath = inPacket.GetStrData();
		if (_chdir(strPath.c_str()) != 0) {
			FILEINFO finfo;
			finfo.hasNext = FALSE;
			lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
			OutputDebugString(_T("û��Ȩ�޷���Ŀ¼����"));
			return -2;
		}
		_finddata_t fdata;////�����ļ�����
		intptr_t hfind = _findfirst("*", &fdata);//���е���д�����ȽϺ�
		if (hfind == -1) {
			OutputDebugString(_T("û���ҵ��κ��ļ�����"));
			FILEINFO finfo;
			finfo.hasNext = FALSE;
			lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
			return -3;
		}
		int servercount{ 0 };
		do {
			FILEINFO finfo;
			finfo.isDirectory = (fdata.attrib & _A_SUBDIR) != 0;
			memcpy(finfo.szFileName, fdata.name, sizeof(fdata.name));
			TRACE("%s \r\n", finfo.szFileName);
			lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
			servercount++;
		} while (!_findnext(hfind, &fdata));
		TRACE("servercount:%d\r\n", servercount);
		//������Ϣ�����ƶ�
		FILEINFO finfo;
		finfo.hasNext = FALSE;
		lstPacket.push_back(CPacket(2, (BYTE*)&finfo, sizeof(finfo)));
		return 0;
	}
	//���ļ�
	int RunFile(std::list<CPacket>& lstPacket, CPacket& inPacket) {
		std::string strPath = inPacket.GetStrData();
		ShellExecuteA(NULL, NULL, strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
		lstPacket.push_back(CPacket(3, NULL, 0));
		return 0;
	}
	//�����ļ�
	int DownLoadFile(std::list<CPacket>& lstPacket, CPacket& inPacket) {
		std::string strPath = inPacket.GetStrData();
		long long data{ 0 };
		FILE* pFile = NULL;
		errno_t ret = fopen_s(&pFile, strPath.c_str(), "rb");
		if (ret != 0) {
			lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));
			return -1;
		}
		if (pFile != NULL) {
			fseek(pFile, 0, SEEK_END);
			data = _ftelli64(pFile);
			lstPacket.push_back(CPacket(4, (BYTE*)&data, 8));
			fseek(pFile, 0, SEEK_SET);
			char buffer[1024]{};
			size_t rLen = 0;
			do {
				rLen = fread(buffer, 1, sizeof(buffer), pFile);
				lstPacket.push_back(CPacket(4, (BYTE*)buffer, rLen));
			} while (rLen >= 1024);
			fclose(pFile);
		}
		lstPacket.push_back(CPacket(4, NULL, 0));
		return 0;
	}

	int MouseEvent(std::list<CPacket>& lstPacket, CPacket& inPacket) {
		MOUSEEV mouse;
		memcpy(&mouse, inPacket.GetStrData().c_str(), sizeof(MOUSEEV));
		DWORD nFlags = 0;
		switch (mouse.nButton)
		{
		case 0://���
			nFlags = 1;
			break;
		case 1://�Ҽ�
			nFlags = 2;
			break;
		case 2://�н�
			nFlags = 4;
			break;
		case 4://û�а���
			nFlags = 8;
			break;
		default:
			break;
		}
		if (nFlags != 8)SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);
		switch (mouse.nAction)
		{
		case 0://����
			nFlags |= 0x10;
			break;
		case 1://˫��
			nFlags |= 0x20;
			break;
		case 2://����
			nFlags |= 0x40;
			break;
		case 3://�ſ�
			nFlags |= 0x80;
			break;
		default:
			break;
		}
		TRACE("mouse move: %08x x: %d y: %d\r\n", nFlags, mouse.ptXY.x, mouse.ptXY.y);
		switch (nFlags) {
		case 0x21://���˫��
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x11://�������
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x41://�������
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x81://����ſ�
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x22://�Ҽ�˫��
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x12://�Ҽ�����
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x42://�Ҽ�����
			mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x82://�Ҽ��ſ�
			mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x24://�м�˫��
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
		case 0x14://�м�����
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x44://�м�����
			mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x84://�м��ſ�
			mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
			break;
		case 0x08://����������ƶ�
			mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
			break;
		default:
			break;
		}
		lstPacket.push_back(CPacket(4, NULL, 0));
		return 0;
	}

	int DeleteLoaclFile(std::list<CPacket>& lstPacket, CPacket& inPacket) {
		std::string strPath = inPacket.GetStrData();
		TCHAR sPath[MAX_PATH] = _T("");
		MultiByteToWideChar(CP_ACP, 0, strPath.c_str(), strPath.size(), sPath, sizeof(sPath) / sizeof(TCHAR));//�����ֽ�ת��Ϊ���ֽ�
		DeleteFileA(strPath.c_str());
		lstPacket.push_back(CPacket(9, NULL, 0));
		return 0;
	}

	int SendScreen(std::list<CPacket>& lstPacket, CPacket& inPacket) {
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
			lstPacket.push_back(CPacket(6, pData, nSize));
			GlobalUnlock(hMem);
		}
		pStream->Release();
		GlobalFree(hMem);
		screen.ReleaseDC();
		return 0;
	}
	static unsigned int _stdcall threadLockDlg(void* arg) {
		Command* thiz = (Command*)arg;
		thiz->threadLockDlgMain();
		_endthreadex(0);
		return 0;
	}
	void threadLockDlgMain() {
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
		//�����ö�
		dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		ShowCursor(false);//���������Ϊ���ɼ���
		::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);//������������
		//������귶Χ
		rect.right = rect.left + 1;
		rect.bottom = rect.top + 1;
		ClipCursor(rect);
		MSG msg;//��Ϣѭ��
		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_KEYDOWN) {
				TRACE("msg:%08X wparam:%08X lparam:%08X\r\n", msg.message, msg.wParam, msg.lParam);
				if (msg.wParam == 0x41)
					break;
			}
		}
		//�ָ�����������
		ShowCursor(true);
		::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
		dlg.DestroyWindow();
	}
	int LockMachine(std::list<CPacket>& lstPacket, CPacket& inPacket) {
		//m_hWnd���ڱ�ʾ���ڵľ��
		if (dlg.m_hWnd == NULL || dlg.m_hWnd == INVALID_HANDLE_VALUE) {
			_beginthreadex(NULL, 0, threadLockDlg, NULL, 0, &threadId);
			TRACE("threadId=%d\r\n", threadId);
		}
		lstPacket.push_back(CPacket(7, NULL, 0));
		return 0;
	}
	int UnLockMachine(std::list<CPacket>& lstPacket, CPacket& inPacket) {
		PostThreadMessage(threadId, WM_KEYDOWN, 0x41, 0x01E0001);
		lstPacket.push_back(CPacket(8, nullptr, 0));
		return 0;
	}
	int TestConnect(std::list<CPacket>& lstPacket, CPacket& inPacket) {
		lstPacket.push_back(CPacket(1981, NULL, 0));
		return 0;
	}
};

