#pragma once

#define logInfo(...)    do { if(CSimpleLog::_plog) CSimpleLog::_plog->Log(LOG_INFO,   __LINE__, __VA_ARGS__) ;}while(0)
#define logWarning(...) do { if(CSimpleLog::_plog) CSimpleLog::_plog->Log(LOG_WARNING,__LINE__, __VA_ARGS__) ;}while(0)
#define logDebug(...)   do { if(CSimpleLog::_plog) CSimpleLog::_plog->Log(LOG_DEBUG,  __LINE__, __VA_ARGS__) ;}while(0)
#define logTrace(...)   do { if(CSimpleLog::_plog) CSimpleLog::_plog->Log(LOG_TRACE,  __LINE__, __VA_ARGS__) ;}while(0)
#define logError(...)   do { if(CSimpleLog::_plog) CSimpleLog::_plog->Log(LOG_ERROR,  __LINE__, __VA_ARGS__) ;}while(0)

enum {
	LOG_TRACE,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR,
};

class CSimpleLog
{
public:
	CSimpleLog();
	~CSimpleLog();
	/*int Info( LPCTSTR fmt, ...);
	int Warning(LPCTSTR fmt, ...);*/
	int Log(int tag, int fline, LPCTSTR fmt, ...);
	void ReopenLogFile();
	static void Init();
	static void UnInit();
	static CSimpleLog* _plog;
protected:
	CFile* m_fileLog;
};

void InitLog();
