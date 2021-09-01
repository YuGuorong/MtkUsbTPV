// CMainFrame.cpp: 实现文件
//

#include "pch.h"
//#include "MtkUsbTPV.h"
#include "CMainFrame.h"
#include "afxdialogex.h"

#include "CFtDevice.h"
#include <shellapi.h>
#include <initguid.h> 
#include <Dbt.h>

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "setupapi.lib")

// CMainFrame 对话框


//#define TMR_ID_SUB_DONE (1111)
#define TMR_ID_SCAN_USB (1113)
#define TMR_ID_SCAN_BURN 1114


IMPLEMENT_DYNAMIC(CMainFrame, CExDialog)

CMainFrame::CMainFrame(CWnd* pParent /*=nullptr*/)
	: CExDialog(IDD_MAIN_FRAME, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	MAX_PANNEL_NUM = 4;
	TCHAR val[64];
	//int reg =  ReadReg(L"USBTPV_SET", L"MAX_PANNEL_MAX", val);
	//if (reg == 0) {
	//	MAX_PANNEL_NUM = _ttoi(val);
	//	if (MAX_PANNEL_NUM < 4) MAX_PANNEL_NUM = 4;
	//}
	INT reg = ReadReg(L"USBTPV_SET", L"display_pannels", val);
	if (reg == 0) {
		m_dispPannelNum = _ttoi(val);
	}
	if (m_dispPannelNum <= 1 || m_dispPannelNum> MAX_PANNEL_NUM)
		m_dispPannelNum = 1;
	else if (m_dispPannelNum % 2 == 1)
		m_dispPannelNum = (m_dispPannelNum/2)*2;
	if (reg != 0) {
		CString str;
		str.Format(L"%d", m_dispPannelNum);
		TCHAR val[16];
		_tcscpy_s(val, str);
		WriteReg(L"USBTPV_SET", L"display_pannels", val);
	}
}

CMainFrame::~CMainFrame()
{
}

void CMainFrame::DoDataExchange(CDataExchange* pDX)
{
	CExDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ST_VERSION, m_strNewVersion);
	DDX_CBIndex(pDX, IDC_COMBO_ITEMS, m_nPannelMax);

	DDX_Control(pDX, IDC_COMBO_ITEMS, m_cbPannelsNum);
}


BEGIN_MESSAGE_MAP(CMainFrame, CExDialog)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_QUERYDRAGICON()
	ON_WM_PAINT()
	ON_MESSAGE(WM_DEVICECHANGE, OnMyDeviceChange)
	ON_CBN_SELCHANGE(IDC_COMBO_ITEMS, &CMainFrame::OnCbnSelchangeComboItems)
END_MESSAGE_MAP()


// CMainFrame 消息处理程序

BOOL CMainFrame::OnInitDialog()
{
	CExDialog::OnInitDialog();

	//MAX_PANNEL_NUM = 2;

	// TODO:  在此添加额外的初始化
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


	m_pannles_state = FALSE;
	m_mapDev.clear();
	m_scanUsbTimes = 0;


	m_strNewVersion.SetWindowText(_T(""));

	

	CRect r(0, 0, 64, 64);
	for (int i = 0; i < MAX_PANNEL_NUM; i++) {
		CStatic* frame = new CStatic();
		frame->Create(_T(""), WS_CHILD | WS_VISIBLE | SS_CENTER, r, this);	// 
		m_oFrames.Add(frame);
		CPannel* dlg;// = new CSubPanelDlg(this);
		CREATE_SUB_WND(dlg, CPannel, frame);

		LRESULT err;
		CFtBoard* pboard = CFtdiDriver::GetDriver()->FindBoard(-1, i, &err);
		if (pboard) {
			dlg->SetBoard(pboard);
		}
		
		//dlg->SetIndex(i);
		m_oPannels.Add(dlg);
	}

	if (!regist_device_htplug())
		return FALSE;

	CString strTitle;
	strTitle.LoadString(IDSTR_VENDOR_APPNAME);
	this->SetWindowText(strTitle);


	// TODO: 在此添加额外的初始化代码
	ShowWindow(SW_SHOWMAXIMIZED);

	m_cbPannelsNum.SetCurSel(m_dispPannelNum / 2);

	//CArray<SSerInfo, SSerInfo&> asi;
	//EnumSerialPorts(asi, TRUE);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CMainFrame::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CExDialog::OnPaint();
	}
}


