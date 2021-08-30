#include "pch.h"
#include "util.h"
#include <locale.h>
#pragma warning(disable: 4996)   



CPMutex::CPMutex(const TCHAR* name)
{
	memset(m_cMutexName, 0, sizeof(m_cMutexName));
	size_t min = _tcslen(name) > (sizeof(m_cMutexName) - 1) ? (sizeof(m_cMutexName) - 1) : _tcslen(name);
	_tcsncpy_s(m_cMutexName, name, min);
	m_pMutex = CreateMutex(NULL, false, m_cMutexName);
}

CPMutex::~CPMutex()
{
	CloseHandle(m_pMutex);
}

BOOL CPMutex::Lock()
{
	//互斥锁创建失败
	if (NULL == m_pMutex)
	{
		return FALSE;
	}

	DWORD nRet = WaitForSingleObject(m_pMutex, INFINITE);
	if (nRet != WAIT_OBJECT_0)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CPMutex::UnLock()
{
	return ReleaseMutex(m_pMutex);
}

static DWORD m_sysError = 0;
void SetFaultError(DWORD err) {
	m_sysError = err;
}

DWORD GetFaultError() {
	return m_sysError;
}

CString strCurPath;
void GetCurPath(CString &strPath)
{
	TCHAR buff[MAX_PATH * 2];
	::GetModuleFileName(NULL, buff, MAX_PATH * 2);
	strPath = buff;
	int pos = strPath.ReverseFind('\\');
	strPath = strPath.Left(pos + 1);
}

BOOL g_dbgLogConsole = FALSE;
void AppEnvInit()
{
	GetCurPath(strCurPath);
	CString str = strCurPath;
	str += _T("bin");
	CString ssvr;

	TCHAR* buf = new TCHAR[16 * 1024+8];
	CString spath = ssvr + strCurPath + _T("bin");
	int rl = GetEnvironmentVariable(_T("PATH"), buf, 16*1024);
	buf[rl] = 0;
	spath += _T(";");
	spath += buf;
	SetEnvironmentVariable(_T("PATH"), spath);

	str = strCurPath + _T("log");
	CreateDirectory(str, NULL);

	char* env_dbg = getenv("USBTPV_LOG_CONSOLE");

	if( env_dbg )
		g_dbgLogConsole = atoi(env_dbg);
	delete[] buf;

}

int BaseAppInit()
{
	_tsetlocale(LC_CTYPE, _T("chs"));
	//创建互斥对象
	TCHAR strAppName[] = TEXT("eExpressMutex");
	HANDLE hMutex = NULL;
	hMutex = CreateMutex(NULL, FALSE, strAppName);
	if (hMutex != NULL)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			//显示原窗口,关闭互斥对象，退出程序
			CString strTitle;
			strTitle.LoadString(IDSTR_VENDOR_APPNAME);
			HWND hwnd = ::FindWindow(NULL, strTitle);
			if (hwnd)
			{
				::ShowWindow(hwnd, SW_SHOWMAXIMIZED);
				::SetForegroundWindow(hwnd);
			}
			CloseHandle(hMutex);
			return (-1);
		}
	}

	
	return 0;
}

int RunCmd(LPCTSTR scmd, CString &sresult, LPCTSTR sdir, BOOL bshow)
{
	SECURITY_ATTRIBUTES sa;
	HANDLE hRead, hWrite;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
	{
		return -1;
	}
	TCHAR command[1024];    //长达1K的命令行，够用了吧  
	_tcscpy_s(command, 1024, _T("Cmd.exe /C "));
	_tcscat_s(command, 1024, scmd);
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdError = hWrite;            //把创建进程的标准错误输出重定向到管道输入  
	si.hStdOutput = hWrite;           //把创建进程的标准输出重定向到管道输入  
	si.wShowWindow = bshow ? SW_SHOW : SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	//关键步骤，CreateProcess函数参数意义请查阅MSDN  
	if (!CreateProcess(NULL, command, NULL, NULL, TRUE, NULL, NULL, sdir, &si, &pi))
	{
		CloseHandle(hWrite);
		CloseHandle(hRead);
		return -1;
	}
	CloseHandle(hWrite);
	char buffer[64] = { 0 };          //用4K的空间来存储输出的内容，只要不是显示文件内容，一般情况下是够用了。  
	DWORD bytesRead;
	//if (WaitForInputIdle(pi.hProcess, INFINITE) == 0)
	{
		while (true)
		{
			BOOL bret = ReadFile(hRead, buffer, 32, &bytesRead, NULL);
			buffer[bytesRead] = 0;
			if (bret == false) //process exit return false here.
				break;
			CString str;
			QUtf2Unc(buffer, str);
			sresult += str;
		}
		//buffer中就是执行的结果，可以保存到文本，也可以直接输出  		
	}

	DWORD ec;
	GetExitCodeProcess(pi.hProcess, &ec);
	CloseHandle(hRead);
	return ec;
}


