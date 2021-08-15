// ExDialog.cpp : implementation file
//

#include "pch.h"
#include "ExDialog.h"
#include "..\\resource.h"		// 主符号
#define WND_RAND_SIZE 0
CArray<CExDialog * , CExDialog *> g_popwnd;
// CExDialog dialog
BOOL SetTransparent(HWND hWnd, int iTransparent) 
{ 
	HINSTANCE hInst = LoadLibrary(_T("User32.DLL")); 
	if(hInst)
	{
		typedef BOOL (WINAPI *SLWA)(HWND,COLORREF,BYTE,DWORD);
		SLWA pFun = NULL;
		//取得SetLayeredWindowAttributes函数指针 
		pFun = (SLWA)GetProcAddress(hInst,"SetLayeredWindowAttributes");
		if(pFun)
		{
			pFun(hWnd,0,iTransparent,2);
		}
		FreeLibrary(hInst); 
		return TRUE;
	}
	return FALSE;
}


IMPLEMENT_DYNAMIC(CExDialog,CDialogEx)

BEGIN_MESSAGE_MAP(CExDialog, CDialogEx)
	ON_WM_ERASEBKGND()
    ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_WM_SIZE()
END_MESSAGE_MAP()

CExDialog::CExDialog(UINT nIDTemplate,CWnd* pParent /*=NULL*/, INT ex_style)
    : CDialogEx(nIDTemplate, pParent)
{
    m_pa_txts = NULL;
    m_pBkDC  = NULL;
	m_pTitDC = NULL;
    m_bTransprent = 0;
	m_pop_state = 0;
	m_ExStyle = ex_style;
	m_TitleHight = 30;
	m_WndDiameter = WND_RAND_SIZE;
	m_crtl = RGB(45,100,217), m_crbr = RGB(18,76,199);
}

CExDialog::~CExDialog() 
{
}

void CExDialog::SetFullScreen()
{

}

void CExDialog::OnDestroy()
{
    CDialogEx::OnDestroy();
    if( m_pa_txts )
    {
    	for(int i=0;i<m_nTxtCount;i++)
        {
            delete m_pa_txts[i];
        }
        delete m_pa_txts;
        m_pa_txts = NULL;
    }
    if(m_pBkDC)
    {
        m_pBkDC->DeleteDC();
        delete m_pBkDC;
        m_pBkDC = NULL;
    }
	if( m_backgrd.m_hObject ) m_backgrd.DeleteObject();
	if( m_pTitDC )
	{
		m_pTitDC->DeleteDC();
		delete m_pTitDC;
		m_pTitDC = NULL;
	}
    for(int i=0; i<m_aChkboxs.GetCount(); i++)
    {
        CLCheckBox * pbox = m_aChkboxs.GetAt(i);
        if( pbox ) delete pbox;
    }
    m_aChkboxs.RemoveAll();
	for (int i=0;i < m_atxts.GetCount();i++)
	{
	   LP_LINK  ptxt  = m_atxts.GetAt(i);
	   if( ptxt ) delete ptxt;
	}
	m_atxts.RemoveAll();
    // TODO: Add your message handler code here
}

int CExDialog::SubTextItems(const int * pIDs, const int * pstrIDs, const COLORREF * pColors, int nTotals)
{

	m_nTxtCount = nTotals;
	m_pa_txts = new LP_LINK[m_nTxtCount];

    CString strCaption;
	for(int i=0;i<m_nTxtCount;i++)
	{
		m_pa_txts[i] = new CLink;
		m_pa_txts[i]->SubclassDlgItem(pIDs[i],this);
		m_pa_txts[i]->SetTxtColor(pColors[i]);
        if( pstrIDs[i] )
        {
            strCaption.LoadString(pstrIDs[i]);
            m_pa_txts[i]->SetWindowText(strCaption);
        }
	}
    return 0;
}


BOOL CExDialog::OnEraseBkgnd(CDC* pDC)
{
    if( m_bTransprent== 0)
    {
	    CRect rc;
	    //GetUpdateRect(&rc);
	    GetWindowRect(&rc);
	    ScreenToClient (&rc);

	    //pDC->FillSolidRect(&rc,RGB(255,0,255));
	    pDC->BitBlt(rc.left,rc.top,rc.Width(),rc.Height(),m_pBkDC,rc.left,rc.top,SRCCOPY);
        //pDC->StretchBlt(0,0,rc.Width(),rc.Height(),m_pBkDC,0,0,sbtmp.bmWidth,sbtmp.bmHeight,SRCCOPY);		
    } 
	return TRUE;
};

