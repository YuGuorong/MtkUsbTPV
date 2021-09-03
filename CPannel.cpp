// CPannel.cpp: 实现文件
//

#include "pch.h"
//#include "MtkUsbTPV.h"
#include "resource.h"
#include "CPannel.h"
#include "CTpvBoard.h"
#include "afxdialogex.h"


// CPannel 对话框

IMPLEMENT_DYNAMIC(CPannel, CExDialog)

CPannel::CPannel(CWnd* pParent /*=nullptr*/)
	: CExDialog(IDD_PANNEL, pParent)
{
	m_pBoard = NULL;
}

CPannel::~CPannel()
{
}

void CPannel::DoDataExchange(CDataExchange* pDX)
{
	CExDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DEV, m_cbDevice);
}


BEGIN_MESSAGE_MAP(CPannel, CExDialog)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_CLRALL, &CPannel::OnBnClickedBtnClrall)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BTN_POWER_1, IDC_BTN_HOMEKEY_8, &CPannel::OnBnClickedGpioCtl)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BTN_GPIO_1, IDC_BTN_GPIO_8, &CPannel::OnBnClickedGpioCtl)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BTN_CON0, IDC_BTN_CON7, &CPannel::OnBnClickedConset)
	ON_BN_CLICKED(IDC_BTN_SETALL, &CPannel::OnBnClickedBtnSetall)
	ON_BN_CLICKED(IDC_BTN_PWRKEY, &CPannel::OnBnClickedBtnPwrkey)
	ON_BN_CLICKED(IDC_BTN_KCOL0, &CPannel::OnBnClickedBtnKcol0)
	ON_BN_CLICKED(IDC_BTN_HOME, &CPannel::OnBnClickedBtnHome)
	ON_BN_CLICKED(IDC_BTN_GPIO, &CPannel::OnBnClickedBtnGpio)
	ON_CBN_SELCHANGE(IDC_COMBO_DEV, &CPannel::OnCbnSelchangeComboDev)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CPannel 消息处理程序

void CPannel::SetBoard(CTpvBoard* board)
{
	m_pBoard = board;
	if (m_pBoard != NULL) {
		UpdateData();
		m_cbDevice.Clear();
		m_cbDevice.ResetContent();
		CFtdiDriver* drv = CFtdiDriver::GetDriver();
		map<int, CTpvBoard*>::iterator it;
		int sel = -1;
		for (it = drv->m_BoardList.begin(); it != drv->m_BoardList.end(); it++) {
			CTpvBoard* pboard = it->second;
			if (pboard->m_bMounted) {
				CString str;
				str.Format(L"%d-(COM %s)", pboard->m_index, pboard ->m_strPortInfo);//  > m_BoardID);
				m_cbDevice.AddString(str);
				if (board == it->second) sel = m_cbDevice.GetCount();
			}
		}
		m_cbDevice.SetCurSel(sel - 1);
		UpdateIoState();	
		m_pBoard->Close();
	}
}