HCURSOR CMainFrame::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CExDialog::OnSize(nType, cx, cy);
	HWND hwd = this->GetSafeHwnd();
	int max = m_dispPannelNum; //(int)m_oFrames.GetSize();
	if (max && ::IsWindow(hwd) && m_oFrames.GetCount() > 0 ) {
		int cols = max > 1 ? 2 : 1; 
		int rows = max / cols;
		int ox = 4;
		int oy = 48;
		cx -= ox * 2;
		cy -= oy + ox;
		cols = cols == 0 ? 1 : cols;

		CRect r(0, 0, cx / cols, cy / rows);
		if (cols) {
			r.OffsetRect(ox, oy);
			for (int i = 0; i < max; i++) {
				if (m_oFrames[i])
				{
					m_oFrames[i]->MoveWindow(r);
					if (m_oPannels.GetAt(i))
						m_oPannels.GetAt(i)->MoveWindow(0, 0, r.Width(), r.Height());
				}

				if (i % cols == (cols - 1)) {
					r.MoveToY(r.top + r.Height());
					r.MoveToX(ox);
				}
				else
					r.MoveToX(r.Width());
			}
		}
		//CRect r(0, 0, cx, cy);
		//r.top = r.bottom - 100;
		//m_oEdit.MoveWindow(r);
	}

}


BOOL CMainFrame::CheckSubPannelRunning(BOOL bmessagebox)
{
	BOOL bruning = FALSE;
	if (m_pannles_state) {
		for (int i = 0; i < m_oPannels.GetSize(); i++)
		{
			if (((CSubPanelDlg*)m_oPannels[i])->m_state & META_BUSY) {
				bruning = TRUE;
				break;
			}
		}
	}

	return bruning;
}


void CMainFrame::OnClose()
{
	if (CheckSubPannelRunning() == TRUE) {
		::MessageBox(NULL, _T("程序烧录中无法退出!"), _T("程序无法退出"), MB_OK);
		return;
	}

	while (m_oPannels.GetSize()) {
		if (m_oPannels[0]) {
			if (::IsWindow(m_oPannels[0]->GetSafeHwnd()))
				m_oPannels[0]->DestroyWindow();
			delete m_oPannels[0];
		}
		m_oPannels.RemoveAt(0);
	}

	while (m_oFrames.GetSize()) {
		if (m_oFrames[0]) {
			if (::IsWindow(m_oFrames[0]->GetSafeHwnd()))
				m_oFrames[0]->DestroyWindow();
			delete m_oFrames[0];
		}
		m_oFrames.RemoveAt(0);
	}
	CExDialog::OnClose();
}


void CMainFrame::OnDestroy()
{
	CExDialog::OnDestroy();
	// TODO: 在此处添加消息处理程序代码
}


void CMainFrame::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类

	CExDialog::OnCancel();
}



BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	return CExDialog::PreTranslateMessage(pMsg);
}



void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == TMR_ID_SCAN_USB)
	{
		KillTimer(nIDEvent);
		{
			CFtdiDriver::GetDriver()->Scan(TRUE);
		}
		return;
	}
	else if (nIDEvent == TMR_ID_SCAN_BURN)
	{
		KillTimer(nIDEvent);
		//if (m_pCurBurnRom)
		{
			//CRomDesc * prom = (CRomDesc*)m_pCurBurnRom;
			//if (prom->m_bAutoBurn)
			{
				BOOL bBurning = FALSE;
				for (int i = 0; i < MAX_DEV_SUPPORT; i++)
				{
					CSubPanelDlg* pdev = (CSubPanelDlg*)m_oPannels[i];
					if (pdev->m_state & META_BUSY)
						bBurning = TRUE;
				}
				if (bBurning == FALSE)
				{
					//ShowBurning(FALSE);
					//m_pCurBurnRom = NULL;
				}
				SetTimer(TMR_ID_SCAN_BURN, 1000, NULL);
			}
		}
	}
	else if (nIDEvent == TMR_ID_SUB_DONE)
	{
		//SetTimer(TMR_ID_SCAN_USB, 300, NULL);
	}
	CExDialog::OnTimer(nIDEvent);
}


LRESULT CMainFrame::OnMyDeviceChange(WPARAM wParam, LPARAM lParam)
{
	SetTimer(TMR_ID_SCAN_USB, 1000, NULL);
	return 1;
}

void CMainFrame::OnCbnSelchangeComboItems()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData();
	int index = m_cbPannelsNum.GetCurSel();
	CString str;
	m_cbPannelsNum.GetLBText(index, str);
	int numSel = _ttoi(str);
	if (m_dispPannelNum == numSel) return;
	m_dispPannelNum = numSel;
	for (int i = 0; i < MAX_PANNEL_NUM; i++) {
		if (i < m_dispPannelNum) {
			m_oFrames[i]->ShowWindow(SW_SHOW);
		}
		else {
			m_oFrames[i]->ShowWindow(SW_HIDE);
		}
	}
	CRect  r;
	this->GetWindowRect(&r);
	this->OnSize(0, r.Width(), r.Height());
	//this->movetocl
	//this->MoveWindow()
	this->Invalidate();
	TCHAR val[16];
	_tcscpy_s(val, str);
	WriteReg(L"USBTPV_SET", L"display_pannels", val);
	//_tcprintf_s(val, L"%d", MAX_PANNEL_NUM);
	//WriteReg(L"USBTPV_SET", L"MAX_PANNEL_MAX", val);

	//this->UpdateWindow();

}