void CExDialog::DrawTitleBar(CDC &dc)
{
	if( m_pTitDC == NULL ) return;
	if( EX_TITLE_BAR & m_ExStyle )
	{
		CRect rc;
		//GetUpdateRect(&rc);
		GetWindowRect(&rc);
		ScreenToClient (&rc);
		dc.BitBlt(0, 0, rc.Width(), rc.Height(), m_pTitDC, 0, 0, SRCCOPY);

		SetTextColor(dc, RGB(255,255,255));
		dc.SetBkColor(RGB(0,0,0));
		dc.SetBkMode(TRANSPARENT);
		CString str;
		this->GetWindowText(str);
		dc.TextOut(8,8, str);
	}
}

void CExDialog::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialogEx::OnPaint() for painting messages
	DrawTitleBar(dc);
}


BOOL CExDialog::PreTranslateMessage(MSG* pMsg)
{
    // TODO: Add your specialized code here and/or call the base class
	if(pMsg -> message == WM_KEYDOWN)
	{
        if(pMsg -> wParam == VK_ESCAPE)
		{
			if(this->GetSafeHwnd() == ::AfxGetMainWnd()->GetSafeHwnd() 
                ||m_bTransprent )
				return TRUE;
		}
	}	
    else if( pMsg->message == WM_COPYDATA)
    {
       
    }

    return CDialogEx::PreTranslateMessage(pMsg);
}


HBRUSH CExDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);


	if((nCtlColor == CTLCOLOR_EDIT) )
	{
		pDC->SetBkMode(TRANSPARENT); //设置背景透明，这样，输出字符的时候就
		//是所谓的空心字，而不是有白的底色
		//pDC->SetTextColor(RGB(255,0,0)); //改变字体的颜色
	
		//return HBRUSH(GetStockObject(HOLLOW_BRUSH));
	}
	// TODO:  Change any attributes of the DC here

	// TODO:  Return a different brush if the default is not desired
	return hbr;
}

BOOL CALLBACK Find_child(HWND hwnd, LPARAM parm)
{
    LONG id =  GetWindowLong(hwnd,GWL_ID);
	if( IDC_STATIC == id )
	{
		CLink * ptxt = new (CLink);
		ptxt->SubclassWindow(hwnd);
		CExDialog* pParent = (CExDialog*)parm;
		pParent->m_atxts.Add(ptxt);
	}

	return true;
}

void CExDialog::SetTitleBar()
{
	if( EX_TITLE_BAR & m_ExStyle )
	{
		if( m_pTitDC == NULL )
		{
			CRect r; 
			this->GetClientRect(&r);
			m_pTitDC = new CDC();
			m_pTitDC->CreateCompatibleDC(m_pBkDC);
			CBitmap btmp;
			btmp.CreateCompatibleBitmap(m_pBkDC, r.Width(), m_TitleHight); 
			m_pTitDC->SelectObject(btmp);
			m_pTitDC->BitBlt(0,0,r.Width(), m_TitleHight, m_pBkDC,0,0, SRCCOPY);

			CRect rx(r);
			rx.bottom = m_TitleHight, rx.top++, rx.left++,rx.right-=2;
			CUtil::DrawGradientFill(m_pTitDC, rx, RGB(0X0,0X30,0X30), RGB(0xd0,0XC0,0XC0));
			//m_pTitDC->FillRect(rx, &CBrush(RGB(0,120,120)));

			BLENDFUNCTION bf;
			bf.BlendOp=AC_SRC_OVER;
			bf.BlendFlags=0;
			bf.SourceConstantAlpha=0x3f;
			bf.AlphaFormat=0;
			m_pTitDC->AlphaBlend(0,0, r.Width(), rx.bottom, m_pBkDC,0, 0, r.Width(), rx.bottom, bf);
		}
	}
}

