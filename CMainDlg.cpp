// CMainDlg.cpp: 实现文件
//

#include "pch.h"
#include "FtdiUsbTpv.h"
#include "CMainDlg.h"
#include "afxdialogex.h"
#include"shellapi.h"
#include "CPca9505.h"
#define WM_TYAY_NOTIFY  (WM_USER+1031)
#define WM_QUERY_CONNECT  (WM_USER+1531)
//#define TMR_ID_SUB_DONE (1111)
#define TMR_ID_SCAN_USB (1113)
#define TMR_ID_READ_PIPE 1114
// CMainDlg 对话框
int parseOneLine(CHIP_TAB_LIST_T& chiplist, const char* cmdline);
int server_entry(CTpvBoard* pboard ,int argc, char** argv, BOOL isServer);

IMPLEMENT_DYNAMIC(CMainDlg, CExDialog)

CMainDlg::CMainDlg(CWnd* pParent /*=nullptr*/)
	: CExDialog(IDD_CMainDlg, pParent)
{
    m_conn_state = SERVER_NOT_READY;
    m_hIcon = AfxGetApp()->LoadIcon(IDI_BOARD);
}

CMainDlg::~CMainDlg()
{
    freePiPrint();
}

void CMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CExDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMainDlg, CExDialog)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_COPYDATA()
	ON_MESSAGE(WM_TYAY_NOTIFY, OnNotifyIcon)   //WM_LIBEN消息映射
	ON_MESSAGE(WM_QUERY_CONNECT, OnQueryConnection)   //WM_LIBEN消息映射
    ON_MESSAGE(WM_DEVICECHANGE, OnMyDeviceChange)
    ON_WM_ERASEBKGND()
    ON_WM_SIZE()
    ON_WM_PAINT()
    ON_COMMAND(ID_M_EXIT, &CMainDlg::OnMExit)
    ON_COMMAND(ID_M_TOGGLE_WND, &CMainDlg::OnMToggleWnd)
    ON_WM_CREATE()
END_MESSAGE_MAP()


void test_proc(void* param) {
    CMainDlg* pdlg = (CMainDlg*)param;
    Sleep(5000);
    char p[] = "run \n  -j test.json "\
        "{w,[0x0,0x1],0x18,[0,0,0,0,0]},"\
        "{w,[0x0,0x1],0x8,[0x20, 0x00, 0, 0, 0]} "\
        "{s,0x0,0x8,[0x40, 0x80, 0x14, 0x10, 0x10]}"\
        "{c,0x0,0x8,[0x20, 0x80, 0x10, 0x00, 0x10]}"\
        "{r,0x0,0x8}";
    COPYDATASTRUCT cds;
    cds.dwData = 1;
    cds.cbData = sizeof(p);
    cds.lpData = p;
    LRESULT val = ::SendMessage(pdlg->GetSafeHwnd(), WM_COPYDATA, 0, (WPARAM)&cds);

}
// CMainDlg 消息处理程序
typedef HWND(WINAPI* PROCGETTASKMANWND)(void);
PROCGETTASKMANWND GetTaskmanWindow;


BOOL CMainDlg::OnInitDialog()
{
	CExDialog::OnInitDialog();
    m_conn_state = SERVER_START;

    // TODO:  在此添加额外的初始化
    SetIcon(m_hIcon, TRUE);			// 设置大图标
    SetIcon(m_hIcon, FALSE);		// 设置小图标

    InitLog();
    logInfo(L"FDTI server start...\n");
    logInfo(L"DO NOT CLOSE this window!!!!\n");
    logInfo(L"DO NOT CLOSE this window!!!!\n");
    logInfo(L"DO NOT CLOSE this window!!!!\n");

    ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
    MoveWindow(0, 0, 0, 0);
    ShowWindow(SW_HIDE);
    
	// TODO:  在此添加额外的初始化
    SetNotifyIcon(NIM_ADD, IDI_NO_BOARD);
	
    SetTimer(TMR_ID_SCAN_USB, 100, NULL);
    
    this->SetWindowText(NAME_WND_MSG_MAIN);
    FdtiPrint = piPrint;
    FdtiPrint("");
    m_conn_state = SERVER_INIT;

    //_beginthread(test_proc, 1024, this);

#ifdef _DEBUG 
    ShowConsole();
#else
    HideConsole();
#endif

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类


	return CExDialog::PreTranslateMessage(pMsg);
}


void CMainDlg::OnDestroy()
{
	CExDialog::OnDestroy();
    SetNotifyIcon(NIM_DELETE, IDI_NO_BOARD);
    m_conn_state = SERVER_NOT_READY;
    Sleep(50);
    freePiPrint();
	// TODO: 在此处添加消息处理程序代码
}


