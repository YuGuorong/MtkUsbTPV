// SubPanelDlg.cpp : ʵ���ļ�
//


#include "pch.h"
#include "SubPanelDlg.h"
#include "afxdialogex.h"


// CSubPanelDlg �Ի���
#define TMR_ID_UPDATE_STATE   5001
#define TMR_ID_UPDATA_CONSOL  5002
IMPLEMENT_DYNAMIC(CSubPanelDlg, CExDialog)

CSubPanelDlg::CSubPanelDlg(CWnd* pParent /*=NULL*/)
	: CExDialog(IDD_SUBPANEL_DLG, pParent)
{
	m_of = NULL;
	m_strDevId = m_strDlFile = m_strVserion = m_strBrand = _T("");  //Brand and type information, for query rom&root
	m_state = META_IDLE;
	m_bMount = FALSE;
	m_bInit = FALSE;
	m_strDevId.Empty();
	m_pBurn = NULL;
	m_pRoms = NULL;
	m_bReflash = FALSE;
	m_extexe = NULL;
}

CSubPanelDlg::~CSubPanelDlg()
{
	if (m_of) {
		delete m_of;
		m_of = NULL;
	}
	if (m_pBurn)
	{
		m_pBurn->stop();
		delete m_pBurn;
		m_pBurn = NULL;
	}
}

void CSubPanelDlg::DoDataExchange(CDataExchange* pDX)
{
	CExDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, m_oProc);
	DDX_Control(pDX, IDC_ST_CAPTION, m_oCaption);
	DDX_Control(pDX, IDC_EDIT_CONSOL, m_oConsol);
}


BEGIN_MESSAGE_MAP(CSubPanelDlg, CExDialog)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_REFLASH, &CSubPanelDlg::OnBnClickedBtnReflash)
END_MESSAGE_MAP()


void CSubPanelDlg::OnSize(UINT nType, int cx, int cy)
{
	CExDialog::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
	if (IsWindow(this->GetSafeHwnd()) && IsWindow(m_oLogInfo.GetSafeHwnd()))
	{
		CRect r, rt;
		m_oProc.GetWindowRect(r);
		m_oLogInfo.GetWindowRect(rt);
		this->ScreenToClient(rt);
		this->ScreenToClient(r);
		r.right = cx - (rt.left*2);
		m_oProc.MoveWindow(r);
		rt.right = r.right;
		rt.bottom = cy - rt.left - r.top;
		m_oLogInfo.MoveWindow(rt);
	}
}

BOOL __stdcall subpannel_async_run_CB(
	const void * const pUserData,
	const CHAR * const pButOutput,
	DWORD dwOutputSize
)
{
	CSubPanelDlg * dlg = (CSubPanelDlg*)pUserData;
	//dlg->GetDlgItem(IDC_ST_VERSION)->SetWindowTextW
	//OutputDebugStringA(pButOutput);
	dlg->OnAsyncRunCallBack(pButOutput);
	return TRUE;
}

BOOL CSubPanelDlg::OnInitDialog()
{
	CExDialog::OnInitDialog();

	m_oLogInfo.SubclassDlgItem(IDC_TXT_LOGINFO, this);
	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	m_of = new CFont;
	m_of->CreateFont(14,            // nHeight 
		0,           // nWidth 
		0,           // nEscapement 
		0,           // nOrientation 
		FW_BOLD,     // nWeight 
		0,           // bItalic 
		FALSE,       // bUnderline 
		0,           // cStrikeOut 
		DEFAULT_CHARSET,              // nCharSet 
		OUT_DEFAULT_PRECIS,        // nOutPrecision 
		CLIP_DEFAULT_PRECIS,       // nClipPrecision 
		DEFAULT_QUALITY,           // nQuality 
		DEFAULT_PITCH | FF_SWISS, // nPitchAndFamily 
		_T("����"));              // lpszFac

	m_state = META_IDLE;
	m_bInit = TRUE;

	m_extexe = new CInvokeExternalExe(subpannel_async_run_CB, this);
	
	ZeroMemory(&m_cf, sizeof(CHARFORMAT2));
	m_cf.cbSize = sizeof(CHARFORMAT2);
	m_cf.dwEffects = 0;
	m_cf.dwMask |= CFM_BOLD;
	m_cf.dwEffects |= CFE_BOLD;//���ô��壬ȡ����cf.dwEffects&=~CFE_BOLD;
	m_cf.dwMask |= CFM_COLOR;
	m_cf.crTextColor = RGB(0, 0, 0);//������ɫ
	m_cf.dwMask |= CFM_SIZE;
	m_cf.yHeight = 240;//���ø߶�
	m_cf.dwMask |= CFM_FACE;
	_tcscpy_s(m_cf.szFaceName, 32, _T("����"));//��������
	//cf.dwMask |= CFM_BACKCOLOR;
	//cf.crBackColor = RGB(0, 0, 255);

	m_oLogInfo.SetSel(-1, -1);
	m_oLogInfo.SetDefaultCharFormat(m_cf);


	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CSubPanelDlg::OnDestroy()
{
	CExDialog::OnDestroy();

	if (m_extexe) {
		delete m_extexe;
		m_extexe = NULL;
	}
	// TODO: �ڴ˴������Ϣ����������
}


void CSubPanelDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CExDialog::OnClose();
}


