#pragma once

#include "ftd2xx.h"
#include <map>
#include <vector>
#include "CFdtiDriver.h"
#include "log.h"


class CFtDevice
{
public:
	CFtDevice(FT_DEVICE_LIST_INFO_NODE * info, int open_idx);
	~CFtDevice();

	LRESULT Open(void);
	LRESULT Close(void);
	LRESULT Set(int raw);
	LRESULT SyncIO(int mode, int con, IO_OP* op, BOOL bhwSync = TRUE);
	LRESULT SetGPIO(BYTE val);
	LRESULT SetGPIO(int num , BYTE val);
	LRESULT GetRaw(BYTE* vl , BYTE* vh);
	BOOL bcheckMount;
	LONG m_ComPort;
protected:
	FT_DEVICE_LIST_INFO_NODE m_devInfo;
	LRESULT ReadChn(int idx);
	LRESULT Write(int idx);
	fdtiid m_FtdiID;
	FT_HANDLE m_ftHandle;
	BYTE m_lowByte;
	BYTE m_highByte;
	int m_OpenIndex;
	
public:
	int isMpsseMode();
};


class CFtBoard {
public:

	BOOL m_bcheckMount;
	BOOL m_bMounted;

public:
	CFtBoard(int boardIndex);
	~CFtBoard();
	INT m_index;
	INT m_deviceNum;
	CString m_strPortInfo;
	fdtiid m_BoardID;
	map<fdtiid, CFtDevice*> m_Devices;
	
	void AddDevice(FT_DEVICE_LIST_INFO_NODE * pdevInfo, int open_idx);
	void DoMount(BOOL bMount);
	
	LRESULT SyncIO(vector<IO_OP>& io_reqQue);
	LRESULT SyncIO(int con, IO_OP* op);
	LRESULT Display(int con, IO_VAL* io_val, int* items);
};