void CMainDlg::OnTimer(UINT_PTR nIDEvent)
{
    KillTimer(nIDEvent);
    switch (nIDEvent ){
    case TMR_ID_SCAN_USB:
        LoadDriver();
        m_conn_state = (m_pboard)? BOARD_READY : BOARD_NOT_FOUND;
        return;
    default: 
        break;
    }
	CExDialog::OnTimer(nIDEvent);
}



#include <vector>
int split(std::string & strContent, const char* flag, std::vector<std::string>& vecDat)
{
    if (strContent.empty() || !flag)
        return -1;

    std::string strTemp;
    std::string::size_type nBeginPos = 0, nEndPos = 0;
    while (true)
    {
        nEndPos = strContent.find(flag, nBeginPos);
        if (nEndPos == std::string::npos)
        {
            
            strTemp = strContent.substr(nBeginPos, strContent.length());
            if (!strTemp.empty())
            {
                vecDat.push_back(strTemp);
            }
            break;
        }
        strTemp = strContent.substr(nBeginPos, nEndPos - nBeginPos);
        nBeginPos = nEndPos + strlen(flag);
        vecDat.push_back(strTemp);
    }
    return vecDat.size();
}

extern CStringA sExeName;

BOOL CMainDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
    char* pcmd = (char*)pCopyDataStruct->lpData;
    logInfo(L"Msg in:[%S]\n", pCopyDataStruct->lpData);
    std::string str = pcmd;
    std::vector<std::string>strlist;
    int items = split(str, "\n", strlist);
    LRESULT ret = -1;
    if (items) {
        if (strlist[0] == "stop") {
            CDialog::OnOK();
        }
        else  {
            CHIP_TAB_LIST_T chiplist;
            std::string strcmd = strlist[0];
            strcmd.erase(0, strcmd.find_first_not_of(" \t"));
            strcmd.erase(strcmd.find_last_not_of(" \t") + 1);
            int ret = 0;
            if (strcmd == "reset") {
                if (m_pboard) {
                    m_pboard->Reset();
                }
                else {
                    FdtiPrint("Device not ready!\n");
                }
            }else if (strcmd == "pipe_reset") {
                freePiPrint();
                FdtiPrint("");

            }else if (strcmd == "run") {
                if (m_pboard) {
                    cout << "run:[" << strlist[1] << endl; //debug info in server side
                    parseOneLine(chiplist, strlist[1].c_str());
                    if (chiplist.size() != 0) {
                        ret = m_pboard->Run(&chiplist);
                    }
                }
                else
                {
                    FdtiPrint("Device not ready!\n");
                }
            }
            else if (strcmd == "step") {
                char* *argv = new char* [strlist.size()+1];
                argv[0] = sExeName.GetBuffer();
                for (int i = 1; i < strlist.size(); i++) {
                    argv[i] = &strlist[i][0];
                }
                ret=  server_entry(m_pboard, strlist.size(), argv, TRUE);
                sExeName.ReleaseBuffer();
            }
        }
        
        freePiPrint();
        piPrint("");
        return ret;
    }
    
    return pCopyDataStruct->dwData+1;
	//return CExDialog::OnCopyData(pWnd, pCopyDataStruct);
}


LRESULT CMainDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类

	return CExDialog::WindowProc(message, wParam, lParam);
}

LRESULT CMainDlg::OnNotifyIcon(WPARAM wParam, LPARAM lParam)
{
    UINT uID;//发出该消息的图标的ID
    UINT uMouseMsg;//鼠标动作
    POINT pt;
    uID = (UINT)wParam;
    uMouseMsg = (UINT)lParam;
    if (uMouseMsg == WM_LBUTTONDOWN || uMouseMsg == WM_RBUTTONDOWN)//如果是单击右键
    {
        {

            CMenu menu;   //定义右键菜单对象
            GetCursorPos(&pt);   //获取当前鼠标位置
            menu.LoadMenu(IDR_TRAY_MENU);   //载入右键快捷菜单
            SetForegroundWindow();//放置在前面
            CMenu* pmenu;    //定义右键菜单指针
            pmenu = menu.GetSubMenu(0);      //该函数取得被指定菜单激活的下拉式菜单或子菜单的句柄
            ASSERT(pmenu != NULL);
            pmenu->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN, pt.x, pt.y, this);   //在指定位置显示右键快捷菜单
            HMENU hmenu = pmenu->Detach();
            pmenu->DestroyMenu();
        }
    }
	return LRESULT(0);
}

LRESULT CMainDlg::OnQueryConnection(WPARAM wParam, LPARAM lParam)
{
    return LRESULT(m_conn_state);
}

LRESULT CMainDlg::OnMyDeviceChange(WPARAM wParam, LPARAM lParam)
{
    KillTimer(TMR_ID_SCAN_USB);
    SetTimer(TMR_ID_SCAN_USB, 300, NULL);
    return LRESULT(0);
}