int CSubPanelDlg::StartRun()
{
	//CreateThread()
	return 0;
}

void CSubPanelDlg::SetRomDescript(LPVOID * const pRoms)
{
	m_pRoms = pRoms;
}


int CSubPanelDlg::StopRun()
{
	if( m_extexe )
		m_extexe->CancelInvoke();
	if (m_pBurn)
	{
		m_pBurn->stop();
		m_pBurn->Reset();
	}
	return 0;
}

void CSubPanelDlg::SetIndex(int idx)
{
	CString strcap;
	strcap.Format(_T("�ն�%d"), idx + 1);
	m_DevId = idx;
	CWnd * pwnd = GetDlgItem(IDC_ST_CAPTION);
	if (pwnd) {
		pwnd->SetWindowText(strcap);
		pwnd->SetFont(m_of);
	}
	strcap.Format(_T("[�ն�%d]�� �豸δ����"), m_DevId+1);
	SetInfoText(strcap, INFO_WARNING);
}

void CSubPanelDlg::OpenDevice(CString & strDevid, BOOL bAdbDev)
{
	m_strDevId = strDevid;
	m_bAdbDev = bAdbDev;
	m_state = META_ALLOCATE;
	GetDlgItem(IDC_BTN_REFLASH)->ShowWindow(SW_HIDE);
	//m_pQueryDev = new CDelegate(this);
	//m_pfnDelegate = &CMetaDlg::QueryDevInfo;
	CString str;
	str.Format(_T("�豸[SN:%s]�����ӣ���ѯ�豸��Ϣ..."), strDevid);
	SetInfoText(str, INFO_REPLACE | INFO_BLUE);
	m_pBurn = new CDelegate(this);
	m_bReflash = FALSE;
	m_pBurn->start(this);
}

void CSubPanelDlg::ReOpenDevice()
{	
}

void CSubPanelDlg::QueryDevInfo()
{
}

int CSubPanelDlg::DevcieRemvoed()
{
	if (m_state & META_BUSY) return -1;
		if (m_pBurn)
	{
		m_pBurn->stop();
		delete m_pBurn;
		m_pBurn = NULL;
	}
	m_strDevId.Empty();
	CString str;
	str.Format(_T("[�ն�%d]�� �豸�ѶϿ�"), m_DevId);
	GetDlgItem(IDC_BTN_REFLASH)->ShowWindow(SW_HIDE);
	SetInfoText(str);
	m_strDevId.Empty();
	m_bMount = FALSE;
	m_strBrand = _T("");
	m_state = META_IDLE;

	return 0;
}

void CSubPanelDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (nIDEvent == TMR_ID_UPDATE_STATE)
	{
		//m_oProc.SetPos( (m_iiBurnedLen + m_spare_len )/ SIZE_K);
	}
	CExDialog::OnTimer(nIDEvent);
}

void CSubPanelDlg::SetInfoSel(int start, int end)
{
	m_oLogInfo.SetSel(-1, -1);
}

