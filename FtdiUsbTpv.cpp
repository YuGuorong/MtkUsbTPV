// FtdiUsbTpv.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "pch.h"
#include "framework.h"
#include "FtdiUsbTpv.h"
#include "CMainFrame.h"

#include "getopt.h"
#include "CFtDevice.h"
#include "CTpvBoard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DEBUG_ARG 0
// 唯一的应用程序对象
CWinApp theApp;
using namespace std;

#define PARAM_INDEX    'i'
#define PARAM_GPIO     'g'
#define PARAM_CON      'c'
#define PARAM_HOME     'o'
#define PARAM_KCOL     'k'
#define PARAM_PWRKEY   'p'
#define PARAM_RAWVAL   'r'
#define PARAM_SETVAL   's'
#define CMD_DISPLAY    'd'
#define CMD_LISTDEV    'l'
#define CMD_GUI        'u'
#define CMD_VERSION    'v'
#define CMD_HELP       'h'


const char* optstring = "vdluhg:i:c:o:k:p:s:r";
//required_argument
const struct option long_options[] = {
    {"index",   optional_argument, NULL, 'i'},
    {"gpio",    optional_argument, NULL, 'g'},
    {"con",     optional_argument, NULL, 'c'},
    {"home" ,   optional_argument, NULL, 'o'},
    {"kcol",    optional_argument, NULL, 'k'},
    {"pwrkey",  optional_argument, NULL, 'p'},
    {"raw",     required_argument, NULL, 'r'},
    {"set",     required_argument, NULL, 's'},
    {"display", no_argument,       NULL, 'd'},
    {"list",    no_argument,       NULL, 'l'},
    {"gui",     no_argument,       NULL, 'u'},
    {"version", no_argument,       NULL, 'v'},
    {"help",    no_argument,       NULL, 'h'},
    {"gpio0",   required_argument, NULL, '0'},
    {"gpio1",   required_argument, NULL, '1'},
    {"gpio2",   required_argument, NULL, '2'},
    {"gpio3",   required_argument, NULL, '3'},
    {"gpio4",   required_argument, NULL, '4'},
    {"gpio5",   required_argument, NULL, '5'},
    {"gpio6",   required_argument, NULL, '6'},
    {"gpio7",   required_argument, NULL, '7'},
    {0, 0, 0, 0}
};

char* testarg[] =
{
    "-h",
    "-v",
    "-l",
    "-i", "0",
    "-c", "-1",
    "-s", "0",
    "-d",
    "-s", "1",
    "-d",
    "-k", "0",
    "-d",
    "-p", "0",
    "-d",
    "-o", "0",
    "-d",
    "--gpio7", "0",
    "-d",
    "-c", "3",
    "-o", "0",
    "-p", "0",
    "-c", "7",
    "-o", "0",
    "-k", "0",
    "-g", "0xFF",
    "-c", "-1",
    "-d",
    "-c", "-1",
    "-o","1",
    "-k","1",
    "-p","1",
    "-d",
};

void showHelp(char* app)
{
    printf("usage %s [-h] [-i INDEX} [-g GPIO] [--gpio0~7 GPIOx] [-c CON] [-o HOME] [-k KCOL] [-p PWRKEY] [-r RAW_VALUE]\r\n"
        "                  [| -s | -d | -l | u | v ]\r\n", app);
    printf("optional arguments:\r\n");
    printf("  -h, --help              Show this help message and exit\r\n");
    printf("  -i INDEX, --interface INDEX\r\n");
    printf("                          Specify the board interface to be controlled (default 0)\r\n");
    printf("  -g GPIO                 Specify output GPIO(8bit)  (default 0)\r\n");
    printf("  --gpio0~7 GPIOx         Specify output GPIO0~GPIO7 (default 0)\r\n");
    printf("  -c CON, --con CON       Specify CON0~CON7 connector to be controlled, CON=-1 means all(default -1)\r\n");
    printf("  -o HOME, --home HOME    Specify HOMEKEY output of CON(0:LOW, 1:HIGH)\r\n");
    printf("  -k KCOL, --kcol KCOL    Specify KCOL0 output of CON(0:LOW, 1:HIGH)\r\n");
    printf("  -p PWRKEY, --pwrkey PWRKEY\r\n");
    printf("                          Specify  PWRKEY output of CON(0:LOW, 1:HIGH)\r\n");
    printf("  -r RAW_VALUE, --raw RAW_VALUE\r\n");
    printf("                          Specify all CONs to a specify value, each CON has 3 bits for\r\n");
    printf("                          HOOMEKEY_KCOL0_PWEKEY(LSB)\r\n");
    printf("  -s, --set               Specify the output of the selected keys on CON\r\n");
    printf("  -d, --display           Show the status of CONs, CONs=9 show GPIO, CON=-1 means all(default -1) \r\n");
    printf("  -l, --list              Show connected FT2322 devices\r\n");
    printf("  -u, --gui               Show the dialog of FT2322 devices control\r\n");
    printf("  -v, --version           Show version information\r\n");
}



