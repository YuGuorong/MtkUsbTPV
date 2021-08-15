// InvokeExternalExe.cpp: implementation of the CInvokeExternalExe class.  
//  
//////////////////////////////////////////////////////////////////////  
  
#include "pch.h"  
#include "InvokeExternalExe.h"  
#include <process.h>  
#include <strsafe.h>  
  
//////////////////////////////////////////////////////////////////////  
// Construction/Destruction  
//////////////////////////////////////////////////////////////////////  
  
#define BUFSIZE 4096   
  
class CAutoLock  
{  
private:  
    LPCRITICAL_SECTION m_pcsLcok;  
      
public:  
    CAutoLock(LPCRITICAL_SECTION pcsLcok)  
    {  
        m_pcsLcok = pcsLcok;  
        if (m_pcsLcok)  
        {  
            EnterCriticalSection(m_pcsLcok);  
        }  
    }  
      
    ~CAutoLock()  
    {  
        if (m_pcsLcok)  
        {  
            LeaveCriticalSection(m_pcsLcok);  
            m_pcsLcok = NULL;  
        }  
    }  
};  


void CInvokeExternalExe::InternalInit()
{
	m_pUserData = NULL;
	m_pExecuteOutputCB = NULL;
	m_pAyncInvokeResultCB = NULL;
	m_hChildStd_IN_Rd = NULL;
	m_hChildStd_IN_Wr = NULL;
	m_hChildStd_OUT_Rd = NULL;
	m_hChildStd_OUT_Wr = NULL;
	m_hProcessHandle = NULL;
	m_hOutputThreadHandle = NULL;
	m_bInvokeFinish = FALSE;
	m_hAsyncInvokeThreadHandle = NULL;
	m_bIsInvoking = FALSE;

	m_bApplicationNameIsNull = TRUE;
	m_bCommandLineIsNull = TRUE;
	m_dwMilliseconds = 0;
	m_wShowWindow = SW_NORMAL;

	InitializeCriticalSection(&m_csLock);
}


CInvokeExternalExe::CInvokeExternalExe(LPExecuteOutputCB pExecuteOutputCB, void * userData)
{
	InternalInit();
	m_pExecuteOutputCB = pExecuteOutputCB;
	m_pUserData = userData;
}

  
CInvokeExternalExe::CInvokeExternalExe()  
{  
	InternalInit();
}  
  
CInvokeExternalExe::~CInvokeExternalExe()  
{  
    CancelInvoke();  
    DeleteCriticalSection(&m_csLock);  
}  
  