BOOL CMainDlg::OnEraseBkgnd(CDC* pDC)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    static bool bFirst = true;
    if (bFirst)
    {
        bFirst = false;
        ShowWindow(SW_HIDE);
        return TRUE;
    }
    return CExDialog::OnEraseBkgnd(pDC);
}


void CMainDlg::OnSize(UINT nType, int cx, int cy)
{
    CExDialog::OnSize(nType, cx, cy);

    // TODO: 在此处添加消息处理程序代码

}


void CMainDlg::OnPaint()
{
    CPaintDC dc(this); // device context for painting
                       // TODO: 在此处添加消息处理程序代码
                       // 不为绘图消息调用 CExDialog::OnPaint()

}


void CMainDlg::OnMExit()
{
    // TODO: 在此添加命令处理程序代码
    //if (this->IsIconic() || !this->IsWindowVisible())
    //{
    //    this->ShowWindow(SW_SHOWNORMAL);
    //}
    //else {
    //    this->ShowWindow(SW_HIDE);
    //}
    CDialog::OnOK();
    
}

void CMainDlg::SetNotifyIcon(int op, int iconid)
{
    NOTIFYICONDATA tnd;   //NOTIFYICONDATA 结构声明
    tnd.cbSize = sizeof(NOTIFYICONDATA);    //结构体的大小，以字节为单位。
    tnd.hWnd = this->m_hWnd;    //窗口句柄，标示的窗口用来接收与托盘图标相关的消息。
    tnd.uID = 128;  //应用程序定义的任务栏图标的标识符。Shell_NotifyIcon函数调用时，hWnd和uID成员用来标示具体要操作的图标
    tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;   //指示具体哪些其他成员为合法数据
    tnd.uCallbackMessage = WM_TYAY_NOTIFY;     //应用程序定义的消息标示，当托盘图标区域发生鼠标事件或者使用键盘选择或激活图标时，系统将使用此标示向由hWnd成员标示的窗口发送消息。
    tnd.hIcon = AfxGetApp()->LoadIcon(iconid);   //图标ID
    _tcscpy_s(tnd.szTip, 128, L"Mediatek Test Server");                 //托盘图标提示文本
    Shell_NotifyIcon(op, &tnd);      //安装托盘图标
}

void CMainDlg::LoadDriver()
{
    LRESULT err;
    m_conn_state = SCAN_DEVICES;
    if (CFtdiDriver::GetDriver()->Scan(TRUE) == S_OK) {
        if (m_pboard) return;
        CFtdiDriver::GetDriver()->MountDevices();
        m_pboard = CFtdiDriver::GetDriver()->FindBoard(0, 0, &err);
    }
    else {
        if (m_pboard == NULL) return;
        m_pboard = NULL;
    }
    if (m_pboard) {
        SetNotifyIcon(NIM_MODIFY, IDI_BOARD);
        auto orig = FdtiPrint;
        FdtiPrint = printf;
        m_pboard->RunScript("init.json");
        FdtiPrint = orig;
        logInfo(L"FDTI device ready!\n");
        piPrint("");

        //FdtiPrint = piPrint;
        //HICON hIcon = AfxGetApp()->LoadIcon(IDI_BOARD);
        //CWnd * pwnd = ::AfxGetMainWnd();
        //
        //::SendMessage(pwnd->GetSafeHwnd(), WM_SETICON, TRUE, (LPARAM)hIcon);
        //::SendMessage(pwnd->GetSafeHwnd(), WM_SETICON, FALSE, (LPARAM)hIcon);
    }
    else {
        //FdtiPrint = printf;
        SetNotifyIcon(NIM_MODIFY, IDI_NO_BOARD);
        
        //HICON hIcon = AfxGetApp()->LoadIcon(IDI_NO_BOARD);
        //CWnd* pwnd = ::AfxGetMainWnd();
        //::SendMessage(pwnd->GetSafeHwnd(), WM_SETICON, TRUE, (LPARAM)hIcon);
        //::SendMessage(pwnd->GetSafeHwnd(), WM_SETICON, FALSE, (LPARAM)hIcon);

    }

}


void CMainDlg::OnMToggleWnd()
{
    // TODO: 在此添加命令处理程序代码
    if (IsConsoleVisible()) {
        HideConsole();
    }
    else {
        ShowConsole();
    }
}


int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CExDialog::OnCreate(lpCreateStruct) == -1)
        return -1;

    // TODO:  在此添加您专用的创建代码
    auto ico= AfxGetApp()->LoadIcon(IDI_BOARD);

    SetIcon(ico, true);

    SetIcon(ico, false);

    return 0;
}
