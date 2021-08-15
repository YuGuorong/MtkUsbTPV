#pragma once

#include "ftd2xx.h"
class CFT2232
{
public:
	CFT2232();
	~CFT2232();
	static DWORD  m_DevNum;
	static DWORD  m_LgoicDev;
	static void* m_pDevList;
	static LRESULT Scan(void);

	LRESULT Open(void);
	LRESULT Read(void);
	LRESULT Set(int con, int home, int kcol, int pwr);
	LRESULT Set(int gpio);
	LRESULT SetAll(int raw);
protected:
	void GetGpio();
	void SetGpio(BYTE lb, BYTE hb);
	int m_chnId;
	FT_HANDLE ftHandle;
	BYTE OutputBuffer[1024];			// Buffer to hold MPSSE commands and data to be sent to FT232H
	BYTE InputBuffer[1024];				// Buffer to hold Data bytes read from FT232H

	

	DWORD dwNumBytesSent ;			// Holds number of bytes actually sent (returned by the read function)

	DWORD dwNumInputBuffer ;			// Number of bytes which we want to read
	DWORD dwNumBytesRead ;			// Number of bytes actually read
	DWORD ReadTimeoutCounter ;		// Used as a software timeout counter when the code checks the Queue Status

	BYTE ByteDataRead[4];				// Array for storing the data which was read from the I2C Slave
	BOOL DataInBuffer ;				// Flag which code sets when the GetNumBytesAvailable returned is > 0 
	BYTE DataByte ;					// Used to store data bytes read from and written to the I2C Slave
	bool bCommandEchod;

	BYTE m_lowByte;
	BYTE m_highByte;

};

