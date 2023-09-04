﻿// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "CServerSocket.h"
#include "CWTool.h"
#include "Command.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#pragma warning(disable:4996)

using namespace std;

// 唯一的应用程序对象

CWinApp theApp;

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr){
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0)){
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else{
            Command cmd;
            CServerSocket* pserver = CServerSocket::getInstance();
            int ret = pserver->Run(&Command::RunCommand, &cmd);
            switch (ret) {
			case -1:
				MessageBox(NULL, _T("网络初始化异常，未能成功初始化，请检查网络状态"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
				exit(0);
				break;
			case -2:
				MessageBox(NULL, _T("多次无法接入用户，结束程序"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
				exit(0);
				break;
			default:
				break;
            }
        }
    }
    else{
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }
    return nRetCode;
}
