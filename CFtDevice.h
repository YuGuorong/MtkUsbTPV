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
	virtual LRESULT SyncIO(int mode, int con, IO_OP* op, BOOL bhwSync = TRUE);
	LRESULT SetGPIO(BYTE val);
	LRESULT SetGPIO(int num , BYTE val);
	BOOL  I2C_Write(BYTE chipAddr, BYTE regAddr, BYTE* pv, const int len);
	BOOL  I2C_Read(BYTE chipAddr, BYTE regAddr, BYTE* pv, const int len);
	virtual LRESULT UpdateRaw(void);
	BOOL bcheckMount;
	LONG m_ComPort;
protected:

	FT_DEVICE_LIST_INFO_NODE m_devInfo;
	void SetI2CLinesIdle(void);
	void SetI2CStart(void);
	void SetI2CStop(void);
	BOOL SendAddrAndCheckACK(BYTE dwDataSend, BOOL Read);
	BOOL SendByteAndCheckACK(BYTE dwDataSend);
	BOOL ReadAndSendNAK(BYTE * pRdBuff, int ReadLen);

	LRESULT Write(int idx);
	fdtiid m_FtdiID;
	FT_HANDLE m_ftHandle;
	BYTE m_lowByte;
	BYTE m_highByte;
	int m_OpenIndex;
	
public:
	int isMpsseMode();
	virtual void SetAttribute(char* attr, int id);
	virtual void RunScript(char* script);
	virtual LRESULT Run(void* chip_op);
	virtual LRESULT Reset(BYTE bitpos);
};





