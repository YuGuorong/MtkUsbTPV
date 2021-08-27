// CUsbTPVApp.cpp: 实现文件
//

#include "pch.h"
#include "FtdiUsbTpv.h"
#include "CUsbTPVApp.h"


// CUsbTPVApp

IMPLEMENT_DYNCREATE(CUsbTPVApp, CWinApp)

CUsbTPVApp::CUsbTPVApp()
{
}

CUsbTPVApp::~CUsbTPVApp()
{
}

BOOL CUsbTPVApp::InitInstance()
{
	// TODO:    在此执行任意逐线程初始化
	return TRUE;
}

int CUsbTPVApp::ExitInstance()
{
	// TODO:    在此执行任意逐线程清理
	return CWinApp::ExitInstance();
}

BEGIN_MESSAGE_MAP(CUsbTPVApp, CWinApp)
END_MESSAGE_MAP()


// CUsbTPVApp 消息处理程序