void CExDialog::CreatBkBmp()
{
	CRect r; 
	this->GetClientRect(&r);
	CClientDC dc(this); 
	if( m_pBkDC == NULL )
	{
		m_pBkDC=new CDC();//Create Backgroud memory DC, new bitmap, instead m_backgrd
		m_pBkDC->CreateCompatibleDC( &dc);
		CBitmap btmp;
		btmp.CreateCompatibleBitmap(&dc, r.Width(), r.Height());       
		m_pBkDC->SelectObject(&btmp);
	}

	CDC tdc;//tmp dc, for stretchblt 
	tdc.CreateCompatibleDC(&dc);
	if( m_backgrd.m_hObject != NULL) m_backgrd.DeleteObject();
	if( !(m_ExStyle & EX_FILL_BK))
	{
		m_backgrd.LoadBitmap(IDB_CANVAS_FLAT);
		tdc.SelectObject(&m_backgrd);
	}
	else
	{
		m_backgrd.CreateCompatibleBitmap(&dc,r.Width(), r.Height());
		tdc.SelectObject(&m_backgrd);		
		CUtil::DrawGradientFill(&tdc, r, m_crtl, m_crbr);
	}

	if( m_WndDiameter )
	{
		r.bottom--, r.right--;
		POINT pt;
		pt.x = pt.y = m_WndDiameter;
		m_pBkDC->RoundRect(r,pt);
		r.bottom++, r.right++;

		CRgn   rgn, rgn2;   
		rgn.CreateRoundRectRgn(0,   0,   r.Width(),   r.Height(),   (m_WndDiameter -1),   (m_WndDiameter -1));   
		SetWindowRgn(rgn,   TRUE);   
	}
	else
	{
		m_pBkDC->FillRect(r,&CBrush(RGB(255,255,255)));
	}

	if( m_ExStyle & EX_STRETCH_BK )
	{
		CSize sz = CUtil::GetBitmapSize(m_backgrd);
		m_pBkDC->StretchBlt(0,0,r.Width(),r.Height(),&tdc,0,0,sz.cx,sz.cy,SRCAND);
	}
	else
		m_pBkDC->BitBlt(0,0,r.Width(),r.Height(),&tdc,0,0,SRCAND);
}

void CExDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	if( m_pBkDC )
	{
		m_pBkDC->DeleteDC();
		delete m_pBkDC;
		m_pBkDC = NULL;
		CreatBkBmp();
	}
	// TODO: Add your message handler code here
}


void CExDialog::GetTitleBarHight(void)
{
    CBitmap bmclose;
    bmclose.LoadBitmap(IDB_BITMAP_CLOSE);
	CSize sz = CUtil::GetBitmapSize(bmclose);
	m_TitleHight = sz.cy + 8;
}

BOOL CExDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    // TODO:  Add extra initialization here
	EnumChildWindows(this->GetSafeHwnd(), Find_child, (LPARAM)this);
	GetTitleBarHight(); 
	CreatBkBmp();
	SetTitleBar();
    
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CExDialog::Create(UINT nIDTemplate, CWnd* pParentWnd)
{
	// TODO: Add your specialized code here and/or call the base class
	BOOL ret = CDialogEx::Create(nIDTemplate, pParentWnd);
	return ret;
}

// Set round corner window style, ndiamt diameter.
int CExDialog::SetWndStyle(int ndiamt, COLORREF crtl, COLORREF crbr)
{
	int or_diameter = m_WndDiameter;
	m_WndDiameter = ndiamt;
	m_crtl = crtl, m_crbr = crbr;
	return or_diameter;
}


IMPLEMENT_DYNAMIC(CeExDialog,CExDialog)
BEGIN_MESSAGE_MAP(CeExDialog,CExDialog)
    ON_WM_DESTROY()
    ON_WM_LBUTTONDOWN() 
    ON_WM_ERASEBKGND()
    ON_COMMAND(IDABORT, &CeExDialog::OnHideWindow)   
    ON_COMMAND(IDIGNORE, &CeExDialog::OnCloseWindow)   
	ON_MESSAGE(WM_SHOWTASK,&CeExDialog::OnTrayClickMsg)		//自定义托盘事件
	ON_WM_PAINT()
END_MESSAGE_MAP()

CeExDialog::CeExDialog(UINT nIDTemplate,CWnd* pParent /*=NULL*/, INT ex_style)
    : CExDialog(nIDTemplate, pParent, ex_style)
{
    m_pa_txts   = NULL;
    m_pBkDC     = NULL;
    m_num_cap   = 0;
    m_pCaptions = NULL;	
	m_pPopMenu = NULL;    
}

CeExDialog::~CeExDialog() 
{
    RemoveListItems();
}

void CeExDialog::RemoveListItems()
{
    if( m_pCaptions )
    {
        for(int i=0; i<m_num_cap; i++)
        {
            delete m_pCaptions[i];
        }
        delete m_pCaptions;
        m_pCaptions  = NULL;
    }
}