unsigned CInvokeExternalExe::ReadOutput()  
{  
    unsigned usRet = 1;  
  
    DWORD dwTotalBytesAvail = 0;  
    DWORD dwRead = 0;  
    CHAR chBuf[BUFSIZE] = {0};  
    BOOL bSuccess = FALSE;  
  
    while (TRUE)  
    {  
        if (!PeekNamedPipe(m_hChildStd_OUT_Rd, NULL, 0, NULL, &dwTotalBytesAvail, NULL))  
        {  
            usRet = GetLastError();  
            break;  
        }  
        if (0 == dwTotalBytesAvail)  
        {  
            if (m_bInvokeFinish)  
            {  
                usRet = 0;  
                break;  
            }  
            else  
            {  
                Sleep(1);  
                continue;  
            }  
        }  
  
        bSuccess = ReadFile(m_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
        if (!bSuccess || dwRead == 0)  
        {  
            usRet = GetLastError();  
            break;  
        }  
        if (dwRead < BUFSIZE)  
        {  
            chBuf[dwRead] = 0;  
            if (IsTextUnicode(chBuf, dwRead, NULL) && dwRead + 1 < BUFSIZE)  
            {  
                chBuf[dwRead + 1] = 0;  
            }  
        }  
        if (m_pExecuteOutputCB)  
        {  
            m_pExecuteOutputCB(m_pUserData, chBuf, dwRead);  
        }  
    }  
      
    return usRet;  
}  
  
unsigned CInvokeExternalExe::ReadOutputThreadFunc(void* pArguments)  
{  
    CInvokeExternalExe *pThis = (CInvokeExternalExe *)pArguments;  
    if (NULL == pThis)  
    {  
        _endthreadex (1);  
        return 1;  
    }  
  
    unsigned usRet = pThis->ReadOutput();  
    _endthreadex (usRet);  
    return usRet;  
}  
  
#pragma warning(disable:4996) //全部关掉

unsigned CInvokeExternalExe::AsyncInvoke()  
{  
    unsigned usRet = 1;  
      
    DWORD dwExitCode = 0;  
    tstring strErrorMsg;  
  
    PTCHAR lpCommandLine = NULL;  
    if (!m_bCommandLineIsNull)  
    {  
        lpCommandLine = new TCHAR[m_strCommandLine.length() + 1];  
        m_strCommandLine.copy(lpCommandLine, m_strCommandLine.length(), 0);  
        lpCommandLine[m_strCommandLine.length()] = _T('\0');  
    }  
      
    BOOL bInvokeSuccess = Invoke(  
        m_bApplicationNameIsNull ? NULL : m_strApplicationName.c_str(),  
        lpCommandLine,  
        m_dwMilliseconds,  
        m_wShowWindow,  
        dwExitCode,  
        strErrorMsg  
        );  
    if (lpCommandLine)  
    {  
        delete [] lpCommandLine;  
        lpCommandLine = NULL;  
    }  
    if (m_pAyncInvokeResultCB)  
    {  
        m_pAyncInvokeResultCB(m_pUserData, bInvokeSuccess, dwExitCode, strErrorMsg);  
    }  
    m_bIsInvoking = FALSE;  
  
    return usRet;  
}  
  
unsigned CInvokeExternalExe::AsyncInvokeThreadFunc(void* pArguments)  
{  
    CInvokeExternalExe *pThis = (CInvokeExternalExe *)pArguments;  
    if (NULL == pThis)  
    {  
        _endthreadex (1);  
        return 1;  
    }  
      
    unsigned usRet = pThis->AsyncInvoke();  
    _endthreadex (usRet);  
    return usRet;  
}

  
BOOL CInvokeExternalExe::Invoke(  
                                IN LPCTSTR lpApplicationName,  
                                IN LPCTSTR lpCommandLine,  
                                IN const BOOL bSynchronousInvoke,  
                                IN const DWORD dwMilliseconds,  
                                IN const WORD wShowWindow,  
                                OUT DWORD &dwExitCode,  
                                OUT tstring &strErrorMsg)  
{  
    {  
        CAutoLock autolock(&m_csLock);  
        if (m_bIsInvoking)  
        {  
            strErrorMsg = _T("In executing.");  
            return FALSE;  
        }  
        m_bIsInvoking = TRUE;  
    }  
  
  
    // 同步调用  
    if (bSynchronousInvoke)  
    {  
        BOOL bRet = Invoke(  
            lpApplicationName,  
            lpCommandLine,  
            dwMilliseconds,  
            wShowWindow,  
            dwExitCode,  
            strErrorMsg  
            );  
        m_bIsInvoking = FALSE;  
        return bRet;  
    }  
  
    // 异步调用  
    if (lpApplicationName)  
    {  
        m_bApplicationNameIsNull = FALSE;  
        m_strApplicationName = lpApplicationName;  
    }  
    else  
    {  
        m_bApplicationNameIsNull = TRUE;  
    }  
    if (lpCommandLine)  
    {  
        m_bCommandLineIsNull = FALSE;  
        m_strCommandLine = lpCommandLine;  
    }  
    else  
    {  
        m_bCommandLineIsNull = TRUE;  
    }  
    m_dwMilliseconds = dwMilliseconds;  
    m_wShowWindow = wShowWindow;  
  
    unsigned threadID = 0;  
    m_hAsyncInvokeThreadHandle =   
        (HANDLE)_beginthreadex(NULL, 0, &AsyncInvokeThreadFunc, this, 0, &threadID);  
    if (NULL == m_hAsyncInvokeThreadHandle)  
    {  
        strErrorMsg = GetLastErrorMsg(_T("_beginthreadex"), GetLastError());  
        return FALSE;  
    }  
  
    return TRUE;  
}  
  
BOOL CInvokeExternalExe::Invoke(  
                                IN LPCTSTR lpApplicationName,  
                                IN LPCTSTR lpCommandLine,
                                IN const DWORD dwMilliseconds,  
                                IN const WORD wShowWindow,  
                                OUT DWORD &dwExitCode,  
                                OUT tstring &strErrorMsg)  
{  
    m_bInvokeFinish = FALSE;  
  
    if (NULL == lpApplicationName && NULL == lpCommandLine)  
    {  
        strErrorMsg = _T("Both lpApplicationName and lpCommandLine are NULL.");  
        CloseAllHandles();  
        return FALSE;  
    }  
  
    SECURITY_ATTRIBUTES saAttr;   
      
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);   
    saAttr.bInheritHandle = TRUE;  
    saAttr.lpSecurityDescriptor = NULL;   
      
    if (!CreatePipe(&m_hChildStd_OUT_Rd, &m_hChildStd_OUT_Wr, &saAttr, 0))  
    {  
        strErrorMsg = GetLastErrorMsg(_T("StdoutRd CreatePipe"), GetLastError());  
        CloseAllHandles();  
        return FALSE;  
    }  
  
    if (!SetHandleInformation(m_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))  
    {  
        strErrorMsg = GetLastErrorMsg(_T("Stdout SetHandleInformation"), GetLastError());  
        CloseAllHandles();  
        return FALSE;  
    }  
  
    if (!CreatePipe(&m_hChildStd_IN_Rd, &m_hChildStd_IN_Wr, &saAttr, 0))  
    {  
        strErrorMsg = GetLastErrorMsg(_T("Stdin CreatePipe"), GetLastError());  
        CloseAllHandles();  
        return FALSE;  
    }  
      
    if (!SetHandleInformation(m_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))  
    {  
        strErrorMsg = GetLastErrorMsg(_T("Stdin SetHandleInformation"), GetLastError());  
        CloseAllHandles();  
        return FALSE;  
    }  
  
    unsigned threadID = 0;  
    m_hOutputThreadHandle =   
        (HANDLE)_beginthreadex(NULL, 0, &ReadOutputThreadFunc, this, 0, &threadID);  
    if (NULL == m_hOutputThreadHandle)  
    {  
        strErrorMsg = GetLastErrorMsg(_T("_beginthreadex"), GetLastError());  
        CloseAllHandles();  
        return FALSE;  
    }  
	TCHAR scmdline[1024];
	_tcsnccpy_s(scmdline, lpCommandLine, 1024);
      
    PROCESS_INFORMATION piProcInfo;   
    STARTUPINFO siStartInfo;  
    BOOL bSuccess = FALSE;   
      
    ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );  
          
    ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );  
    siStartInfo.cb = sizeof(STARTUPINFO);   
    siStartInfo.hStdError = m_hChildStd_OUT_Wr;  
    siStartInfo.hStdOutput = m_hChildStd_OUT_Wr;  
    siStartInfo.hStdInput = m_hChildStd_IN_Rd;  
    siStartInfo.wShowWindow = wShowWindow;  
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
      
    bSuccess = CreateProcess(lpApplicationName,   
		scmdline,	   // command line     //NOT const parameter ,would rewrite itself.
        NULL,          // process security attributes   
        NULL,          // primary thread security attributes   
        TRUE,          // handles are inherited   
        0,             // creation flags   
        NULL,          // use parent's environment   
        NULL,          // use parent's current directory   
        &siStartInfo,  // STARTUPINFO pointer   
        &piProcInfo);  // receives PROCESS_INFORMATION   
    if (!bSuccess)  
    {  
        strErrorMsg = GetLastErrorMsg(_T("CreateProcess"), GetLastError());  
        CloseAllHandles();  
        return FALSE;  
    }  
    else  
    {  
        m_hProcessHandle = piProcInfo.hProcess;  
        CloseHandle(piProcInfo.hThread);  
    }  
  
    switch (WaitForSingleObject(m_hProcessHandle, 0 == dwMilliseconds ? INFINITE : dwMilliseconds))  
    {  
    case WAIT_OBJECT_0:  
        {  
            if (GetExitCodeProcess(m_hProcessHandle, &dwExitCode))  
            {  
                m_bInvokeFinish = TRUE;  
                WaitForSingleObject(m_hOutputThreadHandle, INFINITE);  
                CloseAllHandles();  
                return TRUE;  
            }  
            else  
            {  
                strErrorMsg = GetLastErrorMsg(_T("GetExitCodeProcess"), GetLastError());  
                CloseAllHandles();  
                return FALSE;  
            }  
        }  
        break;  
    case WAIT_TIMEOUT:  
        {  
            if (TerminateProcess(m_hProcessHandle, 1))  
            {  
                WaitForSingleObject(m_hProcessHandle, INFINITE);  
            }  
  
            strErrorMsg = GetLastErrorMsg(_T("WaitForSingleObject"), ERROR_TIMEOUT);  
            CloseAllHandles();  
            return FALSE;  
        }  
        break;  
    default:  
        {  
            if (TerminateProcess(m_hProcessHandle, 1))  
            {  
                WaitForSingleObject(m_hProcessHandle, INFINITE);  
            }  
              
            strErrorMsg = GetLastErrorMsg(_T("WaitForSingleObject"), GetLastError());  
            CloseAllHandles();  
            return FALSE;  
        }  
        break;  
    }  
      
    return TRUE;  
}  
  