int RunCmd(LPCTSTR scmd, TCHAR * sresult, LPCTSTR sdir, BOOL bshow)
{
	SECURITY_ATTRIBUTES sa;
	HANDLE hRead, hWrite;
	if (sresult == NULL) return -1;
	*sresult = '\0';
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
	{
		return -1;
	}
	TCHAR command[1024];    //长达1K的命令行，够用了吧  
	_tcscpy_s(command, 1024, _T("Cmd.exe /C "));
	_tcscat_s(command, 1024, scmd);
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfo(&si);
	si.hStdError = hWrite;            //把创建进程的标准错误输出重定向到管道输入  
	si.hStdOutput = hWrite;           //把创建进程的标准输出重定向到管道输入  
	si.wShowWindow = bshow ? SW_SHOW : SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	//关键步骤，CreateProcess函数参数意义请查阅MSDN  
	if (!CreateProcess(NULL, command, NULL, NULL, TRUE, NULL, NULL, sdir, &si, &pi))
	{
		CloseHandle(hWrite);
		CloseHandle(hRead);
		return -1;
	}
	CloseHandle(hWrite);
	char buffer[64] = { 0 };          //用4K的空间来存储输出的内容，只要不是显示文件内容，一般情况下是够用了。  

	DWORD bytesRead;
	//if (WaitForInputIdle(pi.hProcess, INFINITE) == 0)
	{
		while (true)
		{
			BOOL bret = ReadFile(hRead, buffer, 32, &bytesRead, NULL);
			buffer[bytesRead] = 0;
			if (bret == false) //process exit return false here.
				break;
			CString str;
			USES_CONVERSION;			
			QUtf2Unc(buffer, str);
			_tcscat_s(sresult, 4096,str);
			OutputDebugStringW(_T("\n"));
			OutputDebugStringW(str);
			OutputDebugStringW(_T("\n"));
		}
		//buffer中就是执行的结果，可以保存到文本，也可以直接输出  		
	}

	DWORD ec;
	GetExitCodeProcess(pi.hProcess, &ec);
	CloseHandle(hRead);
	return ec;
}



HANDLE RunProc(LPCTSTR strcmd, LPCTSTR strparam, LPCTSTR strPath, BOOL bsync, BOOL bshow)
{
	SHELLEXECUTEINFO ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = strcmd;
	ShExecInfo.lpParameters = strparam;
	ShExecInfo.lpDirectory = strPath;
	ShExecInfo.nShow = bshow;// bsync ? SW_HIDE : SW_SHOW;
	ShExecInfo.hInstApp = NULL;
	ShExecInfo.hProcess = INVALID_HANDLE_VALUE;
	if (ShellExecuteEx(&ShExecInfo)) {
		if (bsync)
			WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
	}
	return ShExecInfo.hProcess;
}

BOOL IsPathExist(const CString & csPath)
{
	int nRet = _taccess(csPath, 0);
	return 0 == nRet || EACCES == nRet;
}

BOOL GetFileSize(LPCTSTR  sfile, DWORD &flen)
{
	WIN32_FIND_DATA fileInfo;
	HANDLE hFind;
	DWORD fileSize;
	hFind = FindFirstFile(sfile, &fileInfo);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		fileSize = fileInfo.nFileSizeLow;
		FindClose(hFind);
		flen = fileSize;
		return TRUE;
	}
	return FALSE;
}


void Utf2Unc(const char  * bstr, WCHAR  * wstr, int lens, int lenw)
{
	::MultiByteToWideChar(CP_UTF8, 0, bstr, lens, wstr, lenw);
}


void Unc2Utf(LPCTSTR   wstr, char  * bstr, int lenw, int lens)
{
	int ret = ::WideCharToMultiByte(CP_UTF8, 0, wstr, -1, bstr, lens, 0, 0);
}

CString qUtf2Unc(LPCSTR astr)
{
	CString wstr;
	int slen = lstrlenA(astr) + 1;
	WCHAR * pwbuf = wstr.GetBuffer(slen);
	::MultiByteToWideChar(CP_UTF8, 0, astr, slen, pwbuf, slen);
	wstr.ReleaseBuffer();
	return wstr;
}

