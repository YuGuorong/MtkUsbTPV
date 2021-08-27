#pragma once


// CExDialog 对话框
#define WM_BURN_DONE (WM_APP+179)
class CExDialog;
typedef void (CExDialog::*pfnDelegate)(void);

class Thread {
	friend unsigned int __stdcall threadFunction(void *);
public:
	Thread();
	virtual ~Thread();
	int start(void * = NULL);//线程启动函数，其输入参数是无类型指针。
	void stop();
	void* join();//等待当前线程结束
	void detach();//不等待当前线程
	void Reset();//让当前线程休眠给定时间，单位为毫秒
	unsigned int threadID;

protected:
	virtual void * run(void *) = 0;//用于实现线程类的线程函数调用

protected:
	HANDLE threadHandle;
	bool started;
	bool detached;
	void * param;
};

class CDelegate : public Thread
{
public:
	CDelegate(CWnd * pmsgWnd);
	void * prom;
	CWnd  * m_pMsgWnd;
public:
	virtual void * run(void *);
	INT(*onFinish)(void * parm, INT status);//完成后调用的函数. Status < 0 means fail ; return < 0 ,keep connection
};




class CExDialog : public CDialog
{
	DECLARE_DYNAMIC(CExDialog)

public:
	CExDialog(int IDD,CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CExDialog();
	//pfnDelegate m_pfnDelegate;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXDIALOG };
#endif
	virtual void OnOK() {};   //Avoid ENTER key to exit dialog
	virtual void DelegateRun() {}; 

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
