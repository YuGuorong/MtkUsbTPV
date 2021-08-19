#pragma once
#include "ftd2xx.h"
#include <vector>
#include <map>

using namespace std;
typedef  int  fdtiid;


using namespace std;
class CFtDevice;
class CFtBoard;

typedef struct te_op_io {
	int index;
	int con;
	int io_pin;
	int io_val;
}IO_OP;

class CFtdiDriver {
public:
	DWORD  m_DevChannels;
	FT_DEVICE_LIST_INFO_NODE* m_pDevInfoList;
	vector<vector<fdtiid>*> m_BoardList;
	map<int, CFtDevice*> m_Devices;
public:
	CFtdiDriver();
	~CFtdiDriver();
	LRESULT Scan(BOOL blog);
	LRESULT MountDevices();
	static CFtdiDriver* CetDriver();
	LRESULT Read(IO_OP* op);
	LRESULT Write(IO_OP* op);
	LRESULT SetAll(BYTE val);
	void ShowDevices(void);
protected:
	static CFtdiDriver* m_InstFtdi;

public:
	enum {
		IO_POWER_KEY = 1,
		IO_KCOL = 2,
		IO_HOME_KEY = 4,
	};
};

