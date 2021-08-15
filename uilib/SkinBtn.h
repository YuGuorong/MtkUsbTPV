/************************************************************************ 
* 文件名：    SkinBtn.h 
* 文件描述：  图片按钮 
* 创建人：    余国荣, 2009年03月22日
* 版本号：    1.0 
************************************************************************/ 

// CSkinBtn


#pragma once
#include <atlimage.h>       

class CSkinBtn : public CButton
{
	DECLARE_DYNAMIC(CSkinBtn)

public:
	CSkinBtn();
	virtual ~CSkinBtn();

	typedef enum state
	{
		NORMAL,
		HOVER,
		DOWN,
		DISABLE
	}state;

protected:
	DECLARE_MESSAGE_MAP()

	CImage  m_imgNormal;
	CImage  m_imgHover;
	CImage  m_imgDown;
	CImage  m_imgDisable;
	CImage  m_imgFocus;
    CImageList  m_imgList;

private:
	state m_state;
	COLORREF m_fg, m_bg;
	bool m_bMouseOver;
	bool m_bEnabled;
	CFont *m_pFont;
	
	bool m_bDCStored;//是否已经保存背景图
	
	HCURSOR		m_hCursor;			// Handle to cursor
	CImage m_hMouseInIcon;
	CImage m_hMouseOutIcon;
	CPoint m_textPos;
	CRect m_iconRect;

	CToolTipCtrl m_ToolTip;			// Tooltip


	CDC m_memDC;
	CRgn m_rgn;
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
protected:
	virtual void PreSubclassWindow();
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
protected:	
	//在按钮中填充颜色
	void DrawFilledRect(CDC *DC, CRect R, COLORREF color);
	//设置按钮上的字体颜色
	void DrawButtonText(CDC *DC, CRect R, CString str, COLORREF TextColor);
	void DrawButton();
public:
	void SetOwnerDraw(bool IsDraw);
	void SetImage(UINT nNormalID, UINT nHoverID, UINT nDownID, UINT nDisableID);
    void SetImage(UINT nImagesID, int x, int y);//0 NORMAL, 1 HOVER, 2 DOWN ,3 DISABLE, 4, fucos
	void SetImage(CString strNormal, CString strHover, CString strDown, CString strDisable);
	void SetIcon(CString  strMouseOut,CString strMouseIn);
	void SetColor(COLORREF fgcolor,COLORREF bgcolor);
	void SetTextPos(CPoint point);
	CImage* GetPaintImage(){return &m_imgNormal;}
	CImage* GetPaintIcon(){return &m_hMouseOutIcon;}
	CPoint GetTextPos(){return m_textPos;}
	COLORREF GetFGColor() { return m_fg; }	
	COLORREF GetBGColor() { return m_bg; }
	CRect GetRectInParent();
	CRect GetIconRect(){return m_iconRect;}
	DWORD SetBtnCursor(int nCursorId = NULL, BOOL bRepaint = TRUE);
    DWORD SetBtnCursor(LPTSTR nCursorId );
	enum SYTLE{ROUNDRECT,ELLIPSE,UPTRIANGLE,DOWNTRIANGLE}m_Style; // 按钮的外观样式
	void SetRgnStyle(SYTLE nStyle); // 设置按钮有效区域的样式
public:
	afx_msg void OnEnable(BOOL bEnable);
public:
	afx_msg void OnDestroy();
public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
    void SetTooltipText(LPCTSTR lpszText, BOOL bActivate);
    void SetTooltipText(int nText, BOOL bActivate);
    void InitToolTip();
    void ActivateTooltip(BOOL bActivate);

};

