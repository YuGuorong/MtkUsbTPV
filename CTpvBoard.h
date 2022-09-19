#pragma once
#include "CFdtiDriver.h"
#include "ftd2xx.h"
#include <map>
#include <vector>
#include "log.h"


class CTpvBoard {
public:

	BOOL m_bcheckMount;
	BOOL m_bMounted;

public:
	CTpvBoard(int boardIndex);
	~CTpvBoard();
	INT m_index;
	INT m_deviceNum;
	CString m_strPortInfo;
	fdtiid m_BoardID;
	CFtDevice* m_Devices[4];

	void AddDevice(FT_DEVICE_LIST_INFO_NODE* pdevInfo, int open_idx);
	void AddDevice(CFtDevice* pdev);
	void DoMount(BOOL bMount);
	BOOL isMount() { return m_bMounted; };
	void Close();

	LRESULT SyncIO(std::vector<IO_OP>& io_reqQue);
	LRESULT SyncIO(int con, IO_OP* op, BOOL bSyncHW = 1);
	LRESULT Display(int con, IO_VAL* io_val, int* items);

	LRESULT SelMaster(int id);
	LRESULT RunScript(char* script);
	LRESULT Run(void * chip_op);
	LRESULT Reset(void);
};