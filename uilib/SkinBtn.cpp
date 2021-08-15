/************************************************************************ 
* 文件名：    SkinBtn.h 
* 文件描述：  图片按钮 
* 创建人：    余国荣, 2009年03月22日
* 版本号：    1.0 
************************************************************************/ 
// SkinBtn.cpp : 实现文件
//

#include "pch.h"
#include "SkinBtn.h"


// CSkinBtn

IMPLEMENT_DYNAMIC(CSkinBtn, CButton)

CSkinBtn::CSkinBtn()
{
	m_state = NORMAL;
	m_pFont=new CFont();
	m_pFont->CreatePointFont(90,_T("Arial"));
	m_fg = RGB(0, 0, 64);
	m_bg = RGB(255, 255, 255);
	m_bMouseOver = false;
	m_bEnabled = true;  
	m_bDCStored = false;
	m_textPos = CPoint(0,0);
	m_iconRect = CRect(0,0,16,16);
	// No tooltip created
	m_ToolTip.m_hWnd = NULL;
}

CSkinBtn::~CSkinBtn()
{
	delete m_pFont;
	m_memDC.DeleteDC();
	m_rgn.DeleteObject();
}


BEGIN_MESSAGE_MAP(CSkinBtn, CButton)
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
	ON_WM_ENABLE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void CSkinBtn::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{

	//// TODO:  添加您的代码以绘制指定项
	//CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	UINT state = lpDIS->itemState; 
	//CRect rect;
	//rect.CopyRect(&lpDIS->rcItem); 

	m_state = NORMAL;

	if (state & ODS_FOCUS)
	{
		if (state & ODS_SELECTED)
		{ 
			m_state = DOWN;
		}
		else
		{
			if(m_bMouseOver)
				m_state = HOVER;
		}
	}
	else
	{
		m_state = NORMAL;
	}
	if (state & ODS_DISABLED)
	{
		m_state = DISABLE;
		m_bEnabled = false;
	}

	DrawButton();
}

