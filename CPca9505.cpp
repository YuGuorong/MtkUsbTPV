#include "pch.h"
#include "CPca9505.h"
/*
FTDI GPIO Layout
| 31 30 29 28 27 26 25 24 | 23 22 21 20 19 18 17 16 | 15 14 13 12 11 10 09 08 | 07 06 05 04 03 02 01 00 |
|   CON7  |  CON6  |  CON5    |   CON4 |   GPIO-H   |   CON3  |  CON2  |   CON1   | CON0   |   GPIO-L   |

Parameter Raw data layout
| 31 30 29 28 27 26 25 24 | 23 22 21 20 19 18 17 16 | 15 14 13 12 11 10 09 08 | 07 06 05 04 03 02 01 00 |
|                         |   CON7  |  CON6  | CON5     |  CON4   |   CON3 |   CON2  |  CON1  |  CON0   | 

PCA9505 Layout
| 31 30 29 28 27 26 25 24 | 23 22 21 20 19 18 17 16 | 15 14 13 12 11 10 09 08 | 07 06 05 04 03 02 01 00 |
  -  - |  CON7  |  CON6   | -  - |  CON5  |  CON4   | -  - |  CON3  |   CON2  | -  - |  CON1  |  CON0   |

*/



#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_0(x)    DEFER_2(x, __COUNTER__)
#define defer(expr)   auto DEFER_0(_defered_option) = deferer([&](){expr;})


template <typename Function>
struct doDefer {
	Function f;
	doDefer(Function f) : f(f) {}
	~doDefer() { f(); }
};

template <typename Function>
doDefer<Function> deferer(Function F) {
	return doDefer<Function>(f);
}
#define defer(expr) auto __defered = deferer([&](){expr;})

BOOL CPca9505::ReadReg(REG_TYPE regType)
{
	BYTE regAddr = (regType == REG_CONFIG) ? (REG_CFG_ADDR| REG_AUTO_INC ): (REG_PORT_ADDR| REG_AUTO_INC);
	BYTE* pcache = (regType == REG_CONFIG) ?  m_RegICO: m_RegOP;
	BYTE cache[IO_GROUP_MAX*2];
		
	if (I2C_Read(ID_CHIP_ADDR, regAddr, cache, IO_GROUP_MAX))
	{
		memcpy(pcache, cache, IO_GROUP_MAX);
		return TRUE;
	}
	return FALSE;
}

BOOL CPca9505::WriteReg(REG_TYPE regType)
{
	BYTE regAddr = (regType == REG_CONFIG) ? (REG_CFG_ADDR | REG_AUTO_INC) : (REG_PORT_ADDR | REG_AUTO_INC);
	BYTE* pcache = (regType == REG_CONFIG) ? m_RegICO : m_RegOP;
	if (regType == REG_PORT) {
		CStringA str;
		char shex[16];
		char c = ' ';
		for (int i = 0; i < 5; i++) {
			sprintf_s(shex, 16, "%c%02X",c, m_RegOP[i]);
			str += shex;
			c = ',';
		}
		logInfo(L"Write PCA9505 :[%S]", str);
	}

	if (I2C_Write(ID_CHIP_ADDR, regAddr, pcache, IO_GROUP_MAX))
	{
		return TRUE;
	}
	return FALSE;
}

CPca9505::CPca9505(FT_DEVICE_LIST_INFO_NODE* info, int open_idx)
	:CFtDevice(info, open_idx)
{
	memset(m_RegICO, 0xFF, sizeof(m_RegOP));
	memset(m_RegICO, 0x00, sizeof(m_RegICO));
}

CPca9505::~CPca9505()
{
	Close();
}

BOOL CPca9505::CheckConfig()
{
	BOOL ret = TRUE;
	if (m_ftHandle == INVALID_HANDLE_VALUE && Open() != S_OK) return FALSE;
	
	BOOL bSucceed = ReadReg(REG_CONFIG);
	if (bSucceed) {
		bSucceed = ReadReg(REG_CONFIG);
		for (int i = 0; i < IO_GROUP_MAX; i++) {
			if ((m_RegICO[i]) != 0x00) {
				memset(m_RegICO, 0, IO_GROUP_MAX);
				bSucceed =  I2C_Write(ID_CHIP_ADDR, (REG_AUTO_INC | REG_CFG_ADDR), m_RegICO, IO_GROUP_MAX);
				return ReadReg(REG_CONFIG);
			}
		}
	}
	else {
		Close();
	}
	return bSucceed;
}


