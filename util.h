#pragma once

#include "resource.h"

#ifndef __PROCESS_MUTEX_H__
#define __PROCESS_MUTEX_H__

#ifdef WIN32
#include <Windows.h>
#endif

#ifdef linux
#include <unistd.h>
#include <semaphore.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <memory.h>
#endif
#include <sal.h>
class CPMutex
{
public:
	/* 默认创建匿名的互斥 */
	CPMutex(const TCHAR* name = NULL);
	virtual ~CPMutex();

	BOOL Lock();
	BOOL UnLock();

private:

#ifdef WIN32
	void* m_pMutex;
#endif

#ifdef linux
	sem_t* m_pSem;
#endif
	TCHAR m_cMutexName[30];
};


extern CString strCurPath;
void AppEnvInit();
int BaseAppInit();
void GetCurPath(CString &strPath);
int RunCmd(LPCTSTR scmd, CString &sresult, LPCTSTR sdir = NULL, BOOL bshow = FALSE);
int RunCmd(LPCTSTR scmd, TCHAR * sresult, LPCTSTR sdir = NULL, BOOL bshow = FALSE);
BOOL IsPathExist(const CString & csPath);
BOOL GetFileSize(LPCTSTR  sfile, DWORD &flen);
//Utf-8 to Wide char(unicode), not use too much stack 
void Unc2Utf(LPCTSTR   wstr, char  * bstr, int lenw, int lens);
//Wide char(unicode) to Utf-8 , not use too much stack
void Utf2Unc(const char  * bstr, WCHAR  * wstr, int lens, int lenw);
//Wide char(unicode) to Utf-8 , not use too much stack
void QUtf2Unc(LPCSTR  astr, CString &wstr);   //Wide char to Utf-8 , not use too much stack
											  //Utf-8 to Wide char(unicode), not use too much stack 
void QUnc2Utf(LPCWSTR wstr, CStringA &astr);  //Utf-8 to Wide char, not use too much stack 

CStringA qUnc2Utf(LPCWSTR wstr);
CString qUtf2Unc(LPCSTR astr);


unsigned long atox(const void* str, int charwidth=1);
unsigned char atob(const void* ptr_str, int charwidth = 1);
void xtoa(char* str, BYTE dx);

BOOL UnzipFile(CString &sin);

void QA2W(LPCSTR  astr, CString &wstr);
WCHAR* QA2W(LPCSTR astr);
CString GetReadableSize(DWORD32 size);
void QA2W(LPCSTR  astr, CString& wstr, INT charset);
CString ErrorString(int errorcode);
INT SplitStr(CString strSrc, char chGap, CStringArray& strResult);
INT SplitStr(CString strSrc, CString &strGap, CStringArray& strResult);
HANDLE RunProc(LPCTSTR strcmd, LPCTSTR strparam, LPCTSTR strPath, BOOL bsync, BOOL bshow = FALSE);
#include <shellapi.h>
#include <initguid.h> 
#include <Dbt.h>
#include "EnumSerial.h"

#define   GUID_CLASS_USB_DEVICE     GUID_DEVINTERFACE_USB_DEVICE  
extern HDEVNOTIFY hDevNotify ;

extern const GUID GUID_DEVINTERFACE_LIST[];
extern const int GUID_DEVINTERFACE_LIST_LEN;

BOOL  regist_device_htplug(void);

void  PRT_LOG(const _TCHAR* pszFormat, ...);

int ReadReg(LPCTSTR  path, LPCTSTR key, TCHAR* value);
int WriteReg(LPCTSTR path, LPCTSTR key, TCHAR* value);
int DelReg(LPCTSTR path);

extern BOOL g_dbgLogConsole;
DWORD GetFaultError();
void SetFaultError(DWORD err);
#endif