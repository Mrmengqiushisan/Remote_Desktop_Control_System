﻿#pragma once
#include "afxdialogex.h"


// CWatchDlg 对话框

class CWatchDlg : public CDialog
{
	DECLARE_DYNAMIC(CWatchDlg)

public:
	CWatchDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CWatchDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_WATCH };
#endif
public:
	int m_nObjWidth;
	int m_nObjHeight;
	inline bool isFull()const {
		return m_isFull;
	}
	inline void SetImageStatus(bool isFull = false) {
		m_isFull = isFull;
	}
	inline CImage& GetImage() {
		return m_image;
	}
protected:
	bool	m_isFull;	//缓存是否有数据 true 表示有缓存数据 false 表示没有缓存数据
	CImage	m_image;//缓存
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CPoint UserPointToRemoteScreenPoint(CPoint& point, bool isScreen = false);
	virtual BOOL OnInitDialog()override;
	CStatic m_picture;
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnStnClickedWatch();
	afx_msg void OnBnClickedBtnLock();
	afx_msg void OnBnClickedBtnUnlock();
};