LRESULT ProcIOReq(CTpvBoard * pboard, IO_OP& cur_req, vector<IO_OP>& io_reqQue) {

    if (pboard != NULL) {
        if (memcmp(cur_req.val.v.pin, "\xFF\xFF\xFF", 3) != 0) {
            io_reqQue.push_back(cur_req);
            memset(cur_req.val.v.pin, 0xFF, 3);
        }
        pboard->SyncIO(io_reqQue);
        io_reqQue.clear();
        //cur_req = def_request;
    }

    return 0;
}

void DisplayConn(CTpvBoard * board, char con, int index) {
    if (board == NULL) {
        printf("not found the index board: %d ", index);
    }
    else {
        if (con == IO_CON_UNKNOWN) con = IO_ALL_CON;
        int items;
        IO_VAL ioval[IO_CON_MAX+4];
        board->Display((int)con, ioval, &items);

        printf("[Board_%d] status:\n", index);
        BOOL hdrPrinted = FALSE;
        for (int i = 0; i < items; i++) {
            if (ioval[i].con == IO_CON_GPIO) {
                BYTE v = ioval[i].v.gpio.val;
                printf("GPIO:  7  6  5  4 - 3  2  1  0\n"
                    "       %d  %d  %d  %d - %d  %d  %d  %d\n",
                    !(!(v & 0x80)), !(!(v & 0x40)), !(!(v & 0x20)), !(!(v & 0x10)),
                    !(!(v & 0x8)), !(!(v & 0x4)), !(!(v & 0x2)), !(!(v & 0x1)) );
            }
            else {
                if (hdrPrinted == FALSE) {
                    hdrPrinted = TRUE;
                    printf("      | HOME | KCOL | PWRKEY| \n");
                    //printf("------------------------------\n");
                }
                char* pv = ioval[i].v.pin;
                printf("CON%d: |   %d  |   %d  |   %d   |\n", 
                    ioval[i].con, pv[IO_HOME], pv[IO_KCOL], pv[IO_PWRKEY]);
            }
            
        }
    }

}

