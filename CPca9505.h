#pragma once

#include "CFtDevice.h"


class CPca9505: public CFtDevice 
{
	static const int ID_CHIP_ADDR = 0x20;
	static const int IO_GROUP_MAX = 5;
	BYTE m_RegOP[IO_GROUP_MAX];  //Output REG cache
	BYTE m_RegICO[IO_GROUP_MAX]; //Config Reg cache, 0:Output ,1:Input
	typedef enum {
		REG_CONFIG,
		REG_PORT,
	}REG_TYPE;
	enum {
		REG_CFG_ADDR = 0x18,
		REG_PORT_ADDR = 0x8,
		REG_AUTO_INC=0x80,
	};
	BOOL ReadReg(REG_TYPE regType);
	BOOL WriteReg(REG_TYPE regType);
public:
	CPca9505(FT_DEVICE_LIST_INFO_NODE* info, int open_idx);
	~CPca9505();
	BOOL CheckConfig();

	virtual LRESULT SyncIO(int mode, int con, IO_OP* op, BOOL bhwSync = TRUE);
	virtual LRESULT UpdateRaw(void);
};