BOOL CPannel::OnInitDialog()
{
	CExDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	
	//m_AllBtns[0] = new CGdipButton;
	//m_AllBtns[0]->SubclassDlgItem(IDC_BTN_CLRALL, this);
	//m_AllBtns[0]->LoadStdImage(IDB_PNG_SWON, _T("PNG"));
	//m_AllBtns[0]->LoadAltImage(IDB_PNG_SWOFF, _T("PNG"));
	UINT btnids[][2] = {
		{IDC_BTN_GPIO_1,	IDB_PNG_SWON },
		{IDC_BTN_POWER_1,	IDB_PNG_SWON },
		{IDC_BTN_KCOL0_1,	IDB_PNG_SWON },
		{IDC_BTN_HOMEKEY_1, IDB_PNG_SWON },

	};
	for (int i = 0; i < CON_MAX; i++)
	{
		for (int j = 0; j < ROW_MAX; j++) {
			m_cGpioBtns[i][j] = new CGdipButton;
			m_cGpioBtns[i][j]->SubclassDlgItem(btnids[j][0]+i*(j==0?1:3), this);
			m_cGpioBtns[i][j]->LoadStdImage(IDB_PNG_SWON_L, _T("PNG"));
			m_cGpioBtns[i][j]->LoadAltImage(IDB_PNG_SWOFF_L, _T("PNG"));
		}
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CPannel::UpdateIoState()
{
	if (m_pBoard) {
		int items = 0;
		IO_VAL ioval[16];
		m_pBoard->Display(-1, ioval, &items);
		for (int i = 0; i < items; i++) {
			if(ioval[i].con == IO_CON_GPIO) {
				int id = IDC_BTN_GPIO_1;
				BYTE v = ioval[i].v.gpio.val;
				for (int i = 0; i < 8; i++) {
					CGdipButton* pctl = (CGdipButton*)this->GetDlgItem(id+i);
					pctl->SetImage((v & (1 << i)) ? CGdipButton::STD_TYPE : CGdipButton::ALT_TYPE);
				}
			}
			else {
				int id = IDC_BTN_POWER_1 + i*IO_MAX_KEY;
				char* pv = ioval[i].v.pin;
				for (int i = 0; i < IO_MAX_KEY; i++) {
					CGdipButton* pctl = (CGdipButton*)this->GetDlgItem(id + i);
					pctl->SetImage(pv[i] ? CGdipButton::STD_TYPE : CGdipButton::ALT_TYPE);
				}
			}
		}
	}
}


void CPannel::OnSize(UINT nType, int cx, int cy)
{
	CExDialog::OnSize(nType, cx, cy);
	
	if (cy > 500) cy = 500;
	HWND hwd = this->GetSafeHwnd();
	if (!IsWindow(hwd)) return;
	CWnd* pwd = GetDlgItem(IDC_BTN_POWER_1);
	if (!IsWindow(pwd->GetSafeHwnd())) return;
	CRect r;
	CRect r_sw;
	pwd->GetWindowRect(r_sw);
	this->ScreenToClient(&r_sw);

	CWnd* tit_gpio = GetDlgItem(IDC_STATIC_G1);
	CRect rtgpio;
	tit_gpio->GetClientRect(rtgpio);
	this->ScreenToClient(&rtgpio);


	int marg_gpio = 50 + rtgpio.Height();
	CWnd* ctrl = GetDlgItem(IDC_BTN_PWRKEY);
	CRect r_keyl;
	ctrl->GetWindowRect(r_keyl);
	this->ScreenToClient(&r_keyl);

	//wc :width of cons
	//hc :high of cons
	//sw :switch buttons
	//key:header ctrl key buttons(left butons)
	int wcons = cx - r_sw.left - 10;
	int wcon = wcons / 8;
	int wc_con = wcon - wcon/8 - 4;
	TRACE("====> wcon %d\n", wcon);
	for (int i = 0; i < 8; i++)
	{
		ctrl = GetDlgItem(IDC_BTN_CON0 + i);
		ctrl->GetWindowRect(r);
		this->ScreenToClient(r);
		r.right = r.left + wc_con;
		r.MoveToX(r_sw.left + wcon * i);
		ctrl->MoveWindow(r);
	}

	int hkeys = cy - r_sw.top - marg_gpio - 30;
	int hkey = hkeys / 4;
	int wc_sw = wc_con;
	int hc_key = hkey - 16;
	int hc_sw = hc_key;
	if (hc_sw > wc_sw / 2)
		hc_sw = wc_sw / 2;
	else
		wc_sw = hc_sw * 2;

	//resize left ctrl buttons
	int kyeids[] = { IDC_BTN_PWRKEY, IDC_BTN_KCOL0, IDC_BTN_HOME, IDC_BTN_GPIO };
	for (int i = 0; i < 4; i++) {
		ctrl = GetDlgItem(kyeids[ i]);
		ctrl->GetWindowRect(r);
		this->ScreenToClient(r);
		r.bottom = r.top+ hc_sw;
		r.MoveToY(r_sw.top + hkey * i);
		ctrl->MoveWindow(r);
	}
	r.MoveToY(r_sw.top + hkey * 3 + marg_gpio);
	ctrl->MoveWindow(r);


	//resize switch-buttons
	int dert = 0;// hc_sw / 5;
	for (int j = 0; j < 8; j++) {
		for (int i = 0; i < 3; i++) {
			ctrl = GetDlgItem(IDC_BTN_POWER_1 + j*3 +i );
			ctrl->GetWindowRect(r);
			this->ScreenToClient(r);
			
			r.right = r.left + wc_sw;
			r.bottom = r.top + hc_sw -dert;
			r.MoveToXY(r_sw.left + wcon * j, r_sw.top + hkey * i+dert/2);
			ctrl->MoveWindow(r);
		}

		//resize boarders
		if (j < 7) {
			ctrl = GetDlgItem(IDC_STATIC1 + j);
			ctrl->GetWindowRect(r);
			this->ScreenToClient(r);
			r.bottom = cy - 10;
			r.MoveToX(r_sw.left + wcon * (j+1) - 10);
			ctrl->MoveWindow(r);
		}
		
	}
	
	//resize gpio header buttons
	int h = rtgpio.Height();
	for (int i = 0; i < 8; i++) {
		ctrl = GetDlgItem(IDC_BTN_GPIO_1 + i);
		ctrl->GetWindowRect(r);
		this->ScreenToClient(r);

		r.right = r.left + wc_sw;
		r.bottom = r.top + hc_sw;
		r.MoveToXY(r_sw.left + wcon * i, r_sw.top + hkey * 3 + marg_gpio);
		ctrl->MoveWindow(r);

		ctrl = GetDlgItem(IDC_STATIC_G1 + i);
		r.bottom = r.top + h;
		r.MoveToXY(r_sw.left + wcon * i, r_sw.top + hkey * 3 + (marg_gpio - h)/2);
		ctrl->MoveWindow(r);
	}
	
	//resize gpio_ctrl button
	ctrl = GetDlgItem(IDC_STATIC_GPIO);
	ctrl->GetWindowRect(r);
	this->ScreenToClient(r);
	r.right = r.left + cx - r.left - 10;
	r.MoveToY(r_sw.top + hkey * 3 + marg_gpio - 10);
	ctrl->MoveWindow(r);
	
	//resize left boarder
	ctrl = GetDlgItem(IDC_STATIC0);
	ctrl->GetWindowRect(r);
	this->ScreenToClient(r);
	r.bottom = cy - 10;
	ctrl->MoveWindow(r);





	// TODO: 在此处添加消息处理程序代码
}


void CPannel::OnDestroy()
{
	CExDialog::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	for (int i = 0; i < CON_MAX; i++)
	{
		for (int j = 0; j < ROW_MAX; j++) {
			delete m_cGpioBtns[i][j];
		}
	}

}


void CPannel::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CExDialog::OnClose();
}


void CPannel::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CExDialog::OnTimer(nIDEvent);
}



