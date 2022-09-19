#pragma once

#include "pch.h"
//#include "MtkUsbTPV.h"
#include "CMainFrame.h"
#include "afxdialogex.h"

#include "CFtDevice.h"
#include "CTpvBoard.h"
#include <shellapi.h>
#include <initguid.h> 
#include <Dbt.h>
// CMainDlg 对话框

class CMainDlg : public CExDialog
{
	DECLARE_DYNAMIC(CMainDlg)

public:
	CMainDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CMainDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CMainDlg };
#endif
	HANDLE m_hEvent;
	uintptr_t m_threadMsg;
	CTpvBoard* m_pboard ;
	HICON m_hIcon;
	INT m_conn_state;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNotifyIcon(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnQueryConnection(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnMyDeviceChange(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnMExit();

	void SetNotifyIcon(int op, int iconid);
	void LoadDriver();
	afx_msg void OnMToggleWnd();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};