tstring CInvokeExternalExe::GetLastErrorMsg(LPCTSTR lpszFunction, const DWORD dwLastError)  
{  
    LPVOID lpMsgBuf = NULL;  
    LPVOID lpDisplayBuf = NULL;  
      
    FormatMessage(  
        FORMAT_MESSAGE_ALLOCATE_BUFFER |   
        FORMAT_MESSAGE_FROM_SYSTEM |  
        FORMAT_MESSAGE_IGNORE_INSERTS,  
        NULL,  
        dwLastError,  
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  
        (LPTSTR) &lpMsgBuf,  
        0, NULL );  
  
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,   
        (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR));   
    StringCchPrintf((LPTSTR)lpDisplayBuf,   
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),  
        TEXT("%s failed with error %d: %s"),   
        lpszFunction, dwLastError, lpMsgBuf);   
  
    tstring strLastError = (LPTSTR)lpDisplayBuf;  
      
    LocalFree(lpMsgBuf);  
    LocalFree(lpDisplayBuf);  
  
    return strLastError;  
}  
  
void CInvokeExternalExe::CloseAllHandles()  
{  
    CAutoLock autolock(&m_csLock);  
  
    if (m_hChildStd_IN_Wr)  
    {  
        CloseHandle(m_hChildStd_IN_Wr);  
        m_hChildStd_IN_Wr = NULL;  
    }  
    if (m_hChildStd_IN_Rd)  
    {  
        CloseHandle(m_hChildStd_IN_Rd);  
        m_hChildStd_IN_Rd = NULL;  
    }  
    if (m_hChildStd_OUT_Wr)  
    {  
        CloseHandle(m_hChildStd_OUT_Wr);  
        m_hChildStd_OUT_Wr = NULL;  
    }  
    if (m_hChildStd_OUT_Rd)  
    {  
        CloseHandle(m_hChildStd_OUT_Rd);  
        m_hChildStd_OUT_Rd = NULL;  
    }  
    if (m_hOutputThreadHandle)  
    {  
        CloseHandle(m_hOutputThreadHandle);  
        m_hOutputThreadHandle = NULL;  
    }  
    if (m_hProcessHandle)  
    {  
        CloseHandle(m_hProcessHandle);  
        m_hProcessHandle = NULL;  
    }  
    if (m_hAsyncInvokeThreadHandle)  
    {  
        CloseHandle(m_hAsyncInvokeThreadHandle);  
        m_hAsyncInvokeThreadHandle = NULL;  
    }  
}  
  
void CInvokeExternalExe::CancelInvoke()  
{  
    CAutoLock autolock(&m_csLock);  
  
    if (m_hOutputThreadHandle)  
    {  
        TerminateThread(m_hOutputThreadHandle, 1);  
    }  
  
    if (m_hProcessHandle)  
    {  
        TerminateProcess(m_hProcessHandle, 1);  
    }  
  
    if (m_hAsyncInvokeThreadHandle)  
    {  
        TerminateThread(m_hAsyncInvokeThreadHandle, 1);  
        if (m_pAyncInvokeResultCB)  
        {  
            m_pAyncInvokeResultCB(m_pUserData, FALSE, 1, _T("Canceled Asynchronous Invoke."));  
        }  
    }  
  
    CloseAllHandles();  
  
    m_bIsInvoking = FALSE;  
}  