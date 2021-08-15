#include "stdafx.h"

#pragma once

#ifndef _EX_DIALOG_H
#define _EX_DIALOG_H

#include "Link.h"
#include "resource.h"
//#include "controlbutton.h"
#include "officexpmenu.h"        // COfficeXPMenu class declaration
#include "SkinBtn.h"
#include "Util.h"
#include "GdipButton.h"

#define CREATE_SUB_WND(v, cls, parent)  { v = new cls(parent); v->Create(cls::IDD, parent);v->ShowWindow(SW_HIDE);}  

#define WM_SHOWTASK        (WM_USER + 901)



// CExDialog dialog
typedef  CLink * LP_LINK;

BOOL SetTransparent(HWND hWnd, int iTransparent) ;

#define EX_TITLE_BAR      (1)
#define EX_STRETCH_BK     (1<<1)
#define EX_FILL_BK        (1<<2)
#define EX_CLOSE_BTN      (1<<3)
#define DEFAULT_EX_STYLE  (EX_FILL_BK|EX_STRETCH_BK|EX_CLOSE_BTN)

class CExDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CExDialog)

public:
    CExDialog(UINT nIDTemplate,CWnd* pParent = NULL, INT ex_style=DEFAULT_EX_STYLE);  
    virtual ~CExDialog();

    CArray <CLCheckBox * , CLCheckBox* > m_aChkboxs;
	CArray <LP_LINK , LP_LINK > m_atxts;
    LP_LINK *m_pa_txts;
    int  m_nTxtCount;
    CBitmap m_backgrd;
	CDC *m_pBkDC;
	CDC *m_pTitDC;
	INT m_WndDiameter;
    INT m_bTransprent;
	INT m_ExStyle;
	int m_pop_state;
    void OnOK(){};   //Avoid ENTER key to exit dialog
	int m_TitleHight;
public:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDestroy();
public:
    int SubTextItems(const int * pIDs, const int * pstrIDs, const COLORREF * pColors, int nTotals);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
public:
    virtual BOOL OnInitDialog();
public:
	void SetTitleBar();
	void DrawTitleBar(CDC &dc);
	void SetFullScreen();
public:
	afx_msg void OnPaint();
	// Set round corner window style, ndiamt diameter.
	int SetWndStyle(int ndiamt, COLORREF crtl=RGB(45,100,217), COLORREF crbr=RGB(18,76,199));
	void GetTitleBarHight(void);

protected:
	COLORREF m_crtl, m_crbr;
	void CreatBkBmp();
public:
	 BOOL Create(UINT nIDTemplate, CWnd* pParentWnd = NULL);
	 afx_msg void OnSize(UINT nType, int cx, int cy);
};


#define IDC_CAPTION_START   0x8000
#define IDC_EDIT_START      0x8400
#define IDC_CITY_START      0x8800

#define LIST_CAPTION_ONLY   0
#define LIST_WITH_EDIT      1
#define LIST_WITH_CITY      -1
#define LIST_OFFSET         -2

#define MAX_CITY_NUM   1
#define MAX_OF_NC_RES  6

typedef struct tag_dlg_item_info
{
    LPCTSTR caption;
    INT  bEdit;
    LPCTSTR mask;
}DLG_ITEM_INFO;

typedef void (*CloseWndHook)(void);
extern CString g_strPath;

class CeExDialog : public CExDialog
{
	DECLARE_DYNAMIC(CeExDialog)

public:
    CeExDialog(UINT nIDTemplate,CWnd* pParent = NULL,  INT ex_style=DEFAULT_EX_STYLE|EX_TITLE_BAR);  
    virtual ~CeExDialog();
	virtual int ExEndDialog(INT code);
    CSkinBtn m_closeBtn, m_minBtn;

    CBGStatic * *m_pCaptions;
    int      m_num_cap ;
	CFont    m_font;
	COfficeXPMenu *m_pPopMenu;
	int      m_pop_subid;
    void CreateListItems(const DLG_ITEM_INFO * pinfos,int items, CRect &r, INT CaptionW);
    void RemoveListItems();

public:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDestroy();
public:
    virtual BOOL OnInitDialog();
public:
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
public:
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);


public:
    afx_msg void OnHideWindow();
public:
    afx_msg void OnCloseWindow();
public:
    virtual afx_msg LRESULT CeExDialog::OnTrayClickMsg(WPARAM wParam, LPARAM lParam);

public:
    void SetCityText(int idx, LPCTSTR strCity);
public:
    static void SetCloseToHide(BOOL bCloseToHide);
public:
    virtual void HtmlHelp(DWORD_PTR dwData, UINT nCmd = 0x000F);
public:
    virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	afx_msg void OnPaint();
public:
	// 最小化到托盘
	int ToTray(int menuid, int subid);
	// 删除托盘图标函数
	void DeleteTray();

};


#endif /*_EX_DIALOG_H*/