#include "pch.h"
#include "CPca9505.h"

/*
PCA9505 CONFIG 0:Output/1:Input
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

unsigned long atox(const void* ptr_str, int charwidth);

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

CPca9505::CPca9505(FT_DEVICE_LIST_INFO_NODE* info, int open_idx, int chipid)
	:CFtDevice(info, open_idx)
{
	ID_CHIP_ADDR = chipid;
	memset(m_RegOP, 0, sizeof(m_RegOP));
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
		//for (int i = 0; i < IO_GROUP_MAX; i++) {
		//	if ((m_RegICO[i]) != 0x00) {
		//		memset(m_RegICO, 0, IO_GROUP_MAX);
		//		bSucceed =  I2C_Write(ID_CHIP_ADDR, (REG_AUTO_INC | REG_CFG_ADDR), m_RegICO, IO_GROUP_MAX);
		//		return ReadReg(REG_CONFIG);
		//	}
		//}
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

void CPca9505::SetAttribute(char* attr, int id) {
	if( strcmp(attr, "master") == 0 ){
		ID_CHIP_ADDR = (id == 1) ? (KEY_FUN_I2C_ADDR| SLAVER_I2C_ADDR_BIT) : KEY_FUN_I2C_ADDR;
	}
}

#pragma warning(disable: 4996)   

#include "json/json.h"
typedef Json::Writer JsonWriter;
typedef Json::Reader JsonReader;
typedef Json::Value  JsonValue;



DWORD fsize(FILE* fp) {
	DWORD fpos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	DWORD fsz = ftell(fp);
	fseek(fp, fpos, SEEK_SET);
	return fsz;
}
int getIntValue(JsonValue& jval) {
	if (jval.isNumeric()) return jval.asInt();
	if (jval.isString()) {
		const char* str = jval.asCString();
		str += strspn(str, " ");
		int hex = strspn(str, "0xX");
		if (hex != 0) {
			return atox(str, 1);
		}
		else {
			return atoi(str);
		}
	}
	return -1;
}

int getKeyValue(const char * skey, JsonValue& jnode, std::vector<BYTE> &list) {
	if (jnode.isMember(skey)) {
		JsonValue jval = jnode[skey];
		if (jval.isArray()) {
			for (int i = 0; i < jval.size(); i++) {
				list.push_back(getIntValue(jval[i]));
			}
		}
		else {
			list.push_back(getIntValue(jval));
		}
		return list.size();
	}
	return -1;
}



void load_reg_table(JsonValue & jreg, REG_TABLE_LIST_T& reg_list) {
	REG_TABLE_T reg_table;
	std::vector<BYTE> reg_addr ;
	if (getKeyValue("addr", jreg, reg_addr) < 0 ) return;
	JsonValue jval ;
	std::vector<int> regval;
	const char* keywords[] = { "value" , "setbit", "clrbit", "read" };
	for (int i = 0; i < sizeof(keywords) / sizeof(const char*); i++) {
		reg_table.val.clear();
		if (getKeyValue(keywords[i], jreg, reg_table.val) >= 0) {
			reg_table.op = i;
			reg_table.addr = reg_addr[0];
			reg_list.push_back(reg_table);
		}
	}
}

void logList(const char * info, std::vector<BYTE>& vlist) {
	stringstream sinfo;
	sinfo << info<< ":[";
	for (auto v: vlist) {
		sinfo << hex << (int)v << ",";
	}
	sinfo << "]";
	char log[128];
	sinfo >> log;
	USES_CONVERSION;
	logInfo(A2W(log));
}


void json_proc_dev ( JsonValue & jdev , CHIP_TAB_LIST_T& chiplist) {
	CHIP_TABLE_T chip;
	if (getKeyValue("addr", jdev, chip.chipid) < 0) return;
	if (!jdev.isMember("register")) return;
	JsonValue jregs = jdev["register"];
	
	if (jregs.isArray()) {
		for (int i = 0; i < jregs.size(); i++) {
			load_reg_table(jregs[i], chip.reg_table);
		}
	}
	else {
		load_reg_table(jregs, chip.reg_table);
	}
	chiplist.push_back(chip);

}

void load_script(char* script, CHIP_TAB_LIST_T& chiplist) {
	FILE* fp = fopen(script, "rb");
	if (fp == NULL) return;
	DWORD flen = fsize(fp);
	DWORD dBufferLength_string = 512;
	std::vector<char> stringBuff(flen);
	fread(&stringBuff[0], 1, flen, fp);
	fclose(fp);
	//retCode = GetHttpData_ServerInfo(myheader, HTTP_QUERY_DATE, &stringBuff[0], dBufferLength_string, proxy);
	std::string msg(stringBuff.begin(), stringBuff.end());//»òmyStr.assign(stringBuff.begin(), stringBuff.end());

	JsonReader freader;
	JsonValue rootr;
	freader.parse(msg, rootr);
	JsonValue devs = rootr["pca9505"];

	if (devs.isArray()) {
		for (int i = 0; i < devs.size(); i++) {
			json_proc_dev(devs[i], chiplist);
		}
	}
	else {
		json_proc_dev(devs, chiplist);
	}
}

void CPca9505::RunScript(char* script)
{
	load_script(script, m_chip_list);
	Run(&m_chip_list);

}

LRESULT CPca9505::Run(void* p_op)
{
	CHIP_TAB_LIST_T* chip_op = (CHIP_TAB_LIST_T*)p_op;
	for (auto chip : *chip_op) {
		for (auto chipid : chip.chipid)
		{
			chipid |= 0x20;
			logInfo(L"ChipID %X:", chipid);
			for (auto& it : chip.reg_table) {
				const char* tips[] = { "Reg(write):","Reg(setbit):","Reg(clrbit):","Reg(read):" };
				logInfo(L"   %S [%X]=>", tips[it.op], it.addr);
				BYTE regAddr = it.addr | REG_AUTO_INC;
				logList("    Data:", it.val);
				BYTE cache[IO_GROUP_MAX];
				if (it.op == REG_OP_WRITE) {
					memcpy(cache, &it.val[0], it.val.size());
				}
				else {
					int rlen = (it.op == REG_OP_READ) ? IO_GROUP_MAX : it.val.size();
					if (!I2C_Read(chipid | 0x20, regAddr, cache, rlen)) break;
					for (int i = 0; i < rlen; i++) {
						if (it.op == REG_OP_SETBIT)
							cache[i] |= it.val[i];
						else if (it.op == REG_OP_CLRBIT)
							cache[i] &= it.val[i];
						else
							cout << ((i == 0) ? "0x" : ",0x") << hex << (int)cache[i];
					}
					if (it.op == REG_OP_READ) {
						cout << endl;
						continue;
					}
				}

				if (!I2C_Write(chipid | 0x20, regAddr, cache, it.val.size()))
					break;
			}
		}
	}
	return 0;
}
