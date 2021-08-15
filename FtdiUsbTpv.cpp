// FtdiUsbTpv.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "FtdiUsbTpv.h"
#include "CMainFrame.h"

#include "getopt.h"
#include "CFT2232.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;


void showHelp(char * app)
{
    printf("usage %s [-h] [-i INDEX} [-c CON] [-o HOME] [-k KCOL] [-p PWRKEY] [-r RAW_VALUE | -s | -d | -l | u | v]\r\n", app);
    printf("optional arguments:\r\n");
    printf("  -h, --help              Show this help message and exit\r\n");
    printf("  -i INDEX, --interface INDEX\r\n");
    printf("                          Specify the interface to be controlled (default 0)\r\n");
    printf("  -c CON, --con CON       Specify CON0~CON4 connector to be controlled, CON5 means all(default 0)\r\n");
    printf("  -o HOME, --home HOME    Specify HOMEKEY output of CON(0:LOW, 1:HIGH)\r\n");
    printf("  -k KCOL, --kcol KCOL    Specify KCOL0 output of CON(0:LOW, 1:HIGH)\r\n");
    printf("  -p PWRKEY, --pwrkey PWRKEY\r\n");
    printf("                          Specify  PWRKEY output of CON(0:LOW, 1:HIGH)\r\n");
    printf("  -r RAW_VALUE, --raw RAW_VALUE\r\n");
    printf("                          Specify all CONs to a specify value, each CON has 3 bits for\r\n");
    printf("                          HOOMEKEY_KCOL0_PWEKEY(LSB)\r\n");
    printf("  -s, --set               Specify the output of the selected keys on CON\r\n");
    printf("  -d, --display           Show the status of CONs\r\n");
    printf("  -l, --list              Show connected FT2322 devices\r\n");
    printf("  -u, --gui               Show the dialog of FT2322 devices control\r\n");
    printf("  -v, --version           Show version information\r\n");
}


int main(int argc, char** argv)
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {

            int opt;
            int digit_optind = 0;
            int option_index = 0;
            const char* optstring = "vsdluhi:c:o:k:p:r";
            static struct option long_options[] = {
                {"index",  required_argument, NULL, 'i'},
                {"con",    required_argument, NULL, 'c'},
                {"home" ,  required_argument, NULL, 'o'},
                {"kcol",   required_argument, NULL, 'k'},
                {"pwrkey", required_argument, NULL, 'p'},
                {"raw",    required_argument, NULL, 'r'},
                {"set",    required_argument, NULL, 's'},
                {"display",no_argument,       NULL, 'd'},
                {"list",   no_argument,       NULL, 'l'},
                {"gui",    no_argument,       NULL, 'u'},
                {"version",no_argument,       NULL, 'v'},
                {"help",   no_argument,       NULL, 'h'},
                {0, 0, 0, 0}
            };

            
            while ((opt = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1)
            {
                switch (opt) {
                case 'h':
                    showHelp(argv[0]);
                    break;
                case 'v':
                    printf("version : 1.0.0.1");
                    break;
                case 'l':
                    CFT2232::Scan();
                    break;
                case 'u': {
                    CMainFrame dlg;
                    dlg.DoModal();
                    }
                    break;
                case 's':
                    break;
                }
                //printf("opt = %c\n", opt);
                //printf("optarg = %s\n", optarg);
                //printf("optind = %d\n", optind);
                //printf("argv[optind - 1] = %s\n", argv[optind - 1]);
                //printf("option_index = %d\n", option_index);
            }

            
            //dlg.DoModal();
        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