void CSkinBtn::DrawButton()
{    
	CClientDC  dc(this);

	CRect	rect;
	GetClientRect(&rect);

    CRect rTxt ;
    this->GetWindowRect(rTxt );
    this->ScreenToClient(rTxt );

    GetClientRect(&rTxt);

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);

	CBitmap Screen;
	Screen.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	memDC.SelectObject(&Screen);
	Screen.DeleteObject();

	//画背景 
	memDC.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &m_memDC, 0, 0, SRCCOPY);

	CString str;
	GetWindowText(str);	

	memDC.SetBkMode(TRANSPARENT);    
	memDC.SelectObject(GetFont());
	
	switch(m_state)
	{
	case NORMAL:
		//memDC.TextOutW(m_textPos.x,m_textPos.y,str);
        if(m_imgList.GetSafeHandle())
        {
            m_imgList.Draw(&memDC,0, rect.TopLeft(), ILD_TRANSPARENT);
        }
		else if(!m_imgNormal.IsNull())
        {
			m_imgNormal.TransparentBlt(memDC, rect.left, rect.top, 
				m_imgNormal.GetWidth(), m_imgNormal.GetHeight(), RGB(255,0,255));
        }
		else if(!m_hMouseOutIcon.IsNull())
		{
			m_hMouseOutIcon.TransparentBlt(memDC, m_iconRect.left,m_iconRect.top, 
				m_iconRect.Width(), m_iconRect.Height(), RGB(0,0,0));
			memDC.SetTextColor(GetFGColor());
			memDC.TextOutW(m_textPos.x,m_textPos.y,str);
		}
		//DrawButtonText(&memDC, rect, str, GetFGColor());
		break;
	case HOVER:
		//DrawFilledRect(&memDC, rect, RGB(255,255,0));
        if(m_imgList.GetSafeHandle())
        {
            m_imgList.Draw(&memDC,1, rect.TopLeft(), ILD_TRANSPARENT);
        }

		else if(!m_imgHover.IsNull())
			m_imgHover.TransparentBlt(memDC, rect.left, rect.top, 
				m_imgHover.GetWidth(), m_imgHover.GetHeight(), RGB(255,0,255));
		else if(!m_hMouseInIcon.IsNull())
		{
			m_hMouseInIcon.TransparentBlt(memDC, m_iconRect.left,m_iconRect.top, 
				m_iconRect.Width(), m_iconRect.Height(), RGB(0,0,0));
			memDC.SetTextColor(GetBGColor());
			memDC.TextOutW(m_textPos.x,m_textPos.y,str);
		}
  		//DrawButtonText(&memDC, rect, str, GetFGColor());
		break;
	case DOWN:
		//DrawFilledRect(&memDC, rect, RGB(0,0,255)); 
        if(m_imgList.GetSafeHandle())
        {
			int idx = m_imgList.GetImageCount() > 2? 2 : m_imgList.GetImageCount()-1 ;
            m_imgList.Draw(&memDC, idx, rect.TopLeft(), ILD_TRANSPARENT);
        }

		else if(!m_imgDown.IsNull())
        {
			m_imgDown.TransparentBlt(memDC, rect.left+1, rect.top+1, 
				m_imgDown.GetWidth(), m_imgHover.GetHeight(), RGB(255,0,255));
            rTxt.OffsetRect(1,1);
        }
		else if(!m_hMouseOutIcon.IsNull())
		{
			m_hMouseOutIcon.TransparentBlt(memDC, m_iconRect.left+1,m_iconRect.top+1, 
				m_iconRect.Width(), m_iconRect.Height(), RGB(0,0,0));
			memDC.SetTextColor(GetBGColor());
			memDC.TextOutW(m_textPos.x+1,m_textPos.y+1,str);
		}
		//DrawButtonText(&memDC, rect, str, GetFGColor());
		break;
	case DISABLE:
        if(m_imgList.GetSafeHandle())
        {
			int idx = m_imgList.GetImageCount() > 3? 3 : m_imgList.GetImageCount()-1 ;
            m_imgList.Draw(&memDC,idx, rect.TopLeft(), ILD_TRANSPARENT);
        }

		else if(!m_imgDisable.IsNull())
			m_imgDisable.TransparentBlt(memDC, rect.left, rect.top, 
				m_imgDisable.GetWidth(), m_imgHover.GetHeight(), RGB(255,0,255));
		else if(!m_hMouseOutIcon.IsNull())
		{
			m_hMouseOutIcon.TransparentBlt(memDC, m_iconRect.left,m_iconRect.top, 
				m_iconRect.Width(), m_iconRect.Height(), RGB(0,0,0));
			memDC.SetTextColor(GetFGColor());
			memDC.TextOutW(m_textPos.x,m_textPos.y,str);
		}
		//DrawButtonText(&memDC, rect, str, RGB(128, 128, 128));
		break;
	default:
		break;
	}

	UINT btst = GetState();
	if( btst & BST_CHECKED  )
	{
		TRACE("CHECKED!\r\n");
	}
	else
		TRACE("UNCHECKED!\r\n");


	DrawButtonText(&memDC, rTxt, str, m_fg);
	dc.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);

	int cx, cy;
	ImageList_GetIconSize(m_imgList, &cx, &cy);  
	//dc.StretchBlt(rect.left, rect.top, rect.Width(), rect.Height(), &memDC, 0, 0, cx,cy, SRCCOPY);

	memDC.DeleteDC();
}

void CSkinBtn::DrawFilledRect(CDC *DC, CRect R, COLORREF color)
{ 
	CBrush B;
	B.CreateSolidBrush(color);
	DC->FillRect(R, &B);
}