void CPannel::OnBnClickedBtnClrall()
{
	SetAll(0);

}

void CPannel::OnBnClickedGpioCtl(UINT id)
{
	if (m_pBoard == NULL) return;
	if ((id >= IDC_BTN_POWER_1 && id <= IDC_BTN_HOMEKEY_8) ||
		(id >= IDC_BTN_GPIO_1 && id <= IDC_BTN_GPIO_8))
	{
		CGdipButton* pctl = (CGdipButton*)this->GetDlgItem(id);
		if (pctl == NULL) return;



		if (m_pBoard == NULL || m_pBoard->isMount() == FALSE ) return;
		IO_OP op = def_request;
		int con = IO_CON0;
		if (id >= IDC_BTN_POWER_1 && id <= IDC_BTN_HOMEKEY_8) {
			con = (id - IDC_BTN_POWER_1) / IO_MAX_KEY;
			op.val.con = con;
			int key = (id - IDC_BTN_POWER_1) - (con * IO_MAX_KEY);
			//printf("con %d, key:%d\n", con, key);
			op.val.v.pin[key] = (pctl->GetImage() == CGdipButton::STD_TYPE) ? 0 : 1;
		}
		else {
			int bit = id - IDC_BTN_GPIO_1;
			IO_VAL* pv =(IO_VAL*)&op.val;
			con = IO_CON_GPIO;
			pv->con = con;
			pv->fmt = GPIO_CTL;
			pv->v.gpio.bit = bit;
			pv->v.gpio.val = (pctl->GetImage() == CGdipButton::STD_TYPE) ? 0 : 1;
		}
		if (m_pBoard->SyncIO(con, &op) == FT_OK) {

			if (pctl->GetImage() == CGdipButton::ALT_TYPE) {
				pctl->SetImage(CGdipButton::STD_TYPE);
			}
			else {
				pctl->SetImage(CGdipButton::ALT_TYPE);
			}
			pctl->RedrawWindow();
		}
		m_pBoard->Close();
	}
}