void CSubPanelDlg::SetInfoText(LPCTSTR sz, int mode)
{
	CString s, str = sz;
	int start = 0;
	switch (mode & 0xff)
	{
	case INFO_APPEND:
		m_oLogInfo.SetSel(-1, -1);
 		m_oLogInfo.GetWindowText(s);
		if (!s.IsEmpty()) {
			m_oLogInfo.ReplaceSel(_T("\r\n"));
			start = m_oLogInfo.GetWindowTextLength();
		}
		break;
	case INFO_REPLACE:
		m_oLogInfo.SetSel(0, -1);
		break;
	default:
	case INFO_SELECTED:
		break;
	}


	CHARFORMAT2 cf = m_cf;
	ZeroMemory(&cf, sizeof(CHARFORMAT2));
	cf.dwMask |= CFM_COLOR;
	mode = (mode>>8) & 0xFF;
	if (mode > (INFO_ALARM>>8) || mode < (INFO_DEFAULT>>8))
		return;
	const COLORREF clr_indx[] = { RGB(0,0,0), RGB(0,200,64), RGB(0,0,255), RGB(233,220,0), RGB(255,0,0) };
	cf.crTextColor = clr_indx[mode-1]; //������ɫ

	m_oLogInfo.SetSelectionCharFormat(cf);
	m_oLogInfo.ReplaceSel(str);

	int nFirstVisible = m_oLogInfo.GetLineCount();
	if (nFirstVisible > 0)
	{
		m_oLogInfo.LineScroll(nFirstVisible, 0);
	}
}


void CSubPanelDlg::DelegateRun()
{
	if (m_pRoms == NULL)  return;
	m_state |= META_BUSY;
	BOOL bFastbootMode = FALSE;
	
	CString strcmd, sver; 
	//m_oProc.SetPos(0);
	//m_oProc.SetRange32(0, (int)((m_pRoms->m_Romsize64 + SIZE_K - 1)/ SIZE_K));
	//��ʼ time beging
	CTime tRecordTime = CTime::GetCurrentTime();
	m_strInvoke.Empty();

	try {
		sver.Empty();
		strcmd.Format(_T("fastboot -s %s devices"), m_strDevId);
		int ec = RunCmd(strcmd, sver);
		if ( sver.IsEmpty() )
		{
			strcmd.Format(_T("adb -s %s shell getprop ro.build.description"), m_strDevId);
			ec = RunCmd(strcmd, sver);
			if (ec != 0) throw _T("�޷���ȡ�豸��ǰ�汾�����豸ͨ�Ų�������");
			sver = sver.Trim();
		}
		else {
			sver = _T("Fastboot mode ֱ��ˢ��!");
			bFastbootMode = TRUE;
		}

		if ( bFastbootMode || m_bReflash ) {
			SetInfoText(sver, INFO_BLUE);
			
			if (bFastbootMode == FALSE) {
				strcmd.Format(_T("adb -s %s reboot bootloader"), m_strDevId);
				sver.Empty();
				ec = RunCmd(strcmd, sver);
				if (ec != 0)  throw  _T("�豸�޷�������¼ģʽ��");
			}
			DWORD dwtick_burn = ::GetTickCount();

			m_iiBurnedLen = 0;
			//for (int i = 0; i < m_pRoms->m_roms.GetSize(); i++) {
			//	if (m_pRoms->m_roms[i].selcet) {
			//		tRecordTime = CTime::GetCurrentTime();
			//		
			//		CString strRecordTime = tRecordTime.Format("%H:%M:%S ");//
			//		CString strinfo;
			//		strinfo.Format(_T("%s[%s] \t:\t%s"), strRecordTime, m_pRoms->m_roms[i].partion, m_pRoms->m_roms[i].file);
			//		SetInfoText(strinfo);
			//		sver.Empty();
			//		m_spare_len = 0;
			//		m_strInvoke.Empty();
			//		
			//		strcmd.Format(_T("Cmd.exe /C fastboot -s  %s flash %s %s"), m_strDevId,
			//			m_pRoms->m_roms[i].partion, m_pRoms->m_roms[i].file);
			//		tstring strerr;
			//		DWORD dwec;
			//		m_extexe->Invoke(NULL, strcmd, TRUE, 0, SW_HIDE, dwec, strerr);
			//		if (dwec != 0 ) throw strerr.c_str();
			//		m_oConsol.ShowWindow(SW_HIDE);
			//		m_iiBurnedLen += m_pRoms->m_roms[i].len_rom;
			//		SetTimer(TMR_ID_UPDATE_STATE, 30, NULL);
			//	}
			//}
			sver.Empty();
			strcmd.Format(_T("fastboot -s %s reboot"), m_strDevId);
			ec = RunCmd(strcmd, sver);
			if( ec != 0 ) throw _T("�豸��¼��ɣ��޷�������");
			SetInfoText(_T("��¼��ɣ�����ϵͳ��"));
			SetInfoText(sver);
			tRecordTime = CTime::GetCurrentTime();
			dwtick_burn = ::GetTickCount() - dwtick_burn;
			
			CString strRecordTime = tRecordTime.Format(_T("[%H:%M:%S], �豸�������! "));//
			sver.Format(_T("��ʱ��%d.%02d��"), dwtick_burn / 1000, (dwtick_burn & 1000)/10);
			strRecordTime += sver;
			SetInfoText(strRecordTime, INFO_GREEN);
		}
		else {
			SetInfoText(sver, INFO_BLUE);
			CString strRecordTime = tRecordTime.Format(_T("[%H:%M:%S],�豸������! "));//
			SetInfoText(strRecordTime, INFO_GREEN);
			GetDlgItem(IDC_BTN_REFLASH)->ShowWindow(SW_SHOW);
		}
	}
	catch (LPCTSTR err)
	{
		tRecordTime = CTime::GetCurrentTime();
		CString strRecordTime = tRecordTime.Format("[%H:%M:%S], ��¼����\r\n");//
		strRecordTime += m_strInvoke;
		strRecordTime += _T("\r\n");
		strRecordTime += err;
		SetInfoText(strRecordTime, INFO_ALARM);
	}
	catch (...)
	{
		tRecordTime = CTime::GetCurrentTime();
		CString strRecordTime = tRecordTime.Format("[%H:%M:%S], ��¼����\r\n");//
		strRecordTime += m_strInvoke;
		strRecordTime += _T("\r\n");
		SetInfoText(strRecordTime, INFO_ALARM);

	}
	m_oConsol.ShowWindow(SW_HIDE);
	//����ʱ�� time beging
	m_state &= ~META_BUSY;
	m_bReflash = FALSE;
	

	::PostMessage(::AfxGetMainWnd()->GetSafeHwnd(), WM_TIMER, TMR_ID_SUB_DONE, this->m_DevId);
}



