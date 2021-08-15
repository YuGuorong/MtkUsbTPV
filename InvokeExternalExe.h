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
  
// ���õĳ�����;�����Ϣ�󣬻���ô˻ص�  
// pUserData : CInvokeExternalExe::SetUserData()���õ�ֵ  
// pButOutput : ���õĳ��������Ϣ��Buffer�����ܴ����ANSI��Ҳ���ܴ����UNICODE  
// dwOutputSize : ���õĳ��������ϢBuffer��С  
typedef BOOL (__stdcall * LPExecuteOutputCB)(  
    const void * const pUserData,   
    const CHAR * const pButOutput,   
    DWORD dwOutputSize  
);  
  
// �첽����ʱ��ִ�������õ�������ص�  
// pUserData : CInvokeExternalExe::SetUserData()���õ�ֵ  
// bInvodeSuccess : �����Ƿ�ɹ�  
// dwExitCode : �����óɹ�������Ӧ�ó�����˳�����  
// strErrorMsg : ������ʧ�ܣ�����ʧ����Ϣ  
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
  
    // �첽����ʱ��Ҫ����������Ϣ  
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

  
    // ͬ������  
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
  
    // ͬ������ʱ����ֵ��ʾ����Ӧ�ó����Ƿ�ɹ�  
    // �쳣���ñ�ʾ��������Ƿ�ɹ�  
    BOOL Invoke(  
        IN LPCTSTR lpApplicationName,       // Ӧ�ó�����   
        IN LPCTSTR lpCommandLine,            // ����  
        IN const BOOL bSynchronousInvoke,   // �Ƿ�ͬ������  
        IN const DWORD dwMilliseconds,      // ��ʱʱ������λ���룬0��ʾ����ʱ  
        IN const WORD wShowWindow,          // �Ƿ���ʾ���ڣ��ɴ�SW_...ϵ�еĺ�  
        OUT DWORD &dwExitCode,              // ���óɹ���Ӧ�ó�����˳�����  
        OUT tstring &strErrorMsg            // ������ʧ�ܣ�����ʧ����Ϣ  
        );  
  
    // ȡ������  
    void CancelInvoke();  
};  
  
#endif // !defined(AFX_INVOKEEXTERNALEXE_H__9BA72D3F_E9AD_4054_9F51_7C5281B2D2F2__INCLUDED_)  