void CPannel::OnBnClickedConset(UINT id)
{
	printf("con : %d\n", id - IDC_BTN_CON0);
	CButton* pBtn = (CButton*)GetDlgItem(id);
	int state = pBtn->GetCheck();
	pBtn->SetCheck(FALSE);

	if (m_pBoard == NULL || m_pBoard->isMount() == FALSE ) return;
	int con = id - IDC_BTN_CON0;
	CGdipButton* pctl = (CGdipButton*)this->GetDlgItem(con*3 + IDC_BTN_POWER_1);
	if (pctl == NULL) return;
	char val = (pctl->GetImage() == CGdipButton::STD_TYPE) ? 0 : 1;

	IO_OP op = { 0, NULL, {con, CON_CTL, {-1} } };
	memset(op.val.v.pin, val, 3);
	m_pBoard->SyncIO(con, &op);

	int img = val == 1 ? CGdipButton::STD_TYPE : CGdipButton::ALT_TYPE;
	pctl->SetImage(img);
	((CGdipButton*)(GetDlgItem(con*3 + IDC_BTN_KCOL0_1)))->SetImage(img);
	((CGdipButton*)(GetDlgItem(con*3 + IDC_BTN_HOMEKEY_1)))->SetImage(img);
	m_pBoard->Close();

}

void CPannel::OnBnClickedBtnSetall()
{
	SetAll(1);
}



void CPannel::SetCon(int id, int key)
{
	if (m_pBoard == NULL || m_pBoard->isMount() == FALSE) return;
	IO_OP op = { IO_ALL_CON, NULL, {IO_ALL_CON, CON_CTL, {-1}} };
	int con = IO_ALL_CON;
	CGdipButton* pctl = (CGdipButton*)this->GetDlgItem(id);
	op.val.v.pin[key] = (pctl->GetImage() == CGdipButton::STD_TYPE) ? 0 : 1;
	int img = op.val.v.pin[key] == 1 ? CGdipButton::STD_TYPE : CGdipButton::ALT_TYPE;
	if (m_pBoard->SyncIO(con, &op) == S_OK) {
		for (int i = 0; i < IO_CON_MAX; i++) {
			int ctrlid = id + i * IO_MAX_KEY;
			pctl = (CGdipButton*)this->GetDlgItem(ctrlid);
			pctl->SetImage(img);
		}
	}
	m_pBoard->Close();
}

void CPannel::SetAll(char val)
{
	if (m_pBoard == NULL || m_pBoard->isMount() == FALSE ) return;
	IO_OP op = { IO_ALL_CON, NULL, {IO_ALL_CON, CON_RAW, { -1}} , 0 };
	memset(op.val.v.pin, val, 3);
	int con = IO_ALL_CON;
	IO_VAL* praw_req = (IO_VAL*)&op.val;

	memset(&praw_req->v.raw, (val == 0) ? 0 : 0xFF, 3);
	if (m_pBoard->SyncIO(con, &op) == S_OK) {
		int img = val == 1 ? CGdipButton::STD_TYPE : CGdipButton::ALT_TYPE;
		for (int i = 0; i < IO_CON_MAX * IO_MAX_KEY; i++) {
			CGdipButton* pctl = (CGdipButton*)this->GetDlgItem(IDC_BTN_POWER_1 + i);
			pctl->SetImage(img);
		}
		SetGpioRaw(val);
	}
	m_pBoard->Close();
}

