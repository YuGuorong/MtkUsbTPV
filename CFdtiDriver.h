#pragma once
#include "ftd2xx.h"
#include <vector>
#include <map>
#include "CSimpleLog.h"

using namespace std;
typedef  int  fdtiid;

typedef int (*fLog)(const char* fmt, ...);

using namespace std;
class CFtdiDriver;
class CFtDevice;
class CFtBoard;



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
	
	map<int, CFtBoard*> m_BoardList;
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

	CFtBoard *FindBoard(fdtiid id, int index, LRESULT* err = NULL);
	void ShowDevices(void);
protected:
	LRESULT SyncIO(int mode, IO_OP* op);
	static CFtdiDriver* m_InstFtdi;
};

#define RAW_FARMAT   99

typedef struct te_io_gpio {
	char con;
	BYTE israw;
	BYTE bit;
	BYTE val;
}IO_GPIO;

#define RAW_FARMAT   99
typedef struct te_io_raw {
	char con;
	BYTE rawInd; //RAW_FARMAT 
	WORD rawVal;
}IO_VAL_RAW;

typedef struct te_io_val {
	char con;
	char pin[IO_MAX_KEY];
}IO_VAL;

typedef struct te_op_io {
	int index;
	char* ext_val;
	IO_VAL val;
	LRESULT ret;
}IO_OP;