
#include "pch.h"
#include "link.h"



BEGIN_MESSAGE_MAP(CLCheckBox, CButton)
    //{{AFX_MSG_MAP(CLink)
    ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CLCheckBox::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	// TODO: Add your message handler code here

	CRect rect;


	GetClientRect(&rect);
	CRect BoxRect;
	BoxRect=rect;
	BoxRect.right =BoxRect.left +15;
	dc.DrawFrameControl(BoxRect,DFC_BUTTON,DFCS_BUTTONCHECK|GetCheck()?DFCS_CHECKED :0);

	CFont   myFont;  
	myFont.CreatePointFont (90,_T("����"));
	CFont   *pOldFont=(CFont   *)dc.SelectObject   (&myFont);  
	dc.SetBkMode(TRANSPARENT);   

	CString StrWndText;
	GetWindowText(StrWndText);

	rect.OffsetRect (18,0);
	dc.DrawText(StrWndText,   rect,   DT_LEFT|DT_VCENTER|DT_SINGLELINE);   
	dc.SelectObject   (pOldFont);  
	myFont.DeleteObject   ();  
	// Do not call CButton::OnPaint() for painting messages
}


#define TIP_ID 1

CLink::CLink()
: m_bHighLight(FALSE)
{
    m_txtColor = RGB(0,0,0);
	m_bHighLight = FALSE;
    m_bOver      = false; 
	m_bEnTooltip = 1;
	m_hLinkCursor=NULL; //��ֵ����ΪNULL���������û���ڳ��������ø�ֵ���ͻ�ʹ��Ĭ�ϵ����ι��
}

CLink::~CLink()
{}

/////////////////////////////////////////////////////////////////////////////

BOOL CLink::DestroyWindow() 
{
    KillTimer(1);	
	return CStatic::DestroyWindow();
}

BOOL CLink::PreTranslateMessage(MSG* pMsg) 
{
	if( m_bEnTooltip)
		m_ToolTip.RelayEvent(pMsg);
    return CStatic::PreTranslateMessage(pMsg);
}



void CLink::PreSubclassWindow() 
{
    DWORD dwStyle = GetStyle();  //�޸����ԣ�ʹ֮�ɽ�����Ϣ
    ::SetWindowLong(GetSafeHwnd(), GWL_STYLE, dwStyle | SS_NOTIFY);
    
    //SetDefaultCursor();     //����Ĭ�Ϲ��Ϊ���ι��
   if( m_bEnTooltip)
   {
	   CRect rect; 
	   GetClientRect(rect);
	   m_ToolTip.Create(this);
	   m_ToolTip.SetDelayTime(100);
	   m_ToolTip.SetMaxTipWidth(200);
	   //m_ToolTip.AddTool(this, _T("test"), rect, TIP_ID);    
   }
    CStatic::PreSubclassWindow();
}

BOOL CLink::Create(LPCTSTR lpszText, DWORD dwStyle,
				const RECT& rect, CWnd* pParentWnd, UINT nID )
{ 
    if(0)
    {
       if( m_bEnTooltip)
       {
	       CRect rect(0,0,100,100); 
	       //GetClientRect(rect);
	       m_ToolTip.Create(this);
	       m_ToolTip.SetDelayTime(100);
	       m_ToolTip.SetMaxTipWidth(200);
	       //m_ToolTip.AddTool(this, _T("test"), rect, TIP_ID);    
           m_ToolTip.AddTool(this, _T(""), rect, TIP_ID);        
       }
    } 
	m_bEnTooltip = 0;
    BOOL ret = CStatic::Create(lpszText,dwStyle,rect,pParentWnd,nID);
    return ret;
}

BEGIN_MESSAGE_MAP(CLink, CStatic)
    //{{AFX_MSG_MAP(CLink)
    ON_WM_CTLCOLOR_REFLECT()
    ON_WM_SETCURSOR()
    ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

HBRUSH CLink::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	if( m_bHighLight )
	{
		if (m_bOver )
			pDC->SetTextColor(RGB(255,   0,   0));
		else
			pDC->SetTextColor( m_txtColor);
	}
	else
		pDC->SetTextColor(m_txtColor);
    pDC->SetBkMode(TRANSPARENT);   //���ñ�����͸����
    return (HBRUSH)GetStockObject(NULL_BRUSH);
}

void CLink::OnMouseMove(UINT nFlags, CPoint point) 
{ 
	//m_bOver = true;
	//Invalidate();
	//SetTimer(1, 100, NULL); //����һ����ʱ����������������뿪ʱ����ɫ
	
    CStatic::OnMouseMove(nFlags, point);
}

void CLink::OnTimer(UINT_PTR nIDEvent) 
{
    CPoint pt(::GetMessagePos());
    ScreenToClient(&pt);

    CRect rc;
    GetClientRect(rc);

    if (!rc.PtInRect(pt))
    {
        m_bOver = false;
        KillTimer(1);        
        Invalidate();
    }    
	CStatic::OnTimer(nIDEvent);
}

BOOL CLink::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/) 
{
    if (m_bHighLight||m_hLinkCursor)  //��������˹�꣬��ʹ�������õ� ���
    {
        ::SetCursor(m_hLinkCursor);
        return true;
    }
    return false;
}


/////////////////////////////////////////////////////////////////////////////
void CLink::SetToolTipText(CString str)
{
   if( m_bEnTooltip)
		m_ToolTip.UpdateTipText(str,this,TIP_ID);
}

void CLink::SetLinkCursor(HCURSOR hCursor)
{ 
    m_hLinkCursor = hCursor;
    if (m_hLinkCursor == NULL)
        SetDefaultCursor();
}


void CLink::SetDefaultCursor()   //��winhlp32.exe����ȡ���ι�꣬����Ĭ�ϵĹ��
{    
	CString str;
	::GetWindowsDirectory(str.GetBuffer(MAX_PATH), MAX_PATH);
	str.ReleaseBuffer();
	str += _T("\\winhlp32.exe");
	
	HMODULE hModule = ::LoadLibrary(str);
	if (hModule) 
	{
		HCURSOR hHandCursor = ::LoadCursor(hModule, MAKEINTRESOURCE(106));
		if (hHandCursor)
			m_hLinkCursor =CopyCursor(hHandCursor);
	}
	FreeLibrary(hModule);
	
}



BOOL CLink::HighLight(BOOL HighlightEnable)
{
	BOOL old = m_bHighLight;
	m_bHighLight = HighlightEnable;
	return old;
}

void CLink::SetText(LPCTSTR  lpszString)
{
	
	LONG gwl_style = GetWindowLong(this->GetSafeHwnd(),GWL_STYLE);
	BOOL visble = gwl_style & WS_VISIBLE;

	ShowWindow(SW_HIDE);
	SetWindowText(lpszString);
	if( visble  )
		ShowWindow(SW_SHOW);
}


void CLink::SetTxtColor(COLORREF clrTxt)
{
	m_txtColor = clrTxt;
}