BOOL ChkIdx(int index) {
    if (index < 0) {
        printf("Please specify the board index before other parameter!!\r\n");
        SetFaultError(ERROR_INVALID_ADDRESS);
        return FALSE;
    }
    return TRUE;
}
#define CHKIDX() ChkIdx(cur_req.index )
int server_entry(int argc, char** argv)
{
    int nRetCode = 0;
    LRESULT err;
    InitLog();
    LPWSTR szargs = GetCommandLineW();
    logTrace(L"Program start with arg(%d): [%s]", argc, szargs);



    CFtdiDriver  drvFdti;
    drvFdti.Scan(FALSE);
    drvFdti.MountDevices();

    int opt;
    int digit_optind = 0;
    int option_index = 0;
    int boardindex = -1;

    vector<IO_OP>io_reqQue;
    
    IO_OP cur_req = def_request;
    IO_OP qk_req = def_request;

    CTpvBoard* pboard = NULL;

    if (argc <= 1) {
        {CMainFrame dlg; dlg.DoModal();     }
        //showHelp(argv[0]);
        return 1;
    }

    while ((opt = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1)
    {
        int argval =  (optarg == NULL )? 0 : atoi(optarg);

        logInfo(L"opt= '%c'[0x%02X] , arg=[%S], option_idex=%d", opt, opt, optarg, option_index);

        switch (opt) {
        case PARAM_INDEX:
            ProcIOReq(pboard, cur_req, io_reqQue);
            cur_req.index = argval;
            boardindex = argval;
            pboard = drvFdti.FindBoard(0, argval, &err);
            if (pboard == NULL) SetFaultError(err);
            break;
        case PARAM_CON:
            if (optarg == NULL) argval = -1;
            if (cur_req.val.con >= IO_ALL_CON) {
                io_reqQue.push_back(cur_req);
                cur_req = def_request;
                cur_req.index = boardindex;
            }
            cur_req.val.con = argval;
            break;

        case PARAM_HOME:   if (CHKIDX()) cur_req.val.v.pin[IO_HOME] = argval;   break;
        case PARAM_KCOL:   if (CHKIDX()) cur_req.val.v.pin[IO_KCOL] = argval;   break;
        case PARAM_PWRKEY: if (CHKIDX()) cur_req.val.v.pin[IO_PWRKEY] = argval; break;
        case PARAM_RAWVAL: if (CHKIDX()) qk_req=cur_req, qk_req.val.v.raw=atox(optarg, 1), qk_req.val.fmt= CON_RAW;  break;
        case PARAM_SETVAL: if (CHKIDX()) qk_req = cur_req, qk_req.val.v.raw = argval == 0 ? 0 : 0xFF, qk_req.val.fmt = IO_RAW; break;
        case CMD_DISPLAY:  if (CHKIDX()) ProcIOReq(pboard, cur_req, io_reqQue);  DisplayConn(pboard, cur_req.val.con, boardindex); break;
        case CMD_LISTDEV: drvFdti.ShowDevices(); break;
        case CMD_GUI:     {CMainFrame dlg; dlg.DoModal();     }        break;
        case CMD_VERSION: printf("version : 1.0.0.1\r\n"); break;
        case PARAM_GPIO:  
            if (CHKIDX()) {
                qk_req = cur_req;
                qk_req.val.con = IO_CON_GPIO;
                qk_req.val.fmt = GPIO_RAW;
                qk_req.val.v.gpio.bit = 0xFF;
                qk_req.val.v.gpio.val = atox(optarg, 1);
            }
            break;
        case CMD_HELP:showHelp(argv[0]); break;
        case '?':
            break;
        }
        if (opt >= '0' && opt <= '7' && CHKIDX()) {
            qk_req = cur_req;
            qk_req.val.con = IO_CON_GPIO;
            qk_req.val.fmt = GPIO_CTL;
            qk_req.val.v.gpio.bit = opt - '0';
            qk_req.val.v.gpio.val = argval;
        }

        if (qk_req.index != -1 ) {
            io_reqQue.push_back(qk_req);
            qk_req = def_request;
        }

        if (err = GetFaultError()) {
            CString serr = ErrorString(err);
            logError(L"Fault error[%d]: (%s)", err, (LPCTSTR)serr);
            nRetCode = err;
            break;
        }
    }
    ProcIOReq(pboard, cur_req, io_reqQue);
    io_reqQue.clear();

    logTrace(L"Program exit with result : [%d]", nRetCode);
    return nRetCode;
}


#include<DbgHelp.h>
using namespace std;
#pragma comment(lib,"DbgHelp.lib")

// 创建Dump文件
void CreateDumpFile(LPCWSTR lpstrDumpFilePathName, EXCEPTION_POINTERS* pException)
{
    HANDLE hDumpFile = CreateFile(lpstrDumpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    // Dump信息
    MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
    dumpInfo.ExceptionPointers = pException;
    dumpInfo.ThreadId = GetCurrentThreadId();
    dumpInfo.ClientPointers = TRUE;
    // 写入Dump文件内容
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
    CloseHandle(hDumpFile);
}

// 处理Unhandled Exception的回调函数
LONG ApplicationCrashHandler(EXCEPTION_POINTERS* pException)
{
    CreateDumpFile(L"crash.dmp", pException);
    printf("异常已记录到文件：crash.dmp");
    system("pause");
    return EXCEPTION_EXECUTE_HANDLER;
}


int main(int argc, char* argv[])
{
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule == nullptr)
    {   // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        return 1;
    }

    // 初始化 MFC 并在失败时显示错误
    if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
    {
        // TODO: 在此处为应用程序的行为编写代码。
        wprintf(L"错误: MFC 初始化失败\n");
        return 1;
    }
    //::AfxInitRichEdit2();
    //_CrtSetBreakAlloc(132); //383为上面内存泄漏的块号.
#if DEBUG_ARG
    argc = sizeof(testarg) / sizeof(char*);
    return server_entry(argc, testarg);
#else
    return server_entry(argc, argv);
#endif
}