void CSubPanelDlg::OnAsyncRunCallBack(const CHAR * pcons_txt)
{
	CString str = QA2W(pcons_txt);
	INT pos = 0;
	if (str.IsEmpty()) return;
	m_strInvoke += str;
	//sending sparse 'system' (260580 KB)...
	if (str.Find(_T("sending sparse")) >= 0) {
		m_oConsol.SetWindowText(m_strInvoke);
		m_oConsol.LineScroll(m_oConsol.GetLineCount());
		if (!m_oConsol.IsWindowVisible())
		{
			CRect r, rl;
			m_oLogInfo.GetWindowRect(r);
			this->ScreenToClient(r);
			m_oConsol.GetWindowRect(rl);
			this->ScreenToClient(rl);
			if (rl.Width() < r.Width() && rl.Height() < r.Height()) {
				rl.MoveToXY(r.left + 2, r.bottom - rl.Height() - 2);
				m_oConsol.MoveWindow(rl);
				m_oConsol.ShowWindow(SW_SHOW);
			}
		}
	}
	while ((pos = str.Find(_T("sending sparse"), pos)) >= 0)
	{
		int p2 = pos;
		int p1 = str.Find(_T("("), pos);
		if (p1) {
			p2 = str.Find(_T("KB)..."), p1);
			if (p2 > 0)
			{
				CString slen = (str.Mid(p1+1, p2 - p1 - 1 )).Trim();
				m_spare_len += _ttoi(slen) * 1024;
				SetTimer(TMR_ID_UPDATE_STATE, 30, NULL);
				pos = p2 + 5;
				continue;
			}
		}
		break;
	}
}

void CSubPanelDlg::OnBnClickedBtnReflash()
{
	GetDlgItem(IDC_BTN_REFLASH)->ShowWindow(SW_HIDE);
	if (m_pBurn) {
		m_bReflash = TRUE;
		m_pBurn->Reset();
		m_pBurn->start(this);
	}
}
