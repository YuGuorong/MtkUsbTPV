﻿// FtdiUsbTpv.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
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

#ifdef _DEBUG
#define DEBUG_ARG 0
#else
#define DEBUG_ARG 0
#endif
#define WM_QUERY_CONNECT  (WM_USER+1531)
LRESULT SendBoardMessage(const char* msg);

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
#define CMD_SEL_MASTER 'm'
#define CMD_RUN_SCRIPT 'R'


Print_T  FdtiPrint = printf;

const char* optstring = "vdluhg:i:c:o:k:p:s:r:m:R:";
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
    {"master",  required_argument, NULL, 'm'},
    {"run",     required_argument, NULL, 'R'},
    {0, 0, 0, 0}
};

char* testarg[] =
{
    "-h",
    "-R", "test.json",
    "-m", "1",
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
    FdtiPrint("usage %s [-h] [-i INDEX} [-g GPIO] [--gpio0~7 GPIOx] [-c CON] [-o HOME] [-k KCOL] [-p PWRKEY] [-r RAW_VALUE]\r\n"
        "                  [| -s | -d | -l | u | v ]\r\n", app);
    FdtiPrint("optional arguments:\r\n");
    FdtiPrint("  -h, --help              Show this help message and exit\r\n");
    FdtiPrint("  -i INDEX, --interface INDEX\r\n");
    FdtiPrint("                          Specify the board interface to be controlled (default 0)\r\n");
    FdtiPrint("  -g GPIO                 Specify output GPIO(8bit)  (default 0)\r\n");
    FdtiPrint("  --gpio0~7 GPIOx         Specify output GPIO0~GPIO7 (default 0)\r\n");
    FdtiPrint("  -c CON, --con CON       Specify CON0~CON7 connector to be controlled, CON=-1 means all(default -1)\r\n");
    FdtiPrint("  -o HOME, --home HOME    Specify HOMEKEY output of CON(0:LOW, 1:HIGH)\r\n");
    FdtiPrint("  -k KCOL, --kcol KCOL    Specify KCOL0 output of CON(0:LOW, 1:HIGH)\r\n");
    FdtiPrint("  -p PWRKEY, --pwrkey PWRKEY\r\n");
    FdtiPrint("                          Specify  PWRKEY output of CON(0:LOW, 1:HIGH)\r\n");
    FdtiPrint("  -r RAW_VALUE, --raw RAW_VALUE\r\n");
    FdtiPrint("                          Specify all CONs to a specify value, each CON has 3 bits for\r\n");
    FdtiPrint("                          HOOMEKEY_KCOL0_PWEKEY(LSB)\r\n");
    FdtiPrint("  -s, --set               Specify the output of the selected keys on CON\r\n");
    FdtiPrint("  -d, --display           Show the status of CONs, CONs=9 show GPIO, CON=-1 means all(default -1) \r\n");
    FdtiPrint("  -l, --list              Show connected FT2322 devices\r\n");
    FdtiPrint("  -u, --gui               Show the dialog of FT2322 devices control\r\n");
    FdtiPrint("  -v, --version           Show version information\r\n");
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
    else
    {
        FdtiPrint("Board not ready!\n");
    }

    return 0;
}

