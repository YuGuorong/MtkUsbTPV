#pragma once
#include "ftd2xx.h"
#include "ftdi_common.h"
#include <vector>
#include <map>
#include "CSimpleLog.h"


using namespace std;
typedef  DWORD  fdtiid;

typedef int (*fLog)(const char* fmt, ...);
typedef int (*Print_T)(const char* fmt, ...);
extern Print_T  FdtiPrint;

using namespace std;
class CFtdiDriver;
class CFtDevice;
class CTpvBoard;


const LPCTSTR strCustomKeys[] = {
	L"HOME",
	L"KCOL",
	L"PWRKEY",
};
enum {
	IO_CON_UNKNOWN = -3,
	IO_ALL_CON = -1,
	IO_CON0 = 0,
	IO_CON1,
	IO_CON2,
	IO_CON3,
	IO_CON4,
	IO_CON5,
	IO_CON6,
	IO_CON7,
	IO_CON_MAX,
	IO_CON_GPIO ,
};
enum {
	IO_PWRKEY = 0,
	IO_KCOL,
	IO_HOME,
	IO_MAX_KEY,
	IO_MAX_GPIO = 8
};
typedef struct te_op_io IO_OP;

class CFtdiDriver {
public:
	enum {
		IO_WRITE = 0,
		IO_READ,
	};


public:
	DWORD  m_DevChannels;
	FT_DEVICE_LIST_INFO_NODE* m_pDevInfoList;
	
	map<int, CTpvBoard*> m_BoardList;
	int m_BoardIdxNum;
public:
	CFtdiDriver();
	~CFtdiDriver();
	LRESULT Scan(BOOL blog);
	LRESULT MountDevices();
	static CFtdiDriver* GetDriver();
	LRESULT Read(IO_OP* op);
	LRESULT Write(IO_OP* op);
	LRESULT SetAll(int index, BYTE val);
	void  Load();
	void  Save();

	CTpvBoard *FindBoard(fdtiid id, int index, LRESULT* err = NULL);
	void ShowDevices(void);
protected:
	LRESULT SyncIO(int mode, IO_OP* op);
	static CFtdiDriver* m_InstFtdi;
	void FreeBoardList();
};
#define IO_RAW    101
#define CON_RAW   99
#define GPIO_RAW     97
#define GPIO_CTL     95
#define CON_CTL      93

typedef struct te_gpio_val {
	char bit;
	BYTE val;
}GPIO_VAL;

typedef struct te_io_val {
	char con;
	BYTE fmt; //CON_RAW CON_RAW, GPIO_RAW, GPIO, c
	union {
		DWORD raw;
		GPIO_VAL gpio;
		char pin[IO_MAX_KEY];
	}v;
}IO_VAL;

typedef struct te_op_io {
	int index;
	char* ext_val;
	IO_VAL val;
	LRESULT ret;
}IO_OP;

extern const IO_OP def_request;