void CSkinBtn::DrawButtonText(CDC *DC, CRect R, CString str, COLORREF TextColor)
{
    COLORREF prevColor = DC->SetTextColor(TextColor);
	DC->SetBkMode(TRANSPARENT);    
	//DC->SelectObject(m_pFont);
	if(1)//m_hMouseOutIcon.IsNull()&&m_hMouseInIcon.IsNull())
	{
		//int iconwidth=::GetSystemMetrics(SM_CXICON);
		R.right=R.right-m_textPos.x;
		DC->DrawText( str, str.GetLength(), R, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        //CBrush brBtnShadow(GetSysColor(COLOR_BTNSHADOW));
        //DC->FrameRect(R,&brBtnShadow);
	}
	else
	{
		DC->DrawText( str, str.GetLength(), R, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
	}
	DC->SetTextColor(prevColor);
}

void CSkinBtn::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//if (nFlags & MK_LBUTTON && m_bMouseOver == FALSE) 
	//	return;
	if(!m_bMouseOver&&m_bEnabled)
	{
		m_bMouseOver=true;
		m_state = HOVER;

		CPoint	point;
		CRect	rect;
		GetWindowRect(&rect);	
		GetCursorPos(&point);
		if (!rect.PtInRect(point) && m_bMouseOver&&m_bEnabled)
		{
			SetTimer(1,10,NULL);
			return;
		}

		DrawButton();
		
		SetTimer(1,10,NULL);
	}
	CButton::OnMouseMove(nFlags, point);
}

void CSkinBtn::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CPoint	point;
	CRect	rect;
	GetWindowRect(&rect);	
	GetCursorPos(&point);

	if (!rect.PtInRect(point) && m_bMouseOver&&m_bEnabled)
	{
		KillTimer (1);
		m_bMouseOver=false;
		m_state = NORMAL;
		DrawButton();
	}

	CButton::OnTimer(nIDEvent);
}

void CSkinBtn::PreSubclassWindow()
{
	// TODO: 在此添加专用代码和/或调用基类
	SetOwnerDraw(true);

	CButton::PreSubclassWindow();
}

void CSkinBtn::SetOwnerDraw(bool IsDraw)
{
	if(IsDraw)
	{
		ModifyStyle(NULL, BS_OWNERDRAW); 
		Invalidate();
	}
	else
	{
		ModifyStyle(BS_OWNERDRAW, NULL); 
		Invalidate();
	}
}

void CSkinBtn::SetImage(CString strNormal, CString strHover, CString strDown, CString strDisable)
{
	m_imgNormal.Load(strNormal);
	m_imgHover.Load(strHover);
	m_imgDown.Load(strDown);
	m_imgDisable.Load(strDisable);
}

void CSkinBtn::SetImage(UINT nNormalID, UINT nHoverID, UINT nDownID, UINT nDisableID)
{
	m_imgNormal.LoadFromResource(AfxGetApp()->m_hInstance, nNormalID ); 
	m_imgHover.LoadFromResource(AfxGetApp()->m_hInstance, nHoverID ); 
	m_imgDown.LoadFromResource(AfxGetApp()->m_hInstance, nDownID ); 
	m_imgDisable.LoadFromResource(::GetModuleHandle(NULL), nDisableID ); 
}

void CSkinBtn::SetImage(UINT nImagesID, int x, int y)
{
    m_imgList.Detach();
    m_imgList.Create(x,y,ILC_MASK|ILC_COLOR32, 0, 4);

    CBitmap bmt;
    bmt.LoadBitmap(nImagesID);
    m_imgList.Add(& bmt,RGB(255,0,255));

}

BOOL CSkinBtn::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(!m_bDCStored)
	{
    	CRect  rect;
	    GetClientRect(&rect);

		m_memDC.CreateCompatibleDC (pDC);
		CBitmap	btScreen;
		btScreen.CreateCompatibleBitmap (pDC,rect.Width(),rect.Height());
		m_memDC.SelectObject (&btScreen);

		m_memDC.BitBlt (0,0,rect.Width(),rect.Height(),pDC,0,0,SRCCOPY);

		m_bDCStored=true;

		CRgn rgn;
		rgn.CreateRectRgn (0, 0, rect.Width(), rect.Height());
		SetWindowRgn (rgn, TRUE);

		btScreen.DeleteObject();
	}
	return TRUE;// CButton::OnEraseBkgnd(pDC);//
}

