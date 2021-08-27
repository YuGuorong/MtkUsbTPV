#pragma once



// CUsbTPVApp

class CUsbTPVApp : public CWinApp
{
	DECLARE_DYNCREATE(CUsbTPVApp)

protected:
	CUsbTPVApp();           // 动态创建所使用的受保护的构造函数
	virtual ~CUsbTPVApp();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	DECLARE_MESSAGE_MAP()
};