void CeExDialog::OnHideWindow()
{
    //::AfxGetMainWnd()->PostMessage(WM_COMMAND,ID_POPWINDOW ,0);
} 


static BOOL g_CloseToHideWnd = FALSE;

void CeExDialog::SetCloseToHide(BOOL bCloseToHide)
{
    g_CloseToHideWnd = bCloseToHide;
}

int CeExDialog::ExEndDialog(INT code)
{
	EndDialog(code);
	return 1;
}

void CeExDialog::OnCloseWindow()
{
    if(g_CloseToHideWnd)
    {
        OnHideWindow();
    }
    else
    {
		ExEndDialog(IDCANCEL);        
    }
}

void CeExDialog::OnDestroy()
{

    RemoveListItems();
    m_font.DeleteObject();
    
	
	for(int i=0; i<g_popwnd.GetCount(); i++)
    {
		if( g_popwnd[i] == this)
        {
			g_popwnd.RemoveAt(i);
			break;
        }
    }

	if( m_pPopMenu != NULL)
	{
		m_pPopMenu->DestroyMenu();
		delete m_pPopMenu;
		m_pPopMenu = NULL;
	}
    CExDialog::OnDestroy();
    // TODO: Add your message handler code here
}



void AddImageRes(CImageList *img, int id)
{
    CBitmap bmt;
    bmt.LoadBitmap(id);
    img->Add(&bmt,RGB(255,0,255));
}


BOOL CeExDialog::OnInitDialog()
{	   
	CExDialog::OnInitDialog();

    // TODO:  Add extra initialization here
    if( m_bTransprent == 0 )
    {
	    CRect rc;
	    GetClientRect(rc);   

        CRect rbt;
        CBitmap bmclose;
        bmclose.LoadBitmap(IDB_BITMAP_CLOSE);
        BITMAP sbtmp;
        bmclose.GetBitmap(&sbtmp);	

        CRect rect(0,0,sbtmp.bmWidth/3,sbtmp.bmHeight);
        rect.OffsetRect(rc.right-rect.Width()-4-4,6);	
        m_closeBtn.Create( _T(""), WS_CHILD | WS_VISIBLE , rect, this, IDIGNORE );  
		DWORD style = GetClassLong(m_closeBtn.GetSafeHwnd(), GCL_STYLE);
		SetClassLong(m_closeBtn.GetSafeHwnd(), GCL_STYLE, style & ~CS_PARENTDC);   
        m_closeBtn.SetImage(IDB_BITMAP_CLOSE,24,24);
		m_closeBtn.SetRgnStyle(CSkinBtn::ROUNDRECT);

		if( !(EX_CLOSE_BTN & m_ExStyle ) )
		{
			m_closeBtn.ShowWindow(SW_HIDE);
		}
   /*     rect.right = rect.left;
        rect.left = rect.right - sbtmp.bmWidth;
        m_minBtn.Create( _T(""), WS_CHILD | WS_VISIBLE , rect, this, IDABORT );  
        style = GetClassLong(m_closeBtn.GetSafeHwnd(), GCL_STYLE);
		SetClassLong(m_closeBtn.GetSafeHwnd(), GCL_STYLE, style & ~CS_PARENTDC); 
		m_minBtn.SetBitmapId(       IDB_BITMAP_MIN,IDB_BITMAP_MIN,IDB_BITMAP_MIN,IDB_BITMAP_MIN);
		m_minBtn.SetRgnStyle(CControlButton::ROUNDRECT);
		*/
    }

	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = 12;
	lf.lfWeight = FW_NORMAL;
	lf.lfCharSet =GB2312_CHARSET;
	_tcscpy_s(lf.lfFaceName , LF_FACESIZE,_T("宋体"));	//华文彩云 ,宋体,华文行楷,黑体
	m_font.CreateFontIndirect(&lf);	

	this->m_pop_state = 1;
	g_popwnd.Add(this);
    return TRUE;  // return TRUE unless you set the focus to a control
}

void CeExDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	if( point.y < m_TitleHight && m_bTransprent == 0)// ||  (point.y < 50 && point.x < 100) )
	{
		SendMessage(WM_NCLBUTTONDOWN ,HTCAPTION,0);
	}

    CExDialog::OnLButtonDown(nFlags, point);
}

BOOL CeExDialog::OnEraseBkgnd(CDC* pDC)
{
    // TODO: Add your message handler code here and/or call default
    if( m_bTransprent )
        return  CExDialog::OnEraseBkgnd(pDC);

    BOOL ret= CExDialog::OnEraseBkgnd(pDC);
    return ret;
}

