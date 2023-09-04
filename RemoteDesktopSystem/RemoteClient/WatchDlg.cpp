// WatchDlg.cpp: 实现文件
//

#include "pch.h"
#include "RemoteClient.h"
#include "afxdialogex.h"
#include "WatchDlg.h"
#include "RemoteClientDlg.h"

// CWatchDlg 对话框

IMPLEMENT_DYNAMIC(CWatchDlg, CDialog)

CWatchDlg::CWatchDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG_WATCH, pParent)
{
	m_nObjHeight = -1;
	m_nObjWidth = -1;
}

CWatchDlg::~CWatchDlg()
{
}

void CWatchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDlg, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_WATCH, &CWatchDlg::OnStnClickedWatch)
	ON_BN_CLICKED(IDC_BTN_LOCK, &CWatchDlg::OnBnClickedBtnLock)
	ON_BN_CLICKED(IDC_BTN_UNLOCK, &CWatchDlg::OnBnClickedBtnUnlock)
END_MESSAGE_MAP()


// CWatchDlg 消息处理程序


void CWatchDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 0) {
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		if (pParent->isFull()) {
			CRect rect;
			m_picture.GetWindowRect(rect);
			if (m_nObjWidth == -1)m_nObjWidth = pParent->GetImage().GetWidth();
			if (m_nObjHeight == -1)m_nObjHeight = pParent->GetImage().GetHeight();
			pParent->GetImage().StretchBlt(m_picture.GetDC()->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), SRCCOPY);
			m_picture.InvalidateRect(NULL);
			pParent->GetImage().Destroy();
			pParent->SetImageStatus();
		}
	}
	CDialog::OnTimer(nIDEvent);
}

CPoint CWatchDlg::UserPointToRemoteScreenPoint(CPoint& point, bool isScreen)
{
	CRect clientRect;
	if (isScreen)ScreenToClient(&point);////全局坐标到用户坐标
	TRACE("x=%d y=%d\r\n", point.x, point.y);
	m_picture.GetWindowRect(clientRect);
	return CPoint(point.x * m_nObjWidth / clientRect.Width(), point.y * m_nObjHeight / clientRect.Height());
}

BOOL CWatchDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetTimer(0, 45, nullptr);
	
	return TRUE;
}


void CWatchDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1) {
		CPoint remote = UserPointToRemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;
		event.nAction = 1;
		/*CClientSocket* pClient = CClientSocket::getInstance();
		CPacket pack(5, (BYTE*)&event, sizeof(event));
		pClient->Send(pack);*/
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1) {
		TRACE("client x:%d y:%d\r\n", point.x, point.y);
		CPoint remote = UserPointToRemoteScreenPoint(point);
		TRACE("server x:%d y:%d\r\n", point.x, point.y);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;
		event.nAction = 2;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1) {
		CPoint remote = UserPointToRemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;
		event.nAction = 3;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDlg::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1) {
		CPoint remote = UserPointToRemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;
		event.nAction = 1;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1) {
		CPoint remote = UserPointToRemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;
		event.nAction = 2;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1) {
		CPoint remote = UserPointToRemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;
		event.nAction = 3;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1) {
		CPoint remote = UserPointToRemoteScreenPoint(point);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 8;
		event.nAction = 0;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
	CDialog::OnMouseMove(nFlags, point);
}

void CWatchDlg::OnStnClickedWatch()
{
	if (m_nObjWidth != -1 && m_nObjHeight != -1) {
		CPoint point;
		GetCursorPos(&point);
		CPoint remote = UserPointToRemoteScreenPoint(point, true);
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;
		event.nAction = 0;
		CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);
	}
}


void CWatchDlg::OnBnClickedBtnLock()
{
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 7 << 1 | 1);
}


void CWatchDlg::OnBnClickedBtnUnlock()
{
	CRemoteClientDlg* pParent = (CRemoteClientDlg*)GetParent();
	pParent->SendMessage(WM_SEND_PACKET, 8 << 1 | 1);
}
