#pragma once


// CExDialog �Ի���
#define WM_BURN_DONE (WM_APP+179)
class CExDialog;
typedef void (CExDialog::*pfnDelegate)(void);

class Thread {
	friend unsigned int __stdcall threadFunction(void *);
public:
	Thread();
	virtual ~Thread();
	int start(void * = NULL);//�߳����������������������������ָ�롣
	void stop();
	void* join();//�ȴ���ǰ�߳̽���
	void detach();//���ȴ���ǰ�߳�
	void Reset();//�õ�ǰ�߳����߸���ʱ�䣬��λΪ����
	unsigned int threadID;

protected:
	virtual void * run(void *) = 0;//����ʵ���߳�����̺߳�������

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
	INT(*onFinish)(void * parm, INT status);//��ɺ���õĺ���. Status < 0 means fail ; return < 0 ,keep connection
};




class CExDialog : public CDialog
{
	DECLARE_DYNAMIC(CExDialog)

public:
	CExDialog(int IDD,CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CExDialog();
	//pfnDelegate m_pfnDelegate;

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXDIALOG };
#endif
	virtual void OnOK() {};   //Avoid ENTER key to exit dialog
	virtual void DelegateRun() {}; 

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
