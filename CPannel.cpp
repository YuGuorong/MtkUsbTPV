// CPannel.cpp: 实现文件
//

#include "pch.h"
//#include "MtkUsbTPV.h"
#include "resource.h"
#include "CPannel.h"
#include "afxdialogex.h"


// CPannel 对话框

IMPLEMENT_DYNAMIC(CPannel, CExDialog)

CPannel::CPannel(CWnd* pParent /*=nullptr*/)
	: CExDialog(IDD_PANNEL, pParent)
{

}

CPannel::~CPannel()
{
}

void CPannel::DoDataExchange(CDataExchange* pDX)
{
	CExDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPannel, CExDialog)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_CLRALL, &CPannel::OnBnClickedBtnClrall)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BTN_POWER_1, IDC_BTN_HOMEKEY_8, &CPannel::OnBnClickedGpioCtl)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_BTN_GPIO_1, IDC_BTN_GPIO_8, &CPannel::OnBnClickedGpioCtl)
END_MESSAGE_MAP()


// CPannel 消息处理程序

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


void CPannel::OnSize(UINT nType, int cx, int cy)
{
	CExDialog::OnSize(nType, cx, cy);

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
	// TODO: 在此添加控件通知处理程序代码

	if (m_AllBtns[0]->GetImage() == CGdipButton::ALT_TYPE) {
		m_AllBtns[0]->SetImage(CGdipButton::STD_TYPE);
	}
	else {
		m_AllBtns[0]->SetImage(CGdipButton::ALT_TYPE);
	}
	m_AllBtns[0]->RedrawWindow();


}

void CPannel::OnBnClickedGpioCtl(UINT id)
{
	if ((id >= IDC_BTN_POWER_1 && id <= IDC_BTN_HOMEKEY_8) ||
		(id >= IDC_BTN_GPIO_1 && id <= IDC_BTN_GPIO_8))
	{
		CGdipButton* pctl = (CGdipButton*)this->GetDlgItem(id);
		if (pctl) {
			if (pctl->GetImage() == CGdipButton::ALT_TYPE) {
				pctl->SetImage(CGdipButton::STD_TYPE);
			}
			else {
				pctl->SetImage(CGdipButton::ALT_TYPE);
			}
			pctl->RedrawWindow();
		}
	}

}