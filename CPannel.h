﻿#pragma once

#include "ExDialog.h"
#include "uilib/GdipButton.h"
// CPannel 对话框
#define CON_MAX 8
#define ROW_MAX 4

class CPannel : public CExDialog
{
	DECLARE_DYNAMIC(CPannel)

public:
	CPannel(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CPannel();

// 对话框数据
#if 1 //def AFX_DESIGN_TIME
	enum { IDD = IDD_PANNEL };
#endif
	CFont* m_of;
	int m_state;
	CHARFORMAT2 m_cf;
	int  m_DevId; //index of genreation

	CGdipButton* m_AllBtns[2];;
	CGdipButton* m_cGpioBtns[CON_MAX][ROW_MAX];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	void SetIndex(int idx);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnClrall();
	afx_msg void OnBnClickedGpioCtl(UINT id);
};