#include "pch.h"
#include "CFtDevice.h"
#include "ftd2xx.h"
#include "util.h"



#define MID_ECHO_COMMAND_ONCE			0
#define MID_ECHO_COMMAND_CONTINUOUSLY   1
#define MID_ECHO_CMD_1					0xAA
#define MID_ECHO_CMD_2					0xAB
#define MID_BAD_COMMAND_RESPONSE        0xFA
#define MID_CMD_NOT_ECHOED				0
#define MID_CMD_ECHOED					1

/*clock*/
#define MID_SET_LOW_BYTE_DATA_BITS_CMD	0x80
#define MID_GET_LOW_BYTE_DATA_BITS_CMD	0x81
#define MID_SET_HIGH_BYTE_DATA_BITS_CMD	0x82
#define MID_GET_HIGH_BYTE_DATA_BITS_CMD	0x83
#define MID_SET_CLOCK_FREQUENCY_CMD		0x86
#define MID_SET_LOW_BYTE_DATA_BITS_DATA 0x13
#define MPSSE_CMD_GET_DATA_BITS_LOWBYTE		0x81
#define MPSSE_CMD_GET_DATA_BITS_HIGHBYTE	0x83

#define MPSSE_CMD_SEND_IMMEDIATE			0x87
#define MPSSE_CMD_ENABLE_3PHASE_CLOCKING	0x8C
#define MPSSE_CMD_DISABLE_3PHASE_CLOCKING	0x8D
#define MPSSE_CMD_ENABLE_DRIVE_ONLY_ZERO	0x9E

#define MID_SET_HIGH_BYTE_DATA_BITS_DATA 0x0F

#define CHECK_STATUS(exp) 
#define DBG  printf 


DWORD dwClockDivisor = 0x00C8;		// 100khz

CFtDevice::CFtDevice(FT_DEVICE_LIST_INFO_NODE* info, int open_idx)
{
	m_devInfo = *info;
	m_FtdiID = info->LocId;
	m_lowByte = 0;
	m_highByte = 0;
	m_ftHandle = INVALID_HANDLE_VALUE;
	m_OpenIndex = open_idx;
	m_ComPort = -1;
}
CFtDevice::~CFtDevice()
{
	if (m_ftHandle != INVALID_HANDLE_VALUE) {
		FT_SetBitMode(m_ftHandle, 0x0, 0x00);
		// Reset the port to disable MPSSE
		FT_Close(m_ftHandle);
	}
}

