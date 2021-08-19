#pragma once

#include "ftd2xx.h"
#include <map>
#include <vector>
#include "CFdtiDriver.h"


class CFtDevice
{
public:
	CFtDevice(int ftID);
	~CFtDevice();

	LRESULT Open(void);
	LRESULT Read(void);
	LRESULT Set(int con, int home, int kcol, int pwr);
	LRESULT Set(int gpio);
	LRESULT SetAll(int raw);

	BOOL bcheckMount;
protected:

	void GetGpio();
	void SetGpio(BYTE lb, BYTE hb);
	fdtiid m_FtdiID;

	FT_HANDLE ftHandle;
	
	BYTE m_lowByte;
	BYTE m_highByte;
};

class CFtBoard {
public:
	CFtBoard();
	~CFtBoard();
	INT m_index;
	
	vector <fdtiid>m_ftdis;
	
};


