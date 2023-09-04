
// RemoteClientDlg.h: 头文件
//

#pragma once
#include "ClientSocket.h"
#include "StatusDlg.h"
#include "WatchDlg.h"
#define WM_SEND_PACKET (WM_USER+1)//发送数据包消息
// CRemoteClientDlg 对话框
class CRemoteClientDlg : public CDialogEx
{
// 构造
public:
	CRemoteClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTECLIENT_DIALOG };
#endif

private:
	CImage	m_image;	//缓存
	bool	m_isFull;	//缓存是否有数据 true 表示有缓存数据 false 表示没有缓存数据
	bool	m_isClosed;	//监视是否关闭
	static void __cdecl threadEntryForDownFile(void* arg);
	static void __cdecl threadEntryForWatchData(void* arg);
	void threadWatchData();
	void threadDownFile();
public:
	inline bool isFull()const {
		return m_isFull;
	}
	inline CImage& GetImage() {
		return m_image;
	}
	inline void SetImageStatus(bool isFull = false) {
		m_isFull = isFull;
	}
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
private:
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
	int SendCommandPacket(int nCmd, bool bAutoClose = true, BYTE* pData = nullptr, size_t nlength = 0);
	//删除树中对象
	void DeleteTreeChildrenItem(HTREEITEM hTree);
	//获取路径
	CString GetPath(HTREEITEM hTree);
	//加载文件内容
	void LoadFileInfo();
	//重载文件
	void LoadFileCurrent();
// 实现
protected:
	HICON m_hIcon;
	CStatusDlg m_dlgStatus;
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnTest();
	DWORD m_server_address;
	CString m_server_port;
	afx_msg void OnBnClickedBtnFileinfo();
	CTreeCtrl m_Tree;
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	CListCtrl m_List;
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownloadFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnRunFile();
	afx_msg LRESULT OnSendPacket(WPARAM wParam, LPARAM lParam);//自定义消息函数
	afx_msg void OnBnClickedBtnStartWatch();
};
