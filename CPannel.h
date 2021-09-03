#pragma once

#include "ExDialog.h"
#include "uilib/GdipButton.h"
#include "CFtDevice.h"


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
	CComboBox m_cbDevice;

protected:
	CTpvBoard* m_pBoard;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	void SetBoard(CTpvBoard * board);
	void SetCon(int id,  int key);
	void SetAll(char val);
	void SetGpioRaw(int val);
	void UpdateIoState();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnClrall();
	afx_msg void OnBnClickedGpioCtl(UINT id);
	afx_msg void OnBnClickedConset(UINT id);
	afx_msg void OnBnClickedBtnSetall();
	afx_msg void OnBnClickedBtnPwrkey();
	afx_msg void OnBnClickedBtnKcol0();
	afx_msg void OnBnClickedBtnHome();
	afx_msg void OnBnClickedBtnGpio();
	afx_msg void OnCbnSelchangeComboDev();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};
