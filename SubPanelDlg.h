#pragma once
#include "ExDialog.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "util.h"
#include "InvokeExternalExe.h" 

#define TMR_ID_SUB_DONE  2111

enum
{
	META_IDLE = 0,
	META_ALLOCATE = 1,
	META_BUSY = 0X100,
	META_REMOVED = 0X200,
	META_UPDATED = 0x400
};


typedef enum {
	INFO_APPEND = 0x0,
	INFO_REPLACE = 0x1,
	INFO_SELECTED = 0x2, //Need call SetInfoSel before SetInfoText 

	INFO_DEFAULT  =0x100,
	INFO_GREEN    =0x200,
	INFO_BLUE     =0x300,
	INFO_WARNING  =0x400,
	INFO_ALARM    =0x500,

}INFO_MODE;



// CSubPanelDlg 对话框

class CSubPanelDlg : public CExDialog
{
	DECLARE_DYNAMIC(CSubPanelDlg)

public:
	CSubPanelDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CSubPanelDlg();

// 对话框数据
#if 1//def AFX_DESIGN_TIME
	enum { IDD = IDD_SUBPANEL_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl m_oProc;
	CInvokeExternalExe * m_extexe;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	int StartRun();
	int StopRun();
	void SetIndex(int idx);
	CRichEditCtrl m_oLogInfo;
	CStatic m_oCaption;
	CFont * m_of;
	int m_state;
	CHARFORMAT2 m_cf;

	void OpenDevice(CString &strDevid, BOOL bAdbDev = TRUE);
	void ReOpenDevice();
	void QueryDevInfo();
	int DevcieRemvoed();
	void * m_pRomDesc;
	int  m_DevId; //index of genreation
	BOOL    m_bMount;//Devices is already be mounted. 	
	BOOL    m_bInit ;
	BOOL    m_bAdbDev;
	CString m_strDevId;  //Device adb serial number
	CString m_strDlFile; //??
	CString m_strVserion;//Rom version 
	CString m_strBrand;  //Brand and type information, for query rom&root
	CStringArray m_strPhoneInfo;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void SetInfoText(LPCTSTR str, int mode = INFO_DEFAULT|INFO_APPEND);
	void SetInfoSel(int start, int end);

	CDelegate * m_pBurn;
	virtual void DelegateRun();

	LPVOID * m_pRoms;
	BOOL  m_bReflash;
	void SetRomDescript(LPVOID * const pRoms);
	INT64  m_iiBurnedLen;
	DWORD  m_spare_len; //sending sparse 'system' (262097 KB)..
	CString m_strInvoke;
	void OnAsyncRunCallBack(const CHAR * pcons_txt);

	
	afx_msg void OnBnClickedBtnReflash();
	CEdit m_oConsol;
};