CStringA qUnc2Utf(LPCWSTR wstr)
{
	CStringA astr;
	int slen = lstrlenW(wstr) + 1;
	char * psbuf = astr.GetBuffer(slen * 4);
	int ret = ::WideCharToMultiByte(CP_UTF8, 0, wstr, slen, psbuf, slen * 4, 0, 0);
	astr.ReleaseBuffer();
	return astr;
}

void QUtf2Unc(LPCSTR  astr, CString &wstr)
{
	int slen = lstrlenA(astr) + 1;
	WCHAR * pwbuf = wstr.GetBuffer(slen);
	::MultiByteToWideChar(CP_UTF8, 0, astr, slen, pwbuf, slen);
	wstr.ReleaseBuffer();
}

void QUnc2Utf(LPCWSTR wstr, CStringA &astr)
{
	int slen = lstrlenW(wstr) + 1;
	char * psbuf = astr.GetBuffer(slen * 4);
	int ret = ::WideCharToMultiByte(CP_UTF8, 0, wstr, slen, psbuf, slen * 4, 0, 0);
	astr.ReleaseBuffer();
}

WCHAR* QA2W(LPCSTR astr)
{
	int slen = lstrlenA(astr) + 1;
	WCHAR * pwbuf = new WCHAR[slen];
	::MultiByteToWideChar(CP_ACP, 0, astr, slen, pwbuf, slen);
	return pwbuf;
}

void QA2W(LPCSTR  astr, CString &wstr)
{
	int slen = lstrlenA(astr) + 1;
	WCHAR * pwbuf = wstr.GetBuffer(slen);
	::MultiByteToWideChar(CP_ACP, 0, astr, slen, pwbuf, slen);
	wstr.ReleaseBuffer();
}

void QA2W(LPCSTR  astr, CString& wstr, int charset)
{
	int slen = lstrlenA(astr) + 1;
	WCHAR* pwbuf = wstr.GetBuffer(slen);
	::MultiByteToWideChar(charset, 0, astr, slen, pwbuf, slen);
	wstr.ReleaseBuffer();
}


CString GetReadableSize(DWORD32 size)
{
	UINT32 ip, pp;
	int i = 0;
	ip = size; pp = 0;
	TCHAR salias[] = { _T(" KMG") };
	while (ip>1024)
	{
		if (i >= sizeof(salias) / sizeof(TCHAR) - 1) break;
		i++;
		pp = ip % 1024;
		ip = ip / 1024;
	}
	pp = (pp + 5) / 10;
	CString stri;
	stri.Format(_T("%d.%02d"), ip, pp);
	stri.TrimRight(_T('0'));
	stri.TrimRight(_T('.'));
	stri += salias[i];
	stri.TrimRight();
	stri += _T('B');
	return stri;
}


////////////////////////////////////////////////////////////////////////////////
//com.cpp  end
void xtoa(char* str, BYTE dx)
{
	BYTE dx_h = (dx & (BYTE)0xF);
	if (dx_h > 9)
		str[1] = dx_h - 0xa + 'A';
	else
		str[1] = dx_h + '0';

	dx_h = (dx >> 4) & 0xF;
	if (dx_h > 9)
		str[0] = dx_h - 0xa + 'A';
	else
		str[0] = dx_h + '0';
	str[2] = ' ';
}


unsigned char atob(const void* ptr_str, int charwidth) {
	const char* str = (const char*)ptr_str;
	unsigned char	sum = 0;
	int		i = 0;
	char	ch;
	if (str == NULL) return 0;

	while (*str && i < 2)
	{
		ch = *(str);
		str += charwidth;
		if ((ch >= '0') && (ch <= '9'))
		{
			sum = sum * 0x10 + ch - '0';
			i++;
		}
		else if ((ch >= 'a') && (ch <= 'f'))
		{
			sum = sum * 0x10 + ch - 'a' + 0xA;
			i++;
		}
		else if ((ch >= 'A') && (ch <= 'F'))
		{
			sum = sum * 0x10 + ch - 'A' + 0xA;
			i++;
		}
		else {
			break;
		}
	}
	return sum;
}

unsigned long atox(const void * ptr_str, int charwidth)
{
	const char* str = (const char*)ptr_str;
	DWORD	sum;
	int		i = 0;
	char	ch;
	if (str == NULL) return 0;
	str += strspn(str, " xX");
	if (str[0] == '0' && (str[1] | 0x20) == 'x') {
		str += 2;
	}

	sum = 0;

	while (*str && i < 8)
	{
		ch = *(str);
		str += charwidth;
		if ((ch >= '0') && (ch <= '9'))
		{
			sum = sum * 0x10 + ch - '0';
			i++;
		}
		else if ((ch >= 'a') && (ch <= 'f'))
		{
			sum = sum * 0x10 + ch - 'a' + 0xA;
			i++;
		}
		else if ((ch >= 'A') && (ch <= 'F'))
		{
			sum = sum * 0x10 + ch - 'A' + 0xA;
			i++;
		}
		else {
			break;
		}
	}
	return sum;
}