LRESULT CPca9505::SyncIO(int mode, int con, IO_OP* op, BOOL bhwSync)
{
	if (m_ftHandle == INVALID_HANDLE_VALUE && Open() != S_OK) return ERROR_DEVICE_NOT_AVAILABLE;

	int regbase = con ==IO_CON_GPIO?  4 :  con /2;
	FT_STATUS ftStatus = FT_OK;

	if (op->ret != S_OK) return op->ret;
	IO_VAL * ioval = (IO_VAL*)&op->val;

	if (mode == CFtdiDriver::IO_WRITE) {
		if (ioval->fmt == IO_RAW) {
			DWORD rawv = ioval->v.raw;
			for (int i = 0; i < IO_GROUP_MAX; i++) {
				m_RegOP[i] = rawv;
			}
			bhwSync = TRUE;
			op->ret = ERROR_NO_MORE_DEVICES;
		}
		else if (con == IO_CON_GPIO) {
			
			if (ioval->fmt == GPIO_RAW ) {
				m_RegOP[4] = ioval->v.gpio.val;
			}
			else {
				m_RegOP[4] &= ~(1 << ioval->v.gpio.bit);
				if(ioval->v.gpio.val )	m_RegOP[4] |= (1 << ioval->v.gpio.bit);
			}
		}
		else {
			char* pin = ioval->v.pin;
			int gpiobase = (con % 2) * IO_MAX_KEY;
			BYTE* pcache = &m_RegOP[regbase];
			if (ioval->fmt == CON_RAW) {
				BYTE raw = ioval->v.raw;
				for (int i = 0; i < IO_MAX_KEY; i++) {
					pin[i] = raw  & (1 << i);
				}
				ioval->fmt = CON_CTL;
			}

			for (int i = 0; i < IO_MAX_KEY; i++) {
				int gpionum = gpiobase + i;
				if (pin[i] >= 0) { //the pin value 0 means donot change it.keep it stay its state.
					*pcache &= ~(1 << gpionum);
					if (pin[i]) {
						*pcache |= 1 << gpionum;
					}
				}
			}
		}
		if (bhwSync) ftStatus = WriteReg(REG_PORT) ? S_OK : ERROR_WRITE_FAULT;
	}
	else {
		if (bhwSync) op->ret = UpdateRaw();
		
		if (con == IO_CON_GPIO) {
			op->val.v.gpio.bit = 0xff;
			op->val.v.gpio.val = m_RegOP[4];
		}
		else {
			if (op->ext_val == NULL) op->ext_val = op->val.v.pin;
			BYTE* pcache = &m_RegOP[regbase];
			int gpiobase = (con % 2) * IO_MAX_KEY;
			for (int i = 0; i < IO_MAX_KEY; i++) {
				int gpionum = gpiobase + i;
				op->ext_val[i] = (*pcache & (1 << gpionum) )? 1 : 0;
			}
		}
	}
	return  (ftStatus == FT_OK) ? S_OK : ftStatus;
}

LRESULT CPca9505::UpdateRaw(void)
{
	if (m_ftHandle == INVALID_HANDLE_VALUE) {
		if( Open() != S_OK) return ERROR_OPEN_FAILED;//dumy read
		BOOL bSucceed = I2C_Read(ID_CHIP_ADDR, (REG_AUTO_INC | REG_PORT_ADDR), m_RegOP, IO_GROUP_MAX);
	}

	BOOL bSucceed = I2C_Read(ID_CHIP_ADDR, (REG_AUTO_INC | REG_PORT_ADDR ), m_RegOP, IO_GROUP_MAX);
	CStringA str;
	char shex[16];
	char c = ' ';
	for (int i = 0; i < 5; i++) {
		sprintf_s(shex, 16, "%c%02X",c, m_RegOP[i]);
		str += shex;
		c = ',';
	}
	logInfo(L"Update raw cache :[%S]", str);
	return bSucceed ? S_OK : ERROR_READ_FAULT;
}
