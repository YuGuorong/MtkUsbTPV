// InvokeExternalExe.h: interface for the CInvokeExternalExe class.  
//  
//////////////////////////////////////////////////////////////////////  
  
#if !defined(AFX_INVOKEEXTERNALEXE_H__9BA72D3F_E9AD_4054_9F51_7C5281B2D2F2__INCLUDED_)  
#define AFX_INVOKEEXTERNALEXE_H__9BA72D3F_E9AD_4054_9F51_7C5281B2D2F2__INCLUDED_  
  
#if _MSC_VER > 1000  
#pragma once  
#endif // _MSC_VER > 1000  
  
#include <Windows.h>  
#include <string>  
#include <tchar.h>  
  
#if (defined UNICODE) || (defined _UNICODE)  
#define tstring std::wstring  
#else  
#define tstring std::string  
#endif  
  
// 调用的程序中途输出信息后，会调用此回调  
// pUserData : CInvokeExternalExe::SetUserData()设置的值  
// pButOutput : 调用的程序输出信息的Buffer，可能存的是ANSI，也可能存的是UNICODE  
// dwOutputSize : 调用的程序输出信息Buffer大小  
typedef BOOL (__stdcall * LPExecuteOutputCB)(  
    const void * const pUserData,   
    const CHAR * const pButOutput,   
    DWORD dwOutputSize  
);  
  
// 异步调用时，执行完程序得到结果后会回调  
// pUserData : CInvokeExternalExe::SetUserData()设置的值  
// bInvodeSuccess : 调用是否成功  
// dwExitCode : 若调用成功，保存应用程序的退出代码  
// strErrorMsg : 若调用失败，返回失败信息  
typedef BOOL (__stdcall * LPAsyncInvokeResultCB)(  
    const void * const pUserData,   
    const BOOL bInvodeSuccess,  
    const DWORD dwExitCode,  
    const tstring strErrorMsg  
);  
  
class CInvokeExternalExe    
{  
private:  
    void *m_pUserData;  
    LPExecuteOutputCB m_pExecuteOutputCB;  
    LPAsyncInvokeResultCB m_pAyncInvokeResultCB;  
    HANDLE m_hChildStd_IN_Rd;  
    HANDLE m_hChildStd_IN_Wr;  
    HANDLE m_hChildStd_OUT_Rd;  
    HANDLE m_hChildStd_OUT_Wr;  
    HANDLE m_hProcessHandle;  
    HANDLE m_hOutputThreadHandle;  
    BOOL m_bInvokeFinish;  
    HANDLE m_hAsyncInvokeThreadHandle;  
    BOOL m_bIsInvoking;  
  
    // 异步调用时需要保存以下信息  
    BOOL m_bApplicationNameIsNull;  
    tstring m_strApplicationName;  
    BOOL m_bCommandLineIsNull;  
    tstring m_strCommandLine;  
    DWORD m_dwMilliseconds;  
    WORD m_wShowWindow;  
  
    CRITICAL_SECTION m_csLock;  
  
private:  
    tstring GetLastErrorMsg(LPCTSTR lpszFunction, const DWORD dwLastError);  
    void CloseAllHandles();  
  
    unsigned ReadOutput();  
    static unsigned __stdcall ReadOutputThreadFunc(void* pArguments);  
  
    unsigned AsyncInvoke();  
    static unsigned __stdcall AsyncInvokeThreadFunc(void* pArguments);  
	void InternalInit();

  
    // 同步调用  
    BOOL Invoke(  
        IN LPCTSTR lpApplicationName,  
        IN LPCTSTR lpCommandLine,
        IN const DWORD dwMilliseconds,  
        IN const WORD wShowWindow,  
        OUT DWORD &dwExitCode,  
        OUT tstring &strErrorMsg  
        );  
  
public:  
    CInvokeExternalExe();  
	CInvokeExternalExe(LPExecuteOutputCB pExecuteOutputCB, void * userData);
    virtual ~CInvokeExternalExe();  
  
public:  
    void SetUserData(void *pUserData)  
    {  
        m_pUserData = pUserData;  
    }  
  
    void SetExecuteOutputCB(LPExecuteOutputCB pExecuteOutputCB)  
    {  
        m_pExecuteOutputCB = pExecuteOutputCB;  
    }  
  
    void SetAsyncInvokeResultCB(LPAsyncInvokeResultCB pAsyncInvokeResultCB)  
    {  
        m_pAyncInvokeResultCB = pAsyncInvokeResultCB;  
    }  
  
    // 同步调用时返回值表示调用应用程序是否成功  
    // 异常调用表示发起调用是否成功  
    BOOL Invoke(  
        IN LPCTSTR lpApplicationName,       // 应用程序名   
        IN LPCTSTR lpCommandLine,            // 参数  
        IN const BOOL bSynchronousInvoke,   // 是否同步调用  
        IN const DWORD dwMilliseconds,      // 超时时长，单位毫秒，0表示不超时  
        IN const WORD wShowWindow,          // 是否显示窗口，可传SW_...系列的宏  
        OUT DWORD &dwExitCode,              // 调用成功后，应用程序的退出代码  
        OUT tstring &strErrorMsg            // 若调用失败，返回失败信息  
        );  
  
    // 取消调用  
    void CancelInvoke();  
};  
  
#endif // !defined(AFX_INVOKEEXTERNALEXE_H__9BA72D3F_E9AD_4054_9F51_7C5281B2D2F2__INCLUDED_)  