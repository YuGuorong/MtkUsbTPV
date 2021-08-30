#pragma once
#include "ExDialog.h"
#include "SubPanelDlg.h"
#include "CPannel.h"
#include "afxwin.h"
#include <set>
#include <map>
using namespace std;

#define MAX_DEV_SUPPORT 16

#define CREATE_SUB_WND(v, cls, parent)  { v = new cls(parent); v->Create(cls::IDD, parent);v->ShowWindow(SW_SHOW);}  
// CMainFrame 对话框

class CMainFrame : public CExDialog
{
	DECLARE_DYNAMIC(CMainFrame)

public:
	CMainFrame(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CMainFrame();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MAIN_FRAME };
#endif
	CArray <CExDialog*, CExDialog*> m_oPannels;
	CArray <CStatic*, CStatic*>m_oFrames;
	map<CString, void*>m_mapDev;
	int   m_scanUsbTimes;
	int   MAX_PANNEL_NUM;
	int   m_dispPannelNum;
	int   m_pannles_state;
	BOOL m_bDefaultALl;
	CStatic m_strNewVersion;
	CString m_sPannelMax;
	INT   m_nPannelMax;
	HICON m_hIcon;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnPaint();
	afx_msg LRESULT OnMyDeviceChange(WPARAM wParam, LPARAM lParam);
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	BOOL CheckSubPannelRunning(BOOL bmessagebox = FALSE );
	afx_msg void OnCbnSelchangeComboItems();
	CComboBox m_cbPannelsNum;
};
