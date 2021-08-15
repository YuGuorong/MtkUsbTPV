/************************************************************************ 
* 文件名：    Link.h 
* 文件描述：  超级链接
* 版本号：    1.0 
************************************************************************/ 
#if !defined _HYPERLINK_H
#define _HYPERLINK_H

#if _MSC_VER >= 1000
#pragma once
#endif 

class CLCheckBox : public CButton
{
public:
	CLCheckBox(){};
	virtual ~CLCheckBox(){};
protected:
    //{{AFX_MSG(CLink)
    afx_msg void OnPaint();;
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

};
//-------------可设置背景的CStatic控件--------------
/////////////////////////////////////////////////////////////////////////////
// CBGStatic window

class AFX_EXT_CLASS CBGStatic : public CStatic
{
// Construction
public:
	CBGStatic();
	void SetBGColor(COLORREF ref);
	void SetTextColor(COLORREF ref, BOOL bredraw = 0);
	void SetFont(LOGFONT* lf);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBGStatic)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBGStatic();

	// Generated message map functions
public:
    BOOL     m_bTranslate;
	COLORREF m_refBg;
	COLORREF m_refText;
	CFont m_font;

	//{{AFX_MSG(CBGStatic)
	afx_msg void OnPaint();
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


class CLink : public CStatic
{
public:
    CLink();
    virtual ~CLink();

public:

    void SetToolTipText(CString str);
    void SetLinkCursor(HCURSOR hCursor);
    void SetDefaultCursor();

    //{{AFX_VIRTUAL(CLink)
	public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL DestroyWindow();
    virtual BOOL Create(LPCTSTR lpszText, DWORD dwStyle,
				const RECT& rect, CWnd* pParentWnd, UINT nID = 0xffff);
	protected:
    virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

protected:
    BOOL     m_bOver;   
    HCURSOR  m_hLinkCursor; 
    CToolTipCtrl m_ToolTip; 
	int      m_bEnTooltip;

   
protected:
    //{{AFX_MSG(CLink)
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
public:
	BOOL HighLight(BOOL HighlightEnable);
	BOOL m_bHighLight;
public:
	void SetText(LPCTSTR  lpszString);
	void SetTxtColor(COLORREF clrTxt);
	COLORREF m_txtColor;
};

typedef  CLink * LP_LINK;

//{{AFX_INSERT_LOCATION}}
#endif 