void DisplayConn(CTpvBoard * board, char con, int index, int master) {
    if (board == NULL) {
        FdtiPrint("Board not found!\n");
    }
    else {
        if (con == IO_CON_UNKNOWN) con = IO_ALL_CON;
        int items;
        IO_VAL ioval[IO_CON_MAX+4];
        board->Display((int)con, ioval, &items);

        FdtiPrint("%s Board status:\n", master == 0 ? "Master" : "Slave");
        BOOL hdrPrinted = FALSE;
        for (int i = 0; i < items; i++) {
            if (ioval[i].con == IO_CON_GPIO) {
                BYTE v = ioval[i].v.gpio.val;
                FdtiPrint("GPIO:  7  6  5  4 - 3  2  1  0\n"
                    "       %d  %d  %d  %d - %d  %d  %d  %d\n",
                    !(!(v & 0x80)), !(!(v & 0x40)), !(!(v & 0x20)), !(!(v & 0x10)),
                    !(!(v & 0x8)), !(!(v & 0x4)), !(!(v & 0x2)), !(!(v & 0x1)) );
            }
            else {
                if (hdrPrinted == FALSE) {
                    hdrPrinted = TRUE;
                    FdtiPrint("      | HOME | KCOL | PWRKEY| \n");
                    //FdtiPrint("------------------------------\n");
                }
                char* pv = ioval[i].v.pin;
                FdtiPrint("CON%d: |   %d  |   %d  |   %d   |\n", 
                    ioval[i].con, pv[IO_HOME], pv[IO_KCOL], pv[IO_PWRKEY]);
            }
            
        }
    }

}

BOOL ChkIdx(int index) {
    if (index < 0) {
        FdtiPrint("Please specify the board index before other parameter!!\r\n");
        SetFaultError(ERROR_INVALID_ADDRESS);
        return FALSE;
    }
    return TRUE;
}
#define CHKIDX() ChkIdx(cur_req.index )