void CeExDialog::HtmlHelp(DWORD_PTR dwData, UINT nCmd)
{
    // TODO: Add your specialized code here and/or call the base class
    //::ExMessageBox(_T("help"));
    //CExDialog::HtmlHelp(dwData, nCmd);
}

void CeExDialog::WinHelp(DWORD dwData, UINT nCmd)
{
    // TODO: Add your specialized code here and/or call the base class
    CString   appPath;   
    GetModuleFileName(NULL,   appPath.GetBuffer(MAX_PATH),   MAX_PATH);   
    //注：使用该API函数得到的是程序文件完整路径文件名，去掉文件名后才是路径。   
    appPath.ReleaseBuffer();   
    int   n   =   appPath.ReverseFind(_T('\\'));   
    CString   helpFile;   
    helpFile   =   appPath.Left(n);   
    TCHAR   c   =   helpFile.GetAt(n   -   1);   
    if(c   ==   _T('\\'))
        helpFile   +=   _T("Help\\Help.chm");   
    else   
        helpFile   +=   _T("\\Help\\Help.chm");   
    //通过   HtmlHelp   调用帮助文件(.chm)的程序代码如下：   
    ShellExecute(NULL,_T("open"),helpFile,NULL,NULL,SW_RESTORE); 
    
    //CExDialog::WinHelp(dwData, nCmd);
}


void CeExDialog::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CExDialog::OnPaint() for painting messages
	DrawTitleBar(dc);

}


// 托盘弹出菜单
void TrayPopupMenu(CMenu *psub, CWnd * pwnd)
{
	CPoint pt;
	GetCursorPos(&pt);
	DWORD dwID =psub->TrackPopupMenu((TPM_LEFTALIGN|TPM_RIGHTBUTTON),
		 pt.x, pt.y, pwnd); 
}


// 最小化到托盘
int CeExDialog::ToTray(int menuid, int subid)
{
	NOTIFYICONDATA nid; 
	nid.cbSize=(DWORD)sizeof(NOTIFYICONDATA); 
	nid.hWnd=this->GetSafeHwnd(); 
	nid.uID=IDR_MAINFRAME; 
	nid.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP ; 
	nid.uCallbackMessage=WM_SHOWTASK;//自定义的消息名称 
    nid.hIcon= (HICON)::LoadImage(AfxGetInstanceHandle(), \
        MAKEINTRESOURCE(IDR_MAINFRAME),IMAGE_ICON,16,16,LR_DEFAULTCOLOR);
    ::LoadString( AfxGetApp()->m_hInstance,IDSTR_VENDOR_APPNAME,nid.szTip,30);
	Shell_NotifyIcon(NIM_ADD,&nid);    //在托盘区添加图标 
	if( m_pPopMenu == NULL )
	{
		m_pPopMenu = new COfficeXPMenu();
		m_pPopMenu->LoadMenu( menuid );	
		m_pPopMenu->SetType(TYPE_XP);
		m_pop_subid = subid;
	}
	return 0;
}

// 删除托盘图标函数
void CeExDialog::DeleteTray()
{
	NOTIFYICONDATA nid;
    
	nid.cbSize=(DWORD)sizeof(NOTIFYICONDATA);
	nid.hWnd=this->GetSafeHwnd();
	nid.uID=IDR_MAINFRAME;
	nid.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP;
	nid.uCallbackMessage=WM_SHOWTASK;				 
	nid.hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));
    ::LoadString( AfxGetApp()->m_hInstance,IDSTR_VENDOR_APPNAME,nid.szTip,30);
	Shell_NotifyIcon(NIM_DELETE,&nid);				//在托盘区删除图标
}
//



LRESULT CeExDialog::OnTrayClickMsg(WPARAM wParam, LPARAM lParam)
{
	//恢复界面函数
	if(wParam!=IDR_MAINFRAME)
		return 1;
	switch(lParam)
	{
	case WM_RBUTTONDOWN:			//单击右键的处理
		{
			if( m_pPopMenu )
				TrayPopupMenu((CMenu *)m_pPopMenu->GetSubMenu(m_pop_subid),this);		//弹出菜单
			break;
		}
//	case WM_LBUTTONDOWN:			//单击左键的处理
	case WM_LBUTTONDBLCLK:
		{
            //m_bHiden = ~m_bHiden;
            //OnPopwindow();
			break;
		} 
	default:
		break;
	} 
	return 0; 
}