BOOL CSkinBtn::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	return CButton::OnSetCursor(pWnd, nHitTest, message);
	// If a cursor was specified then use it!
	if (m_hCursor != NULL)
	{
		::SetCursor(m_hCursor);
		return TRUE;
	} // if

	return CButton::OnSetCursor(pWnd, nHitTest, message);
} // End of OnSetCursor


DWORD CSkinBtn::SetBtnCursor(LPTSTR nCursorId)
{
	CString str;
	::GetWindowsDirectory(str.GetBuffer(MAX_PATH), MAX_PATH);
	str.ReleaseBuffer();
	str += _T("\\winhlp32.exe");
	
	HMODULE hModule = ::LoadLibrary(str);
	if (hModule) 
	{
		HCURSOR hHandCursor = ::LoadCursor(hModule, MAKEINTRESOURCE(106));
		if (m_hCursor)
			m_hCursor =CopyCursor(hHandCursor);
	}
	FreeLibrary(hModule);


	return 0;
} // End of SetBtnCursor


DWORD CSkinBtn::SetBtnCursor(int nCursorId, BOOL bRepaint)
{
	HINSTANCE	hInstResource = NULL;
	// Destroy any previous cursor
	if (m_hCursor)
	{
		::DestroyCursor(m_hCursor);
		m_hCursor = NULL;
	} // if

	// Load cursor
	if (nCursorId)
	{
		hInstResource = AfxFindResourceHandle(MAKEINTRESOURCE(nCursorId), RT_GROUP_CURSOR);
		// Load cursor resource
		m_hCursor = (HCURSOR)::LoadImage(hInstResource, MAKEINTRESOURCE(nCursorId), IMAGE_CURSOR, 0, 0, 0);
		// Repaint the button
		if (bRepaint) Invalidate();
		// If something wrong
		if (m_hCursor == NULL) return -1;
	} // if

	return 0;
} // End of SetBtnCursor

void CSkinBtn::SetIcon(CString  strMouseOut,CString strMouseIn)
{
	m_hMouseOutIcon.Load(strMouseOut);
	m_hMouseInIcon.Load(strMouseIn);
}

void CSkinBtn::SetColor(COLORREF fgcolor,COLORREF bgcolor)
{
	m_fg = fgcolor;
	m_bg = bgcolor;
	DrawButton();
}

void CSkinBtn::SetTextPos(CPoint point)
{
	m_textPos = point;
	DrawButton();
}
CRect CSkinBtn::GetRectInParent()
{
	CRect rcWindowParent;
	GetParent()->GetWindowRect(rcWindowParent);//client
	CRect rcWindow;
	GetWindowRect(&rcWindow);
	CRect rect;
	rect.left = rcWindow.left-rcWindowParent.left;
	rect.top = rcWindow.top-rcWindowParent.top;
	rect.right = rcWindow.right-rcWindowParent.left;
	rect.bottom = rcWindow.bottom-rcWindowParent.top;

	return rect;
}
void CSkinBtn::OnEnable(BOOL bEnable)
{
	CButton::OnEnable(bEnable);

	// TODO: 在此处添加消息处理程序代码
	if(bEnable)
		m_bEnabled = true;
	else
		m_bEnabled = false;
}

void CSkinBtn::OnDestroy()
{
	CButton::OnDestroy();

	m_imgNormal.Destroy();
	m_imgHover.Destroy();
	m_imgDown.Destroy();
	m_imgDisable.Destroy();
    m_imgList.Detach();

	// TODO: 在此处添加消息处理程序代码
}