int server_entry(CTpvBoard* pboard ,int argc, char** argv, BOOL isServer)
{
    int nRetCode = 0;
    LRESULT err;
    InitLog();
    if (isServer == FALSE && pboard == NULL) {
        CFtdiDriver  drvFdti;
        drvFdti.Scan(FALSE);
        drvFdti.MountDevices();
    }

    int opt;
    int digit_optind = 0;
    int option_index = 0;
    int boardindex = (isServer) ?  0 : -1;
    int master = 0;

    vector<IO_OP>io_reqQue;
    
    IO_OP cur_req = def_request;
    IO_OP qk_req = def_request;
    cur_req.index = boardindex;
    //qk_req.index = boardindex;
    
    if (pboard) cur_req.index = 0;
    if (argc <= 1) {
        //{CMainFrame dlg; dlg.DoModal();     }
        showHelp(argv[0]);
        return 1;
    }
    optind = 0;
    while ((opt = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1)
    {
        int argval =  (optarg == NULL )? 0 : atoi(optarg);

        logInfo(L"opt= '%c'[0x%02X] , arg=[%S], option_idex=%d", opt, opt, optarg, option_index);

        switch (opt) {
        case PARAM_INDEX:
            ProcIOReq(pboard, cur_req, io_reqQue);
            if (!isServer) {
                cur_req.index = argval;
                boardindex = argval;
                pboard = CFtdiDriver::GetDriver()->FindBoard(argval, argval, &err);
                if (pboard)
                    pboard->SelMaster(master);
                else
                    SetFaultError(err);
            }
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
        case CMD_DISPLAY:  if (CHKIDX()) ProcIOReq(pboard, cur_req, io_reqQue);  DisplayConn(pboard, cur_req.val.con, boardindex, master); break;
        case CMD_LISTDEV: CFtdiDriver::GetDriver()->ShowDevices(); break;
        case CMD_GUI:     {CMainFrame dlg; dlg.DoModal();     }        break;
        case CMD_VERSION: FdtiPrint("version : %S\r\n", (LPCTSTR)GetSWVersion()); break;
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
        case CMD_SEL_MASTER: 
            if (optarg == NULL) argval = 0;
            if (argval <= 1) {
                if (master != argval) ProcIOReq(pboard, cur_req, io_reqQue);
                master = argval;
                if (pboard) pboard->SelMaster(master);
            }
            break;
        case CMD_RUN_SCRIPT:
            if (!isServer && pboard == NULL) {
                cur_req.index =  boardindex = 0;
                pboard = CFtdiDriver::GetDriver()->FindBoard(0, boardindex, &err);
            }
            if( pboard ) pboard->RunScript(optarg);
            break;
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
            qk_req.index = boardindex;
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
    FdtiPrint("异常已记录到文件：crash.dmp");
    system("pause");
    return EXCEPTION_EXECUTE_HANDLER;
}

CString GetExeName(HMODULE hModule, CString &strpath) {
    if(hModule == NULL )
        hModule = ::GetModuleHandle(nullptr);

    CString sfname;
    TCHAR* psn = sfname.GetBuffer(MAX_PATH);
    DWORD len = ::GetModuleFileName(hModule, psn, MAX_PATH);
    psn[len] = 0;
    sfname.ReleaseBuffer();
    int pos = sfname.ReverseFind('\\');
    strpath = sfname.Left(pos);
    sfname.Delete(0, pos + 1);
    
    return sfname;
}

int getParamVal(const char * str) {
    str += strspn(str, " ");
    int hex = strspn(str, "0xX");
    if (hex != 0) {
        return atox(str, 1);
    }
    else {
        return atoi(str);
    }
    return -1;
}


void show_i2c_tools_help(LPCTSTR exename) {
    logError(L"Usage of Ftdi_i2c_tool :\n");
    FdtiPrint("%S [-j jsonfile] {operat,[chip_addr], reg_addr: [reg_value...]} ...\n", exename);
    FdtiPrint("-j:       Initialize registers by descript a json file.\n");
    FdtiPrint("operat:   [w/write/r/read/s/setbit/c/clrbit]\n");
    FdtiPrint("           w/write:  Direct write value to spec chip 9505 registers\n");
    FdtiPrint("           r/read:   Direct read the spec chip 9505 registers\n");
    FdtiPrint("           s/setbit: Read spec 9505 addr register value and set+write-back one/multi bits\n");
    FdtiPrint("           c/clrbit: Read spec 9505 addr register value and clear+write-back one/multi bits\n");
    FdtiPrint("chip_addr:The chip i2c addresses, If group by square brackets ,each chip separated by commas. Otherwise means single address.\n");
    FdtiPrint("reg_addr: The start address of register\n");
    FdtiPrint("reg_value:The list of register value to be writen, separated by commas,group by square brackets\n");
    FdtiPrint("\nNote: Each group braces defined an i2c operation. There is no limit on i2c operation groups\n");
    FdtiPrint("\n");
}

#include "CPca9505.h"
CHIP_TAB_LIST_T chip_op_arg;
const char* cmdlist[] = {  "write",  "setbit",  "clrbit", "read" };
int get_cmd(const char* str) {
    CStringA scmd = str;
    scmd.Trim();
    for (int i = 0; i < sizeof(cmdlist) / sizeof(const char*); i++) {
        if (scmd[0] == cmdlist[i][0]) {
            if (scmd[1] == 0 || scmd[1] == 0 || strcmp(scmd, cmdlist[i]) == 0) {
                return i;
            }
        }
    }
    return -1;
}
int getCmd(int pos, int &opmod, int argc, char* argv[]) {
    while (pos < argc) {
        int cmd = get_cmd(argv[pos++]);
        if (cmd != -1) {
            opmod = cmd;
            return pos;
        }
    }
    return -1;
}

int getListValue(int &pos, std::vector<BYTE>& list, int argc, char* argv[]) {
    if (pos >= argc) return -1;
    int offset = (argv[pos][0] == '[') ? 1 : 0;
    BOOL gonext = TRUE;
    int count = 0;
    while (pos < argc) {
        int chipid = getParamVal(&argv[pos][offset]);
        if (chipid != 0) {
            gonext = FALSE;
            list.push_back((BYTE)chipid);
            count++;
        }
        if (strchr(&argv[pos][offset], ']') != 0) {
            pos++;
            return count;
        }
        char* pn = strchr(&argv[pos][offset], ',');
        if (pn ) {
            offset = pn - argv[pos] + 1;
            gonext = TRUE;
        }
        else {
            if (++pos >= argc) return count;
            offset = 0;
            if (!gonext) {
                pn = strchr(argv[pos], ',');
                if (pn == nullptr) return count;
                offset = pn - argv[pos] + 1;
                gonext = TRUE;
            }
        }
    }
    return -1;
}

int parseargs(CHIP_TAB_LIST_T &chiplist, int argc, char* argv[]) {
    int optpos = 1;
    while (optpos < argc) { 
        CHIP_TABLE_T chip;
        int i2caddr = 0;
        int opmod = REG_OP_READ;
        int regaddr = -1;
        //char* p = strspn(argv[optpos], " ");
        //cmd
        if (getCmd(optpos, opmod, argc, argv) < 0) return optpos;
        //chip addr,
        if (optpos >= argc || getListValue(optpos, chip.chipid, argc, argv) <= 0)
            return optpos;
        //reg addr,
        if (optpos >= argc || (regaddr = getParamVal(argv[optpos++])) == -1) return optpos;
        //reg value,
        REG_TABLE_T regtbl;
        if (optpos >= argc || getListValue(optpos, regtbl.val, argc, argv) <= 0)
            return optpos;

        regtbl.addr = regaddr;
        regtbl.op = opmod;
        chip.reg_table.push_back(regtbl);
        chiplist.push_back(chip);
    }
    return 0;
}

int load_sub_string(char* buf, const char* ps, const char *key) {
    const char* ptr = strchr(ps, key[0]);
    char strimchr[] = { ' ',key[0],key[1],0 };
    if (ptr == NULL) return -1;
    ptr = ptr + strspn(ptr, strimchr);
    const char * pe = strchr(ptr, key[1]);
    if (pe == nullptr) return -1;
    int len = pe - ptr ;

    memcpy(buf, ptr , len);
    buf[len] = 0;
    return pe - ps + 1;
}

int load_list(std::vector<BYTE>&list, char* str) {
    for (int count = 0; str && *str; count++) {
        str += strspn(str, " ,[");
        int iv = getParamVal(str);
        if (iv == -1) return count;
        str = strchr(str, ',');
        list.push_back((BYTE)iv);
    }
    return 0;
}

char * move_next(char*& ptr, char* &pnext) {
    ptr = pnext + 1 + strspn(pnext + 1, " ,");
    pnext = strchr(ptr, ((ptr[0] == '[') ? ']' : ','));
    if (pnext == nullptr) return ptr;
    *pnext = 0;
    return ptr ;
}

LRESULT load_json(CHIP_TAB_LIST_T& chiplist, const char* cmdline) {
    const char* pjson = strstr(cmdline, "-j");
    if (pjson) {
        pjson += strspn(pjson, " -j");
        const char* pe = pjson + strcspn(pjson, " ,");
        char jname[256];
        memcpy(jname, pjson, pe - pjson);
        jname[pe - pjson] = 0;
        return CPca9505::load_script(jname, chiplist);
    }
    return S_OK;
}
////test.exe {w,0x26,0,[1,3,2]}, {s,0x3}
int parseOneLine(CHIP_TAB_LIST_T& chiplist, const char* cmdline) {
    char str[256];
    int offset = 0;
    int len = 0;
    LRESULT ret = S_OK;
    if ((ret = load_json(chiplist, cmdline)) != S_OK) return ret;
    while ((len = load_sub_string(str, &cmdline[offset], "{}") ) > 0 ) {
        offset += len;

        CHIP_TABLE_T chip;
        REG_TABLE_T reg;
        char* ptr = str;
        char* pnext = strchr(ptr, ',') ;
        *pnext = '\0';
        int icmd = get_cmd(ptr);
        if (icmd == -1) return (chiplist.size())? S_OK : ERROR_INVALID_COMMAND_LINE;

        load_list(chip.chipid, move_next(ptr, pnext));

        if (pnext == nullptr) return ERROR_INVALID_COMMAND_LINE;
        move_next(ptr, pnext);
        reg.addr = getParamVal(ptr);

        if (icmd != REG_OP_READ) {
            if (pnext == nullptr) return ERROR_INVALID_COMMAND_LINE;
            load_list(reg.val, move_next(ptr, pnext));
        }

        reg.op = icmd; 
        chip.reg_table.push_back(reg);
        chiplist.push_back(chip);
    }
    return S_OK;
}

int connectpipe(HANDLE &pipe ) {
    //cout << __func__ << "/" << __LINE__ << endl;
    //printf("Connect to Server ...\n");
    if (WaitNamedPipe(PRINT_PIPE_NAME, 3000) == FALSE) {
        printf("Reset and connect to server...\n");
        SendBoardMessage("pipe_reset\n0\n");
        if (WaitNamedPipe(PRINT_PIPE_NAME, 10000) == FALSE) {
            printf("wait pipe failed!/n");
            return -1;
        }
    }
    
    pipe = CreateFile(PRINT_PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (pipe == INVALID_HANDLE_VALUE) pipe = 0;
    if (pipe == 0) {
        printf("can not ope pipe!");
        return -1;
    }
    //cout << __func__ << "/" << __LINE__ << endl;
    return S_OK;
}

void localprint(void* param) {

    HANDLE* phPipe = (HANDLE*)param;
    if (*phPipe == 0) return;
    char buff[1025];
    DWORD rxb = 0;
    ;
    while (ReadFile(*phPipe, buff, 1024, &rxb, NULL) && rxb) {
        buff[rxb] = 0;
        printf("%s", buff);
    }
    CloseHandle(*phPipe);
    *phPipe = 0;
    //cout << __func__ << "/" << __LINE__ << endl;
}
HANDLE m_hEvtMsg;
HWND  m_hMsgWnd = NULL;
HANDLE hpipe = 0;
INT conn = 0;

int GetConnectState() {
    conn = ::SendMessage(m_hMsgWnd, WM_QUERY_CONNECT, 0, 0);
    return conn;
}

INT connect_server(DWORD nTimeOut) {
    DWORD64 ticktimeout = ::GetTickCount64() + nTimeOut + 1;
    while(::GetTickCount64() < ticktimeout){
        //cout << __func__ << "/" << __LINE__ << endl;
        if ((m_hMsgWnd = ::FindWindow(NULL, NAME_WND_MSG_MAIN)) != NULL) {
            //cout << "find window and event [";
            //cout << __func__ << "/" << __LINE__ << endl;
            do{
                if (GetConnectState() >= SCAN_DONE) break;
                //printf("conn query ret:%d\n", conn);
                Sleep(50);
            } while (::GetTickCount64() < ticktimeout );
            if( hpipe == 0 && connectpipe(hpipe) == S_OK)
                _beginthread(localprint, 1024, &hpipe);
            //cout << __func__ << "/" << __LINE__ << endl;
            return hpipe ==0 ? ERROR_PIPE_BUSY : S_OK;
        }
        DWORD sleeptick = (nTimeOut > 100) ? 100 : nTimeOut;
        Sleep(sleeptick);
    }
    //cout << __func__ << "/" << __LINE__ << endl;
    return ERROR_TIMEOUT;
}

LRESULT SendBoardMessage(const char* msg) {
    int slen = strlen(msg) + 1;
    char* pmsg = new char[slen];
    strcpy(pmsg, msg);
    COPYDATASTRUCT cds;
    cds.dwData = slen;
    cds.cbData = slen;
    cds.lpData = pmsg;
    LRESULT val = ::SendMessage(m_hMsgWnd, WM_COPYDATA, 0, (WPARAM)&cds);
    delete[] pmsg;
    return val;
}



int backgrd_server(CString &sfname, CString &spath) {
    if (connect_server(NMPWAIT_NOWAIT) == ERROR_TIMEOUT ) {
        RunProc(sfname, L"server run", spath, FALSE, TRUE);
    }
    if ( (hpipe) || (connect_server(60000) == S_OK)) {
        return S_OK;
    }
    FdtiPrint("Connect Server failed!\n");
    return -1;
}

#include<conio.h>
#include "CMainDlg.h"
CStringA sExeName;

int main(int argc, char* argv[])
{
   //test.exe {w,0x26,0,[1,3,2]}, {s,0x3}
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);
    

    HMODULE hModule = ::GetModuleHandle(nullptr);
    if (hModule == nullptr)
    {   // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        return 1;
    }

    // 初始化 MFC 并在失败时显示错误
    if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
    {   // TODO: 在此处为应用程序的行为编写代码。
        wprintf(L"错误: MFC 初始化失败\n");
        return 1;
    }
    CString spath, sfname;
    sfname = GetExeName(hModule, spath);
    sExeName = qUnc2Utf((LPCTSTR)sfname);
    LRESULT ret = S_OK;
    const char* p = ::GetCommandLineA();
    const char* pp = NULL;
    while ((pp = strchr(p, '\\')) != nullptr)  p = pp + 1;
    pp = p + strcspn(p, " \t");
    pp += strspn(pp, " \t");

    if (strstr(pp, "keyval") != nullptr) {
        int key = _getch();
        return key;
    }
    else if (strstr(pp, "server") != nullptr ) {
        const char* ps = strstr(pp, "server");
        if (ps != nullptr) {
            ps += strspn(ps + 6, " \t") + 6;
            if (strstr(ps, "run") == ps) {
                CMainDlg dlg;
                return (dlg.DoModal() == IDOK) ? 0 : -1;
            }
            else if (strstr(ps, "stop") == ps) {
                if (connect_server(1000) == S_OK) {
                    SendBoardMessage("stop\n stop");
                    return 0;
                }
                return -1;
            }
            else if (strstr(ps, "start") == ps) {
                backgrd_server(sfname, spath);
                return 0;
            }
            else {
                show_i2c_tools_help(sfname);
                return -1;
            }
        }
    }
    else {
        backgrd_server(sfname, spath);
    }

#if DEBUG_ARG
    argc = sizeof(testarg) / sizeof(char*);
    argv = testarg;

    p = "\\mtk_USBTPV\\USBTPV\\MtkUsbTPV\\Debug\\Ftdi_i2c_tool.exe  -j test.json "\
        "{w,[0x0,0x1],0x18,[0,0,0,0,0]},"\
        "{w,[0x0,0x1],0x8,[0x20, 0x00, 0, 0, 0]} "\
        "{s,0x0,0x8,[0x40, 0x80, 0x14, 0x10, 0x10]}"\
        "{c,0x0,0x8,[0x20, 0x80, 0x10, 0x00, 0x10]}"\
        "{r,0x0,0x8}";
#endif
    
    CHIP_TAB_LIST_T chiplist;
    if (strstr(p, "reset") != nullptr) {
        SendBoardMessage("reset\n0\n");
        CancelIo(hpipe);
        Sleep(50);
        return 0;
    }
    if (strchr(p, '{') != nullptr || strstr(p, "-j") != nullptr) {
        logInfo(L"i2c param %S\n", p);
        if ((ret = parseOneLine(chiplist, p)) != S_OK) {
            wprintf(L"Fault error[%d]: (%s)", ret, (LPCTSTR)ErrorString(ret));
            show_i2c_tools_help(sfname);
            CancelIo(hpipe);
            Sleep(50);
            return -1;
        }
        

        std::string str;
        str = "run\n";
        str += pp;
        SendBoardMessage(str.c_str());

        CancelIo(hpipe);
        Sleep(50);
        /*LRESULT err;
        InitLog();
        CFtdiDriver  drvFdti;
        drvFdti.Scan(FALSE);
        drvFdti.MountDevices();
        CTpvBoard* pboard = NULL;

        pboard = drvFdti.FindBoard(0, 0, &err);
        if (pboard) return pboard->Run(&chiplist);*/
        return -1;
    }
    else  {
        //::AfxInitRichEdit2();
        //int ret = server_entry(NULL, argc, argv, FALSE);

        std::string sarg = "step\n";
        for (int i = 1; i < argc; i++) {
            sarg += argv[i];
            sarg += "\n";
        }
        SendBoardMessage(sarg.c_str());
        CancelIo(hpipe);
        Sleep(50);
        return ret;
    }
    
}

