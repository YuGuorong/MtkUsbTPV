// ExDialog.cpp : 实现文件
//

#include "pch.h"
#include "ExDialog.h"
#include "afxdialogex.h"

//>>>>>>>>====================>thread class============================>
unsigned int __stdcall threadFunction(void * object)
{
	Thread * thread = (Thread *)object;
	return (unsigned int)thread->run(thread->param);
}

Thread::Thread()
{
	started = false;
	detached = false;
}

Thread::~Thread()
{
	stop();
}

int Thread::start(void* pra)
{
	if (!started)
	{
		param = pra;
		if (threadHandle = (HANDLE)_beginthreadex(NULL, 0, threadFunction, this, 0, &threadID))
		{
			if (detached)
			{
				CloseHandle(threadHandle);
			}
			started = true;
		}
	}
	return started;
}

//wait for current thread to end.
void * Thread::join()
{
	DWORD status = (DWORD)NULL;
	if (started && !detached)
	{
		WaitForSingleObject(threadHandle, INFINITE);
		GetExitCodeThread(threadHandle, &status);
		CloseHandle(threadHandle);
		detached = true;
	}
	return (void *)status;
}

void Thread::detach()
{
	if (started && !detached)
	{
		CloseHandle(threadHandle);
	}
	detached = true;
}

void Thread::stop()
{
	DWORD status = (DWORD)NULL;
	if (started && !detached)
	{
		if (!GetExitCodeThread(threadHandle, &status) || status == STILL_ACTIVE)
		{
			WaitForSingleObject(threadHandle, 1000);
			TerminateThread(threadHandle, 0);
		}
		CloseHandle(threadHandle);
		detached = true;
	}
}

void Thread::Reset()
{
	started = false;
	detached = false;
}
//<====================thread class<============================<<<<<<<<<
CDelegate::CDelegate(CWnd * pmsgWnd)
	: Thread()
{
	m_pMsgWnd = pmsgWnd;
}

void * CDelegate::run(void * param)
{
	CExDialog * pdlg = (CExDialog*)param;
	pdlg->DelegateRun();
	
	pdlg->PostMessage(WM_BURN_DONE, (WPARAM)this, 0);
	return 0;
}

// CExDialog 对话框

IMPLEMENT_DYNAMIC(CExDialog, CDialog)

CExDialog::CExDialog(int IDD, CWnd* pParent /*=NULL*/)
	: CDialog(IDD, pParent)
{
	//m_pfnDelegate = NULL;
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif

}

CExDialog::~CExDialog()
{
}

void CExDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CExDialog::PreTranslateMessage(MSG * pMsg)
{

	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
		{
			//if (this->GetSafeHwnd() == ::AfxGetMainWnd()->GetSafeHwnd() )
				return TRUE;
		}
	}
	else if (pMsg->message == WM_COPYDATA)
	{

	}

	return CDialog::PreTranslateMessage(pMsg);
}


BEGIN_MESSAGE_MAP(CExDialog, CDialog)
END_MESSAGE_MAP()


// CExDialog 消息处理程序