LRESULT CFtDevice::ReadChn( int idx)
{
	if (m_ftHandle == INVALID_HANDLE_VALUE) return ERROR_DEVICE_NOT_AVAILABLE;
	 FT_STATUS status;
	 BYTE byOutputBuffer[64];
	 BYTE byInputBuffer[64];
	 INT dwNumBytesToSend = 0;
	 DWORD dwNumBytesToRead = 0;
	 DWORD dwNumBytesSent = 0;
	 DWORD bytesToTransfer = 0;
	 DWORD dwNumBytesRead = 0;
	 byOutputBuffer[dwNumBytesToSend++] = 0x81;
	 byOutputBuffer[dwNumBytesToSend++] = 0x87;

	 byOutputBuffer[dwNumBytesToSend++] = 0x83;
	 byOutputBuffer[dwNumBytesToSend++] = 0x87;

	 status = FT_Write(m_ftHandle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
	 // Read the low GPIO byte
	 dwNumBytesToSend = 0;
	 // Reset output buffer pointer
	 Sleep(20);

	 status = FT_GetQueueStatus(m_ftHandle, &dwNumBytesToRead);
	 // Get the number of bytes in the
	 //	FT2232H receive buffer
	 status |= FT_Read(m_ftHandle, &byInputBuffer, dwNumBytesToRead, &dwNumBytesRead);
	 if ((status != FT_OK) & (dwNumBytesToRead != 1))
	 {
		 logError(L"Error - GPIO cannot be read");
		 return ERROR_READ_FAULT;
	 }
	 m_lowByte = byInputBuffer[0];
	 m_highByte = byInputBuffer[1];
	 logInfo(L" R: Dev[%d/%x] 0x%X , 0x%X\n",  m_OpenIndex, m_FtdiID, m_lowByte, m_highByte);

	 return S_OK;
}



LRESULT CFtDevice::Write(int idx) {
	if (m_ftHandle == INVALID_HANDLE_VALUE) return ERROR_DEVICE_NOT_AVAILABLE;
	int dwNumBytesToSend = 0;			//Clear output buffer
	DWORD dwNumBytesSent = 0;
	BYTE OutputBuffer[1024];			//Buffer to hold MPSSE commands and data to be sent to FT232H

	if (idx & 1 ){
		// Set the idle states for the AD lines
		OutputBuffer[dwNumBytesToSend++] = MID_SET_LOW_BYTE_DATA_BITS_CMD;	// Command to set directions of ADbus and data values for pins set as o/p
		OutputBuffer[dwNumBytesToSend++] = m_lowByte;    // Set all 8 lines to high level (only affects pins which are output)
		OutputBuffer[dwNumBytesToSend++] = 0xFF;	// Set all pins as output except bit 2 which is the data_in
	}
	if (idx & 2) {
		// Set the idle states for the AC lines
		OutputBuffer[dwNumBytesToSend++] = MID_SET_HIGH_BYTE_DATA_BITS_CMD;	// Command to set directions of ACbus and data values for pins set as o/p
		OutputBuffer[dwNumBytesToSend++] = m_highByte;	// Set all 8 lines to high level (only affects pins which are output)
		OutputBuffer[dwNumBytesToSend++] = 0xff;	// Only bit 6 is output
	}

	FT_STATUS ftStatus = FT_Write(m_ftHandle, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent);		//Send off the commands
	logInfo(L" W: Dev[%d/%d]: 0x%X , 0x%X\n", m_OpenIndex, m_FtdiID, m_lowByte, m_highByte);
	return S_OK;
}

LRESULT CFtDevice::GetRaw(BYTE* vl, BYTE* vh)
{
	ReadChn(0);
	if (vl) {
		*vl = m_lowByte;
	}

	if (vh) {
		*vh = m_highByte;
	}

	return S_OK;
}




LRESULT CFtDevice::Set(int raw)
{
	return LRESULT(0);
}



LRESULT CFtDevice::SyncIO(int mode, int con, IO_OP* op, BOOL bhwSync )
{
	if (m_ftHandle == INVALID_HANDLE_VALUE) Open();
	if (m_ftHandle == INVALID_HANDLE_VALUE) return ERROR_DEVICE_NOT_AVAILABLE;
	int gpiobase = con * 3+4;
	
	
	if (op->ret != S_OK) return op->ret;
	IO_VAL_RAW* rawval = (IO_VAL_RAW*)&op->val;

	if (mode == CFtdiDriver::IO_WRITE) {
		if (rawval->rawInd == RAW_FARMAT ) {
			DWORD rawv = rawval->rawVal << 4;;
			m_lowByte &= ~0xF0;
			m_lowByte |= rawv & 0xF0;
			m_highByte = (rawv >> 8) & 0xFF;
			if( bhwSync) Write(3);
		}
		else {
			char* pin = op->val.pin;
			for (int i = 0; i < IO_MAX_KEY; i++,pin++) {
				int gpionum = gpiobase + i;
				BYTE* pcache = (gpionum < 8) ? &m_lowByte : &m_highByte;
				gpionum = gpionum % 8;
				if (*pin >= 0) {
					*pcache &= ~(1 << gpionum);
					if (*pin) {
						*pcache |= 1 << gpionum;
					}
				}
			}
			if (bhwSync) Write(3);
		}
	}
	else {
		op->ret = S_OK;

		if (bhwSync) op->ret = ReadChn(0);

		if (op->ext_val == NULL) op->ext_val = op->val.pin;
		for (int i = 0; i < IO_MAX_KEY; i++, op->ext_val++) {
			int gpionum = gpiobase + i;
			BYTE* pcache = (gpionum < 8) ? &m_lowByte : &m_highByte;
			gpionum = gpionum % 8;
			*op->ext_val = 0;
			if (*pcache & (1 << gpionum) ){
				*op->ext_val |= 1;// << gpionum;
			}
		}
	}
	return S_OK;
}


LRESULT CFtDevice::SetGPIO(BYTE val)
{
	m_lowByte &= 0xF0;
	m_lowByte |= val;
	return  Write(1);
}

LRESULT CFtDevice::SetGPIO(int num, BYTE val)
{
	m_lowByte &= ~(1<<num);
	if( val )
		m_lowByte |= (1 << num);
	return  Write(1);
}

int CFtDevice::isMpsseMode()
{
	DWORD dwCount;
	FT_STATUS ftStatus;
	int dwNumBytesToSend = 0;			//Clear output buffer
	DWORD dwNumBytesSent = 0;
	BYTE OutputBuffer[64];			// Buffer to hold MPSSE commands and data to be sent to FT232H
	BYTE InputBuffer[64];				// Buffer to hold Data bytes read from FT232H
	DWORD dwNumInputBuffer;			// Number of bytes which we want to read
	DWORD dwNumBytesRead;			// Number of bytes actually read
	DWORD ReadTimeoutCounter;		// Used as a software timeout counter when the code checks the Queue Status

	bool bCommandEchod = false;
	dwNumBytesToSend = 0;																// Used as an index to the buffer
	OutputBuffer[dwNumBytesToSend++] = 0xAB;											// Add an invalid command 0xAB
	ftStatus = FT_Write(m_ftHandle, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent);		// Send off the invalid command

	// Check if the bytes were sent off OK
	if (dwNumBytesToSend != dwNumBytesSent)
	{
		logError(L"Write timed out! \n");
		return 1;
	}


	// Now read the response from the FT232H. It should return error code 0xFA followed by the actual bad command 0xAA
	// Wait for the two bytes to come back 

	dwNumInputBuffer = 0;
	ReadTimeoutCounter = 0;

	ftStatus = FT_GetQueueStatus(m_ftHandle, &dwNumInputBuffer);						// Get number of bytes in the input buffer
	while ((dwNumInputBuffer < 2) && (ftStatus == FT_OK) && (ReadTimeoutCounter < 500))
	{
		ftStatus = FT_GetQueueStatus(m_ftHandle, &dwNumInputBuffer);	// Get number of bytes in the input buffer
		ReadTimeoutCounter++;
		Sleep(1);													// short delay
	}

	// If the loop above exited due to the byte coming back (not an error code and not a timeout)
	// then read the bytes available and check for the error code followed by the invalid character
	if ((ftStatus == FT_OK) && (ReadTimeoutCounter < 500))
	{
		ftStatus = FT_Read(m_ftHandle, &InputBuffer, dwNumInputBuffer, &dwNumBytesRead); // Now read the data

		// Check if we have two consecutive bytes in the buffer with value 0xFA and 0xAB
		bCommandEchod = false;
		for (dwCount = 0; dwCount < dwNumBytesRead - 1; dwCount++)
		{
			if ((InputBuffer[dwCount] == BYTE(0xFA)) && (InputBuffer[dwCount + 1] == BYTE(0xAB)))
			{
				bCommandEchod = true;
				break;
			}
		}
	}
	// If the device did not respond correctly, display error message and exit.

	if (bCommandEchod == false)
	{
		logError(L"fail to synchronize MPSSE with command 0xAB \n");
		return 1;
	}	
	return 0;
}


LRESULT CFtDevice::Open()
{
	DWORD dwCount;
	FT_STATUS ftStatus;
	// Open the UM232H module by it's description in the EEPROM
	// Note: See FT_OpenEX in the D2xx Programmers Guide for other options available
	//ftStatus = FT_OpenEx("Dual RS232-HS B", FT_OPEN_BY_DESCRIPTION, &ftHandle);
	ftStatus = FT_Open(m_OpenIndex, &m_ftHandle);

	// Check if Open was successful
	if (ftStatus != FT_OK)
	{
		printf("Can't open FT232H device! \n");
		return 1;
	}
	else
	{
		int dwNumBytesToSend = 0;			//Clear output buffer
		DWORD dwNumBytesSent = 0;
		BYTE OutputBuffer[256];			// Buffer to hold MPSSE commands and data to be sent to FT232H
		BYTE InputBuffer[256];				// Buffer to hold Data bytes read from FT232H
		DWORD dwNumInputBuffer;			// Number of bytes which we want to read
		DWORD dwNumBytesRead;			// Number of bytes actually read
		DWORD ReadTimeoutCounter;		// Used as a software timeout counter when the code checks the Queue Status


		// #########################################################################################
		// After opening the device, Put it into MPSSE mode
		// #########################################################################################
		// Print message to show port opened successfully
		logInfo(L"Successfully opened FT232H device! \n");

		// Reset the FT232H
		ftStatus |= FT_ResetDevice(m_ftHandle);

		// Purge USB receive buffer ... Get the number of bytes in the FT232H receive buffer and then read them
		ftStatus |= FT_GetQueueStatus(m_ftHandle, &dwNumInputBuffer);
		if ((ftStatus == FT_OK) && (dwNumInputBuffer > 0))
		{
			FT_Read(m_ftHandle, &InputBuffer, dwNumInputBuffer, &dwNumBytesRead);
		}

		ftStatus = FT_GetComPortNumber(m_ftHandle, &m_ComPort);
		UCHAR bInMpsseMode = isMpsseMode();
		
		logInfo(L"Device COM port is: %d", m_ComPort);
		if ( bInMpsseMode != 0 ) {
			ftStatus |= FT_SetUSBParameters(m_ftHandle, 65536, 65535);			// Set USB request transfer sizes
			ftStatus |= FT_SetChars(m_ftHandle, false, 0, false, 0);				// Disable event and error characters
			ftStatus |= FT_SetTimeouts(m_ftHandle, 5000, 5000);					// Set the read and write timeouts to 5 seconds
			ftStatus |= FT_SetLatencyTimer(m_ftHandle, 16);						// Keep the latency timer at default of 16ms
			ftStatus |= FT_SetBitMode(m_ftHandle, 0x0, 0x00); 					// Reset the mode to whatever is set in EEPROM
			ftStatus |= FT_SetBitMode(m_ftHandle, 0x0, 0x02);	 					// Enable MPSSE mode
			bInMpsseMode = isMpsseMode();
			logInfo(L"Set MPSSE mode: %d ", bInMpsseMode);
		}



		// Inform the user if any errors were encountered
		if (ftStatus != FT_OK)
		{
			logError(L"failure to initialize FT232H device! \n");
			return 1;
		}

		Sleep(50);

		// #########################################################################################
		// Synchronise the MPSSE by sending bad command AB to it
		// #########################################################################################


		//printf("MPSSE synchronized with ECHO command \n");

		// #########################################################################################
		// Configure the MPSSE settings
		// #########################################################################################

		dwNumBytesToSend = 0;							// Clear index to zero
		OutputBuffer[dwNumBytesToSend++] = 0x8A; 		// Disable clock divide-by-5 for 60Mhz master clock
		OutputBuffer[dwNumBytesToSend++] = 0x97;		// Ensure adaptive clocking is off
		OutputBuffer[dwNumBytesToSend++] = 0x8C; 		// Enable 3 phase data clocking, data valid on both clock edges for I2C

		OutputBuffer[dwNumBytesToSend++] = 0x9E; 		// Enable the FT232H's drive-zero mode on the lines used for I2C ...
		OutputBuffer[dwNumBytesToSend++] = 0x07;		// ... on the bits 0, 1 and 2 of the lower port (AD0, AD1, AD2)...
		OutputBuffer[dwNumBytesToSend++] = 0x00;		// ...not required on the upper port AC 0-7

		OutputBuffer[dwNumBytesToSend++] = 0x85;		// Ensure internal loopback is off

		ftStatus = FT_Write(m_ftHandle, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent);	// Send off the commands

		// Now configure the dividers to set the SCLK frequency which we will use
		// The SCLK clock frequency can be worked out by the algorithm (when divide-by-5 is off)
		// SCLK frequency  = 60MHz /((1 +  [(1 +0xValueH*256) OR 0xValueL])*2)
		dwNumBytesToSend = 0;													// Clear index to zero
		OutputBuffer[dwNumBytesToSend++] = 0x86; 								// Command to set clock divisor
		OutputBuffer[dwNumBytesToSend++] = dwClockDivisor & 0xFF;				// Set 0xValueL of clock divisor
		OutputBuffer[dwNumBytesToSend++] = (dwClockDivisor >> 8) & 0xFF;		// Set 0xValueH of clock divisor
		ftStatus = FT_Write(m_ftHandle, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent);	// Send off the commands

		Sleep(20);																// Short delay 	
	}
	
	//SetGpio(0x00, 0x00);
	//SetGpio(0xFF, 0xFF);
	ReadChn(0); //dumy read
	ReadChn(0);

	//FT_Close(m_ftHandle);

	return S_OK;
}

LRESULT CFtDevice::Close(void)
{
	FT_Close(m_ftHandle);
	m_ftHandle = INVALID_HANDLE_VALUE;
	return LRESULT(0);
}


CFtBoard::CFtBoard(int idx) {
	m_index = idx;
	m_deviceNum = 0;
	m_strPortInfo = L"";
	m_BoardID = (int)(((DWORD)-1)>>1);
}

CFtBoard::~CFtBoard()
{
	for (map<fdtiid, CFtDevice*>::iterator it = m_Devices.begin(); it != m_Devices.end(); ) {
		if( it->second )
			delete it->second;
		m_Devices.erase(it);
	}
}

void CFtBoard::AddDevice(FT_DEVICE_LIST_INFO_NODE* pdevInfo, int open_idx)
{
	fdtiid devid = pdevInfo->LocId;
	map< fdtiid, CFtDevice*>::iterator iter = m_Devices.find(devid);
	if (iter == m_Devices.end()) {
		m_Devices[devid] = new CFtDevice(pdevInfo, open_idx);
	}	
	if (m_BoardID > devid ) {
		m_BoardID = devid;
	}
	m_deviceNum++;
}

void CFtBoard::DoMount(BOOL bMount)
{
	if (bMount != m_bMounted) 
	{
		m_strPortInfo.Empty();
		CString strInfo;
		for (map<fdtiid, CFtDevice*>::iterator it = m_Devices.begin(); it != m_Devices.end(); it++ ) {
			if (bMount) {
				LRESULT ret = it->second->Open();
				if (ret != S_OK) {
					SetFaultError(ret);
					return;
				}
				strInfo.Format(L"%d", it->second->m_ComPort);
				m_strPortInfo =  (m_strPortInfo.IsEmpty() ? strInfo : m_strPortInfo + L"/" + strInfo);

			}
			else
			{
				it->second->Close();
			}
		}
		m_bMounted = bMount;
	}
}

LRESULT CFtBoard::SyncIO(vector<IO_OP>& io_reqQue)
{
	LRESULT ret = S_OK;
	for (int i = 0; i < (int)io_reqQue.size(); i++) {
		int con = io_reqQue[i].val.con;
		if (con == IO_CON_GPIO) {
			int id = m_BoardID ;
			IO_GPIO*op = (IO_GPIO * )&io_reqQue[i].val;
			if ( op->israw ) {
				m_Devices[id]->SetGPIO(op->val & 0xF);
				m_Devices[id+1]->SetGPIO( (op->val>>4) & 0xF);
			}
			else {
				if( op->bit < 4 )
					m_Devices[id]->SetGPIO((int)op->bit,  op->val);
				else 
					m_Devices[id+1]->SetGPIO((int)op->bit-4, op->val);
			}
		}
		else {
			//Lowbyte : con0,con1
			if (con == IO_ALL_CON) {
				for (int c = 0; c < IO_CON_MAX; c++)
					SyncIO(c, &io_reqQue[i]);
			}
			else
				SyncIO(con, &io_reqQue[i]);
		}
	}
	return ret;
}

LRESULT CFtBoard::SyncIO(int con, IO_OP* op)
{
	LRESULT ret = S_OK;
	if (con == IO_CON_GPIO) {
		int id = m_BoardID;
		IO_GPIO* ogpio = (IO_GPIO*)&op->val;
		if (ogpio->israw) {
			m_Devices[id]->SetGPIO(ogpio->val & 0xF);
			m_Devices[id + 1]->SetGPIO((ogpio->val >> 4) & 0xF);
		}
		else {
			if (ogpio->bit < 4)
				m_Devices[id]->SetGPIO((int)ogpio->bit, ogpio->val);
			else
				m_Devices[id + 1]->SetGPIO((int)ogpio->bit - 4, ogpio->val);
		}
	}
	else if (con == IO_ALL_CON) {
		for (int c = 0; c < IO_CON_MAX; c++)
			SyncIO(c, op);
	}
	else {
		int idx = con / 4;
		if (idx < m_deviceNum) {
			int id = m_BoardID + idx;
			LRESULT err = m_Devices[id]->SyncIO(CFtdiDriver::IO_WRITE, con % 4, op);
			if (err != S_OK) {
				logError(L"Process  IO Req faile with:%d(%s)", err, (LPCTSTR)ErrorString(err));
				ret = err;
			}
			op->ret = err;
		}
	}
	return ret;
}


LRESULT CFtBoard::Display(int con, IO_VAL* io_val, int* items)
{
	if (con == IO_CON_GPIO) {
		*items = 1;
	}
	else  if (con == IO_ALL_CON){
		*items = 1 + 8;
	}
	else {
		*items = 1;
	}
	if (io_val == NULL) return ERROR_INVALID_ADDRESS;

	
	if (con == IO_CON_GPIO) {
		BYTE vl, vh;
		m_Devices[m_BoardID]->GetRaw(&vl, NULL);
		m_Devices[m_BoardID+1]->GetRaw(&vh, NULL);
		vl = (((vh << 4) & 0xF0) | (vl & 0x0F));
		IO_GPIO* vgpio = (IO_GPIO*)io_val;
		vgpio[0].con = IO_CON_GPIO;
		vgpio[0].bit = 0xFF;
		vgpio[0].val = vl;
	}
	else if (con == IO_ALL_CON) {
		BYTE val[4];
		m_Devices[m_BoardID]->GetRaw(&val[0], &val[1]);
		m_Devices[m_BoardID + 1]->GetRaw(&val[2], &val[3]);
		for (int c = IO_CON0; c < IO_CON_MAX; c++) {
			io_val[c].con = c;
			char* pkey = (char *)io_val[c].pin;
			IO_OP op = { m_index, pkey, (char)c, {0} };
			if (c < 4) {
				m_Devices[m_BoardID]->SyncIO(CFtdiDriver::IO_READ, c, &op, FALSE);
			}
			else {
				m_Devices[m_BoardID + 1]->SyncIO(CFtdiDriver::IO_READ, c-4, &op, FALSE);
			}
		}
		BYTE v = (((val[2] << 4) & 0xF0) | (val[0] & 0x0F));
		IO_GPIO* vgpio = (IO_GPIO*)&io_val[IO_CON_MAX];
		vgpio->con = IO_CON_GPIO;
		vgpio->bit = 0xFF;
		vgpio->val = v;

	}
	else {
		char* pkey = (char*)io_val[0].pin;
		IO_OP op = { m_index, pkey, con, {0} };
		if (con < 4) {
			m_Devices[m_BoardID]->SyncIO(CFtdiDriver::IO_READ, con, &op);
		}
		else {
			m_Devices[m_BoardID+1]->SyncIO(CFtdiDriver::IO_READ, con-4, &op);
		}
	}
	return S_OK;
}

