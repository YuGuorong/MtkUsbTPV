#include "pch.h"
#include "CSimpleLog.h"
#include "util.h"
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
//记得导入头文件
#include<time.h>
#include<stdarg.h>


CSimpleLog * CSimpleLog::_plog = NULL;

CSimpleLog::CSimpleLog()
{
	AppEnvInit();
	ReopenLogFile();
}

CSimpleLog::~CSimpleLog()
{
	if (m_fileLog) {
		m_fileLog->Close();
		delete m_fileLog;
		m_fileLog = NULL;
	}
}

TCHAR log_buf[4096 + 48] = { 0 };
int CSimpleLog::Log(int tag, int fline,  LPCTSTR fmt, ...)
{
	const LPCTSTR stag[] = {
		L"[TRACE]: ",
		L"[DEBUG]: ",
		L"[INFO]: ",
		L"[WARNING]: ",
		L"[ERROR]: ",
	};

	CTime time;
	time = CTime::GetCurrentTime();
	CString curdata = time.Format("%y-%m-%d %H:%M:%S");

	struct   _timeb   timebuffer;
	_ftime64_s(&timebuffer);

	_tcscpy_s(log_buf, 2048, curdata);
	TCHAR* ptr = log_buf + _tcslen(log_buf);
	swprintf_s(ptr, 2048, L".%3d(%d),", timebuffer.millitm, fline);
	ptr = ptr + _tcslen(ptr);
	_tcscat_s(ptr, 2048, stag[tag]);
	ptr += _tcslen(ptr);

	int len;
	va_list args;
	va_start(args, fmt);
	len = vswprintf_s(ptr, 2048, fmt, args);//因为%的原因导致了崩溃
	va_end(args);
	_tcscat_s(ptr, 2048, L"\r\n");
	CStringA stxt = qUnc2Utf(log_buf);
	
	if (m_fileLog)
		m_fileLog->Write(stxt, stxt.GetLength());
	if (g_dbgLogConsole) {
		USES_CONVERSION;
		printf(T2A(log_buf));
	}
	return 0;
}

void CSimpleLog::ReopenLogFile()
{
    if (m_fileLog) {
        m_fileLog->Close();
        delete m_fileLog;
    }

    CTime time;
    time = CTime::GetCurrentTime();

    CString spath(L"log\\FT2322_log_" );
    spath = strCurPath + spath + time.Format("%y%m%d.log");
    m_fileLog = new CFile;

    if (m_fileLog->Open(spath, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate, NULL) == FALSE)
    {
        delete m_fileLog;
        m_fileLog = NULL;
        return;
    }
    m_fileLog->SeekToEnd();
}

void CSimpleLog::Init()
{
	if (_plog == NULL)
		_plog = new CSimpleLog();
}

void CSimpleLog::UnInit()
{
	if (_plog) {
		delete _plog;
		_plog = NULL;
	}
}


class _cgc {
public:
	_cgc() {};
	~_cgc() { CSimpleLog::UnInit(); };
};

_cgc gc;

void InitLog() 
{
	CSimpleLog::Init();
}