void CPannel::SetGpioRaw(int val)
{
	if (m_pBoard == NULL || m_pBoard->isMount() == FALSE ) return;
	CGdipButton* pctl = (CGdipButton*)this->GetDlgItem(IDC_BTN_GPIO_1);
	IO_OP op = { IO_CON_GPIO, NULL, {IO_CON_GPIO, GPIO_RAW, {-1}} , 0 };
	memset(op.val.v.pin, val, 3);
	int con = IO_CON_GPIO;
	op.val.v.gpio.bit = 0xFF;
	op.val.v.gpio.val = val == 0 ? 0 : 0xFF;
	int img = val == 0 ? CGdipButton::ALT_TYPE : CGdipButton::STD_TYPE;
	if (m_pBoard->SyncIO(con, &op) == S_OK) {
		for (int i = 0; i < IO_CON_MAX; i++) {
			int ctrlid = IDC_BTN_GPIO_1 + i;
			pctl = (CGdipButton*)this->GetDlgItem(ctrlid);
			pctl->SetImage(img);
		}
	}
	m_pBoard->Close();
}


void CPannel::OnBnClickedBtnPwrkey()
{
	SetCon(IDC_BTN_POWER_1, IO_PWRKEY);

}


void CPannel::OnBnClickedBtnKcol0()
{
	SetCon(IDC_BTN_KCOL0_1, IO_KCOL);
}


void CPannel::OnBnClickedBtnHome()
{
	SetCon(IDC_BTN_HOMEKEY_1, IO_HOME);
}


void CPannel::OnBnClickedBtnGpio()
{
	CGdipButton* pctl = (CGdipButton*)this->GetDlgItem(IDC_BTN_GPIO_1);
	int val =  (pctl->GetImage() == CGdipButton::STD_TYPE) ? 0 : 1;
	SetGpioRaw(val);
}


void CPannel::OnCbnSelchangeComboDev()
{
	this->UpdateData();
	CString str;
	int index = m_cbDevice.GetCurSel();
	m_cbDevice.GetLBText(index, str);
	int idx = _ttoi(str);
	LRESULT err;
	CTpvBoard* board = CFtdiDriver::GetDriver()->FindBoard(0, idx, &err);
	if (board) this->SetBoard(board);
}


BOOL CPannel::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect rcPannel;
	GetClientRect(rcPannel);
	CRgn rgn1;
	rgn1.CreateRectRgnIndirect(rcPannel);
	for (int i = 0; i < CON_MAX; i++) {
		for (int j = 0; j < ROW_MAX; j++) {
			CRgn rx;
			CRect rcbtn;
			m_cGpioBtns[i][j]->GetClientRect(rcbtn);
			rx.CreateRectRgnIndirect(rcbtn);
			rgn1.CombineRgn(&rgn1, &rx, RGN_XOR);
		}
	}
	int ids[] = {IDC_BTN_CLRALL,IDC_BTN_SETALL,	IDC_BTN_PWRKEY,	IDC_BTN_KCOL0, IDC_BTN_HOME, IDC_BTN_GPIO, IDC_COMBO_DEV};
	for (int i = 0; i < sizeof(ids)/sizeof(int); i++) {
		CRgn rx;
		CRect rcbtn;
		GetDlgItem(ids[i])->GetClientRect(rcbtn);
		rx.CreateRectRgnIndirect(rcbtn);
		rgn1.CombineRgn(&rgn1, &rx, RGN_XOR);
	}

	for (int i = IDC_BTN_CON0; i <= IDC_BTN_CON7; i++) {
		CRgn rx;
		CRect rcbtn;
		GetDlgItem(i)->GetClientRect(rcbtn);
		rx.CreateRectRgnIndirect(rcbtn);
		rgn1.CombineRgn(&rgn1, &rx, RGN_XOR);
	}



	CBrush* pbrush = new CBrush(0xF0F0F0);
	pDC->FillRgn(&rgn1, pbrush);
	delete pbrush;
	return TRUE;
	//return CExDialog::OnEraseBkgnd(pDC);
}