CString ErrorString(int errorcode) 
{
	CString str;
	LPVOID lpMsgBuf = LocalLock(LocalAlloc(LMEM_MOVEABLE | LMEM_ZEROINIT, 1000));
	if (lpMsgBuf) {
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			errorcode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR)&lpMsgBuf,
			0,
			NULL);
		str = (LPCTSTR)lpMsgBuf;
		LocalFree(lpMsgBuf);
	}
	return str;
}


INT SplitStr(CString strSrc, CString &strGap, CStringArray& strResult)
{
	int i = 0; 
	int nPos = strSrc.Find(strGap);
	CString strLeft = _T("");

	while (0 <= nPos)
	{
		i++;
		strLeft = strSrc.Left(nPos);
		if (!strLeft.IsEmpty())
		{
			strResult.Add(strLeft);
		}

		strSrc = strSrc.Right(strSrc.GetLength() - nPos - strGap.GetLength());
		nPos = strSrc.Find(strGap);
	}

	if (!strSrc.IsEmpty())
	{
		strResult.Add(strSrc);
	}
	return i;
}

INT SplitStr(CString strSrc, char chGap, CStringArray& strResult)
{
	int i = 0;
	int nPos = strSrc.Find(chGap);
	CString strLeft = _T("");

	while (0 <= nPos)
	{
		i++;
		strLeft = strSrc.Left(nPos);
		if (!strLeft.IsEmpty())
		{
			strResult.Add(strLeft);
		}

		strSrc = strSrc.Right(strSrc.GetLength() - nPos - 1);
		nPos = strSrc.Find(chGap);
	}

	if (!strSrc.IsEmpty())
	{
		strResult.Add(strSrc);
	}
	return i;
}

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "setupapi.lib")




//DEFINE_GUID(GUID_DEVINTERFACE_USB_DEVICE,
//	0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED);
///* f18a0e88-c30c-11d0-8815-00a0c906bed8 */
//DEFINE_GUID(GUID_DEVINTERFACE_USB_HUB,
//	0xf18a0e88, 0xc30c, 0x11d0, 0x88, 0x15, 0x00, 0xa0, 0xc9, 0x06, 0xbe, 0xd8);

/*
未装驱动，收不到WM_DEVICECHANGE 解决方法
只需用DEVICE_NOTIFY_ALL_INTERFACE_CLASSES替代DEVICE_NOTIFY_WINDOW_HANDLE即可
*/
// Compute Device Class: this is used to get the tree contrl root icon
const GUID GUID_DEVCLASS_COMPUTER =
{ 0x4D36E966, 0xE325, 0x11CE,{ 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } };

// Copy from HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\DeviceClasses
const GUID GUID_DEVINTERFACE_LIST[] =
{
	// GUID_DEVINTERFACE_USB_DEVICE
	{ 0xA5DCBF10, 0x6530, 0x11D2,{ 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } },

	//// GUID_DEVINTERFACE_DISK
	//{ 0x53f56307, 0xb6bf, 0x11d0, { 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } },

	//// GUID_DEVINTERFACE_HID, 
	//{ 0x4D1E55B2, 0xF16F, 0x11CF, { 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } },

	//// GUID_NDIS_LAN_CLASS
	//{ 0xad498944, 0x762f, 0x11d0, { 0x8d, 0xcb, 0x00, 0xc0, 0x4f, 0xc3, 0x35, 0x8c } }

	//// GUID_DEVINTERFACE_COMPORT
	//{ 0x86e0d1e0, 0x8089, 0x11d0, { 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73 } },

	//// GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR
	//{ 0x4D36E978, 0xE325, 0x11CE, { 0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18 } },

	//// GUID_DEVINTERFACE_PARALLEL
	//{ 0x97F76EF0, 0xF883, 0x11D0, { 0xAF, 0x1F, 0x00, 0x00, 0xF8, 0x00, 0x84, 0x5C } },

	//// GUID_DEVINTERFACE_PARCLASS
	//{ 0x811FC6A5, 0xF728, 0x11D0, { 0xA5, 0x37, 0x00, 0x00, 0xF8, 0x75, 0x3E, 0xD1 } }
};