BOOL CSkinBtn::PreTranslateMessage(MSG* pMsg)
{
    // TODO: Add your specialized code here and/or call the base class
	InitToolTip();
	m_ToolTip.RelayEvent(pMsg);

    return CButton::PreTranslateMessage(pMsg);
}

void CSkinBtn::InitToolTip()
{
	if (m_ToolTip.m_hWnd == NULL)
	{
		// Create ToolTip control
		m_ToolTip.Create(this);
		// Create inactive
		m_ToolTip.Activate(FALSE);
		// Enable multiline
		m_ToolTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, 400);
	} // if
} // End of InitToolTip

void CSkinBtn::SetTooltipText(int nText, BOOL bActivate)
{
	CString sText;

	// Load string resource
	sText.LoadString(nText);
	// If string resource is not empty
	if (sText.IsEmpty() == FALSE) SetTooltipText((LPCTSTR)sText, bActivate);
} // End of SetTooltipText

// This function sets the text to show in the button tooltip.
//
// Parameters:
//		[IN]	lpszText
//				Pointer to a null-terminated string containing the text to show.
//		[IN]	bActivate
//				If TRUE the tooltip will be created active.
//
void CSkinBtn::SetTooltipText(LPCTSTR lpszText, BOOL bActivate)
{
	// We cannot accept NULL pointer
	if (lpszText == NULL) return;

	// Initialize ToolTip
	InitToolTip();

	// If there is no tooltip defined then add it
	if (m_ToolTip.GetToolCount() == 0)
	{
		CRect rectBtn; 
		GetClientRect(rectBtn);
		m_ToolTip.AddTool(this, lpszText, rectBtn, 1);
	} // if

	// Set text for tooltip
	m_ToolTip.UpdateTipText(lpszText, this, 1);
	m_ToolTip.Activate(bActivate);
} // End of SetTooltipText
// This function enables or disables the button tooltip.
//
// Parameters:
//		[IN]	bActivate
//				If TRUE the tooltip will be activated.
//
void CSkinBtn::ActivateTooltip(BOOL bActivate)
{
	// If there is no tooltip then do nothing
	if (m_ToolTip.GetToolCount() == 0) return;

	// Activate tooltip
	m_ToolTip.Activate(bActivate);
} // End of EnableTooltip


void CSkinBtn::SetRgnStyle(SYTLE nStyle)
{
	m_Style = nStyle;

	if( m_rgn.GetSafeHandle() != NULL )
	{
		m_rgn.DeleteObject();
	}

	//设置按钮的有效区域(实现异型按钮)
	CRect rc;
	GetClientRect(&rc);


	// 设置有效区域
	// ROUNDRECT,ELLIPSE,UPTRIANGLE,DOWNTRIANGLE
    
	if(m_Style==ROUNDRECT) // 圆角矩形
	{
		m_rgn.CreateRoundRectRgn(rc.left,rc.top,rc.right,rc.bottom,6,6);
	}
	else if(m_Style==ELLIPSE)// 椭圆
	{
		m_rgn.CreateEllipticRgn(rc.left,rc.top,rc.right,rc.bottom);	  
	}
	else if(m_Style==UPTRIANGLE)// 上三角
	{
		CPoint ptArray[] ={CPoint(rc.left+rc.Width()/2,rc.top),
			               CPoint(rc.left,rc.bottom),
                           CPoint(rc.right,rc.bottom)};

		VERIFY(m_rgn.CreatePolygonRgn(ptArray, 3, ALTERNATE));
        
	}
	else if(m_Style==UPTRIANGLE)// 下三角
	{
      CPoint ptArray[] ={CPoint(rc.left,rc.top),
			               CPoint(rc.right,rc.top),
                           CPoint(rc.left+rc.Width()/2,rc.bottom)};

	  VERIFY(m_rgn.CreatePolygonRgn(ptArray, 3, ALTERNATE));
	}

	int nRes = SetWindowRgn(m_rgn,TRUE);
    if (nRes==0)
    {
		MessageBox(_T("设置形状不成功呢"));
    }
	
}