const int GUID_DEVINTERFACE_LIST_LEN = sizeof(GUID_DEVINTERFACE_LIST) / sizeof(GUID);
HDEVNOTIFY hDevNotify = (HDEVNOTIFY)0;

BOOL  regist_device_htplug(void)
{
	DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
	ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
	NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
	NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	for (int i = 0; i < GUID_DEVINTERFACE_LIST_LEN; i++) {
		NotificationFilter.dbcc_classguid = GUID_DEVINTERFACE_LIST[i];
		CWnd* pwnd = ::AfxGetMainWnd();
		hDevNotify = RegisterDeviceNotification(pwnd->GetSafeHwnd(), &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
		if (!hDevNotify) {
			AfxMessageBox(CString("Can't register device notification: ")
				+ _com_error(GetLastError()).ErrorMessage(), MB_ICONEXCLAMATION);
			return FALSE;
		}
	}
	return TRUE;
}


void  PRT_LOG(const _TCHAR* pszFormat, ...)
{
	_TCHAR buf[1024] = { 0 };
	// ZeroMemory( buf, 1024*sizeof(TCHAR ) );
	
	va_list arglist;
	va_start(arglist, pszFormat);
	vswprintf_s(&buf[0], 1024 , pszFormat, arglist);
	va_end(arglist);
	int nBuffSize = _tcslen(buf);
	_tcscat_s(buf, 1024 - nBuffSize, _T("\n"));
	OutputDebugString(buf);
}


/************************************
@ Brief:		打开注册表，读取Key对应value
@ Author:		woniu201
@ Created:		2018/09/07
@ Return:
************************************/
int ReadReg(LPCTSTR  path, LPCTSTR key, TCHAR* value)
{
	CString strpath = L"SOFTWARE\\";
	strpath += path;
	HKEY hKey;
	int ret = RegOpenKeyEx(HKEY_CURRENT_USER, strpath, 0, KEY_EXECUTE, &hKey);
	if (ret != ERROR_SUCCESS)
	{
		TRACE( "打开注册表失败" );
		return 1;
	}

	char rbuf[64];
	//读取KEY
	DWORD dwType = REG_SZ; //数据类型
	DWORD cbData = 256;
	ret = RegQueryValueEx(hKey, key, NULL, &dwType, (LPBYTE)rbuf, &cbData);
	if (ret == ERROR_SUCCESS)
	{
		rbuf[cbData] = 0;
		TRACE( "%s\n" , (char *)rbuf);
		USES_CONVERSION;
		TCHAR* vn = A2T(rbuf);
		_tcscpy_s(value, 64, vn);
		value[cbData] = 0;
	}
	else
	{
		TRACE( "读取注册表中KEY 失败\n" );
		RegCloseKey(hKey);
		return 1;
	}
	RegCloseKey(hKey);

	return 0;
}




/************************************
@ Brief:		写注册表，如不存在自动创建
@ Author:		woniu201
@ Created:		2018/09/07
@ Return:
************************************/
int WriteReg(LPCTSTR path, LPCTSTR key, TCHAR* value)
{
	HKEY hKey;
	DWORD dwDisp;
	DWORD dwType = REG_SZ; //数据类型
	CString strpath = L"SOFTWARE\\";
	strpath += path;

	int ret = RegCreateKeyEx(HKEY_CURRENT_USER, strpath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisp);
	if (ret != ERROR_SUCCESS)
	{
		TRACE("创建注册表失败\n");
		return 1;
	}
	ret = RegSetValueEx(hKey, key, 0, dwType, (BYTE*)value, _tcslen(value));
	if (ret != ERROR_SUCCESS)
	{
		TRACE("注册表中创建KEY VALUE失败\n");
		RegCloseKey(hKey);
		return 1;
	}
	RegCloseKey(hKey);
	return 0;
}

/************************************
@ Brief:		删除注册表
@ Author:		woniu201
@ Created:		2018/09/07
@ Return:
************************************/
int DelReg(LPCTSTR path)
{
	CString strpath = L"SOFTWARE\\";
	strpath += path;
	int ret = RegDeleteKey(HKEY_CURRENT_USER, strpath);
	if (ret == ERROR_SUCCESS)
	{
		TRACE("删除成功\n");
	}
	else
	{
		TRACE("删除失败\n");
		return 1;
	}
	return 0;
}

int test_reg(int argc, _TCHAR* argv[])
{
	TCHAR value[MAX_PATH] = { 0 };
	//ReadReg(L"Software\\Woniu", L"aaa", value);

	//WriteReg(L"Software\\Woniu", L"aaa", L"bbb");

	//DelReg(L"Software\\Woniu");
	getchar();
	return 0;
}
