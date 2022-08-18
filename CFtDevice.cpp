#include "pch.h"
#include "CFtDevice.h"
#include "ftd2xx.h"
#include "util.h"
#include "ftdi_common.h"


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

LRESULT CFtDevice::UpdateRaw(void)
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
	logInfo(L" W(%d): Dev[%d/%d]: 0x%X , 0x%X\n", ftStatus, m_OpenIndex, m_FtdiID, m_lowByte, m_highByte);
	if (ftStatus != FT_OK) {
		SetFaultError(ftStatus);
		return ftStatus;
	}
	return S_OK;
}

// AD0 (SCL) is output driven low
// AD1 (DATA OUT) is output high (open drain)
// AD2 (DATA IN) is input (therefore the output value specified is ignored)
// AD3 to AD7 are inputs driven high (not used in this application)
void CFtDevice::SetI2CLinesIdle(void)
{
	if (m_ftHandle == INVALID_HANDLE_VALUE) return ;
	int dwNumBytesToSend = 0;			//Clear output buffer
	DWORD dwNumBytesSent = 0;
	BYTE OutputBuffer[256];			//Buffer to hold MPSSE commands and data to be sent to FT232H

	dwNumBytesToSend = 0;			//Clear output buffer

	// Set the idle states for the AD lines
	OutputBuffer[dwNumBytesToSend++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;	// Command to set directions of ADbus and data values for pins set as o/p
	OutputBuffer[dwNumBytesToSend++] = 0xFF;    // Set all 8 lines to high level (only affects pins which are output)
	OutputBuffer[dwNumBytesToSend++] = 0xFB;	// Set all pins as output except bit 2 which is the data_in

	FT_Write(m_ftHandle, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent);		//Send off the commands
}

void CFtDevice::SetI2CStart(void)
{
	if (m_ftHandle == INVALID_HANDLE_VALUE) return ;
	int dwNumBytesToSend = 0;			//Clear output buffer
	DWORD dwNumBytesSent = 0;
	BYTE OutputBuffer[256];			//Buffer to hold MPSSE commands and data to be sent to FT232H
	// Pull Data line low, leaving clock high (open-drain)
	for (int dwCount = 0; dwCount < 4; dwCount++)	// Repeat commands to ensure the minimum period of the start hold time is achieved
	{
		OutputBuffer[dwNumBytesToSend++] = 0x80;	// Command to set directions of ADbus and data values for pins set as o/p
		OutputBuffer[dwNumBytesToSend++] = 0xFD;	// Bring data out low (bit 1)
		OutputBuffer[dwNumBytesToSend++] = 0xFB;	// Set all pins as output except bit 2 which is the data_in
	}

	// Pull Clock line low now, making both clcok and data low
	for (int dwCount = 0; dwCount < 4; dwCount++)	// Repeat commands to ensure the minimum period of the start setup time is achieved
	{
		OutputBuffer[dwNumBytesToSend++] = 0x80; 	// Command to set directions of ADbus and data values for pins set as o/p
		OutputBuffer[dwNumBytesToSend++] = 0xFC; 	// Bring clock line low too to make clock and data low
		OutputBuffer[dwNumBytesToSend++] = 0xFB;	// Set all pins as output except bit 2 which is the data_in
	}

	FT_Write(m_ftHandle, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent);		//Send off the commands
}

void CFtDevice::SetI2CStop(void)
{
	if (m_ftHandle == INVALID_HANDLE_VALUE) return ;
	int dwNumBytesToSend = 0;			//Clear output buffer
	DWORD dwNumBytesSent = 0;
	BYTE OutputBuffer[256];			//Buffer to hold MPSSE commands and data to be sent to FT232H
	dwNumBytesToSend = 0;			//Clear output buffer
	DWORD dwCount;

	// Initial condition for the I2C Stop - Pull data low (Clock will already be low and is kept low)
	for (dwCount = 0; dwCount < 4; dwCount++)		// Repeat commands to ensure the minimum period of the stop setup time is achieved
	{
		OutputBuffer[dwNumBytesToSend++] = 0x80;	// Command to set directions of ADbus and data values for pins set as o/p
		OutputBuffer[dwNumBytesToSend++] = 0xFC;	// put data and clock low
		OutputBuffer[dwNumBytesToSend++] = 0xFB;	// Set all pins as output except bit 2 which is the data_in
	}

	// Clock now goes high (open drain)
	for (dwCount = 0; dwCount < 4; dwCount++)		// Repeat commands to ensure the minimum period of the stop setup time is achieved
	{
		OutputBuffer[dwNumBytesToSend++] = 0x80;	// Command to set directions of ADbus and data values for pins set as o/p
		OutputBuffer[dwNumBytesToSend++] = 0xFD;	// put data low, clock remains high (open drain, pulled up externally)
		OutputBuffer[dwNumBytesToSend++] = 0xFB;	// Set all pins as output except bit 2 which is the data_in
	}

	// Data now goes high too (both clock and data now high / open drain)
	for (dwCount = 0; dwCount < 4; dwCount++)	// Repeat commands to ensure the minimum period of the stop hold time is achieved
	{
		OutputBuffer[dwNumBytesToSend++] = 0x80;	// Command to set directions of ADbus and data values for pins set as o/p
		OutputBuffer[dwNumBytesToSend++] = 0xFF;	// both clock and data now high (open drain, pulled up externally)
		OutputBuffer[dwNumBytesToSend++] = 0xFB;	// Set all pins as output except bit 2 which is the data_in
	}
	FT_Write(m_ftHandle, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent);		//Send off the commands
}

BOOL CFtDevice::SendAddrAndCheckACK(BYTE dwDataSend, BOOL Read)
{
	if (m_ftHandle == INVALID_HANDLE_VALUE) return ERROR_DEVICE_NOT_AVAILABLE;
	DWORD dwNumBytesSent = 0;
	BYTE OutputBuffer[256], InputBuffer[16];			//Buffer to hold MPSSE commands and data to be sent to FT232H
	DWORD dwNumBytesToSend = 0, dwNumBytesRead= 0;			//Clear output buffer
	FT_STATUS ftStatus = FT_OK;
	
	dwDataSend = (Read == TRUE) ? ((dwDataSend << 1) | 0x01) : ((dwDataSend << 1) & 0xFE);

	OutputBuffer[dwNumBytesToSend++] = 0x11; 		// command to clock data bytes out MSB first on clock falling edge
	OutputBuffer[dwNumBytesToSend++] = 0x00;		// 
	OutputBuffer[dwNumBytesToSend++] = 0x00;		// Data length of 0x0000 means 1 byte data to clock out
	OutputBuffer[dwNumBytesToSend++] = dwDataSend;	// Actual byte to clock out

	// Put I2C line back to idle (during transfer) state... Clock line driven low, Data line high (open drain)
	OutputBuffer[dwNumBytesToSend++] = 0x80;		// Command to set lower 8 bits of port (ADbus 0-7 on the FT232H)
	OutputBuffer[dwNumBytesToSend++] = 0xFE;		// Set the value of the pins (only affects those set as output)
	OutputBuffer[dwNumBytesToSend++] = 0xFB;		// Set the directions - all pins as output except Bit2(data_in)

	OutputBuffer[dwNumBytesToSend++] = 0x22; 	// Command to clock in bits MSB first on clock rising edge
	OutputBuffer[dwNumBytesToSend++] = 0x00;	// Length of 0x00 means to scan in 1 bit

	// This command then tells the MPSSE to send any results gathered back immediately
	OutputBuffer[dwNumBytesToSend++] = 0x87;	//Send answer back immediate command

	ftStatus = FT_Write(m_ftHandle, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent);		//Send off the commands

	//Check if ACK bit received by reading the byte sent back from the FT232H containing the ACK bit
	ftStatus = FT_Read(m_ftHandle, InputBuffer, 1, &dwNumBytesRead);  	//Read one byte from device receive buffer
	if ((ftStatus != FT_OK) || (dwNumBytesRead == 0) || ((InputBuffer[0] & 0x01) != 0x00))
	{
		logError(L"Failed to get ACK from I2C Slave \n");
		return FALSE; //Error, can't get the ACK bit
	}

	return TRUE;		// Return True if the ACK was received
}

BOOL CFtDevice::SendByteAndCheckACK(BYTE dwDataSend)
{
	if (m_ftHandle == INVALID_HANDLE_VALUE) return ERROR_DEVICE_NOT_AVAILABLE;
	DWORD dwNumBytesSent = 0;
	BYTE OutputBuffer[256], InputBuffer[16];			//Buffer to hold MPSSE commands and data to be sent to FT232H
	DWORD dwNumBytesToSend = 0, dwNumBytesRead = 0;			//Clear output buffer
	FT_STATUS ftStatus = FT_OK;

	OutputBuffer[dwNumBytesToSend++] = 0x11; 		// command to clock data bytes out MSB first on clock falling edge
	OutputBuffer[dwNumBytesToSend++] = 0x00;		// 
	OutputBuffer[dwNumBytesToSend++] = 0x00;		// Data length of 0x0000 means 1 byte data to clock out
	OutputBuffer[dwNumBytesToSend++] = dwDataSend;	// Actual byte to clock out

	// Put I2C line back to idle (during transfer) state... Clock line driven low, Data line high (open drain)
	OutputBuffer[dwNumBytesToSend++] = 0x80;		// Command to set lower 8 bits of port (ADbus 0-7 on the FT232H)
	OutputBuffer[dwNumBytesToSend++] = 0xFE;		// Set the value of the pins (only affects those set as output)
	OutputBuffer[dwNumBytesToSend++] = 0xFB;		// Set the directions - all pins as output except Bit2(data_in)



	OutputBuffer[dwNumBytesToSend++] = 0x22; 	// Command to clock in bits MSB first on clock rising edge
	OutputBuffer[dwNumBytesToSend++] = 0x00;	// Length of 0x00 means to scan in 1 bit

	// This command then tells the MPSSE to send any results gathered back immediately
	OutputBuffer[dwNumBytesToSend++] = 0x87;	//Send answer back immediate command

	ftStatus = FT_Write(m_ftHandle, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent);		//Send off the commands

	// ===============================================================
	// Now wait for the byte which we read to come back to the host PC
	// ===============================================================

	DWORD dwNumInputBuffer = 0;
	DWORD ReadTimeoutCounter = 0;
	ftStatus = FT_GetQueueStatus(m_ftHandle, &dwNumInputBuffer);	// Get number of bytes in the input buffer
	while ((dwNumInputBuffer < 1) && (ftStatus == FT_OK) && (ReadTimeoutCounter < 500))
	{
		ftStatus = FT_GetQueueStatus(m_ftHandle, &dwNumInputBuffer);	// Get number of bytes in the input buffer
		ReadTimeoutCounter++;
		Sleep(1);													// short delay
	}

	// If the loop above exited due to the byte coming back (not an error code and not a timeout)
	if ((ftStatus == FT_OK) && (ReadTimeoutCounter < 500))
	{
		ftStatus = FT_Read(m_ftHandle, &InputBuffer, dwNumInputBuffer, &dwNumBytesRead); // Now read the data

		if (((InputBuffer[0] & 0x01) == 0x00))		//Check ACK bit 0 on data byte read out
		{
			return TRUE;							// Return True if the ACK was received
		}
	}
	return FALSE;									// Failed to get any data back or got an error code back


}

BOOL CFtDevice::ReadAndSendNAK(BYTE* pRdBuff, int ReadLen)
{
	if (m_ftHandle == INVALID_HANDLE_VALUE) return FALSE;
	if (pRdBuff == NULL) return FALSE;
	DWORD dwNumBytesSent = 0;
	BYTE OutputBuffer[256];			//Buffer to hold MPSSE commands and data to be sent to FT232H
	DWORD dwNumBytesToSend = 0, dwNumBytesRead = 0;			//Clear output buffer
	FT_STATUS ftStatus = FT_OK;

	for (int i = 0; i < ReadLen; i++) {
		// Read the first byte of data over I2C and ACK it
		//Clock one byte in
		OutputBuffer[dwNumBytesToSend++] = 0x20; 		// Command to clock data byte in MSB first on clock rising edge
		OutputBuffer[dwNumBytesToSend++] = 0x00;
		OutputBuffer[dwNumBytesToSend++] = 0x00;		// Data length of 0x0000 means 1 byte data to clock in
		// Clock out one bit...send ack bit as '0'
		OutputBuffer[dwNumBytesToSend++] = 0x13;		// Command to clock data bit out MSB first on clock falling edge
		OutputBuffer[dwNumBytesToSend++] = 0x00;		// Length of 0x00 means 1 bit
		OutputBuffer[dwNumBytesToSend++] = (i==(ReadLen-1))? 0xFF: 0x00;		// Data value to clock out is in bit 7 of this value
		// Put I2C line back to idle (during transfer) state... Clock line driven low, Data line high (open drain)
		OutputBuffer[dwNumBytesToSend++] = 0x80;		// Command to set lower 8 bits of port (ADbus 0-7 on the FT232H)
		OutputBuffer[dwNumBytesToSend++] = 0xFE;		// Set the value of the pins (only affects those set as output)
		OutputBuffer[dwNumBytesToSend++] = 0xFB;		// Set the directions - all pins as output except Bit2(data_in)
	}


	// This command then tells the MPSSE to send any results gathered back immediately
	OutputBuffer[dwNumBytesToSend++] = '\x87';		// Send answer back immediate command

	ftStatus = FT_Write(m_ftHandle, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent);		//Send off the commands

	// ===============================================================
	// Now wait for the 3 bytes which we read to come back to the host PC
	// ===============================================================

	DWORD dwNumInputBuffer = 0;
	DWORD ReadTimeoutCounter = 0;

	ftStatus = FT_GetQueueStatus(m_ftHandle, &dwNumInputBuffer);	// Get number of bytes in the input buffer

	while ((dwNumInputBuffer < (DWORD)ReadLen) && (ftStatus == FT_OK) && (ReadTimeoutCounter < 500))
	{
		ftStatus = FT_GetQueueStatus(m_ftHandle, &dwNumInputBuffer);	// Get number of bytes in the input buffer
		ReadTimeoutCounter++;
		Sleep(1);													// short delay
	}

	// If the loop above exited due to the bytes coming back (not an error code and not a timeout)
	// then read the bytes available and return True to indicate success
	if ((ftStatus == FT_OK) && (ReadTimeoutCounter < 500))
	{
		ftStatus = FT_Read(m_ftHandle, pRdBuff, dwNumInputBuffer, &dwNumBytesRead); // Now read the data
		return TRUE; // Indicate success
	}
	return FALSE;									// Failed to get any data back or got an error code back
}

BOOL CFtDevice::I2C_Write(BYTE chipAddr, BYTE regAddr, BYTE* pv, int len)
{
	//read back
	SetI2CLinesIdle();								// Set idle line condition
	SetI2CStart();
	BOOL bSucceed = SendAddrAndCheckACK(chipAddr, FALSE);
	if (bSucceed == 0) {
		return FALSE;
	}
	bSucceed = SendByteAndCheckACK(regAddr);
	for(int i=0; i<len && bSucceed; i++)
		bSucceed = SendByteAndCheckACK(pv[i]);
	SetI2CStop();
	return bSucceed;
}


BOOL CFtDevice::I2C_Read(BYTE chipAddr, BYTE regAddr, BYTE* pval, const int len)
{
	DWORD dwRead = len;
	memset(pval, 0, dwRead);
	//read back
	SetI2CLinesIdle();								// Set idle line condition
	SetI2CStart();
	BOOL bSucceed = SendAddrAndCheckACK(chipAddr, FALSE);
	if (bSucceed == 0) {
		return FALSE;
	}
	bSucceed = SendByteAndCheckACK(regAddr);		
	SetI2CLinesIdle();
	SetI2CStart();    // Set idle line condition as part of repeated start

	bSucceed = SendAddrAndCheckACK(chipAddr, TRUE);  	

	BOOL ret = ReadAndSendNAK(pval, dwRead);		// Send the device address 0x22 rd (I2C = 0x45)
	SetI2CStop();
	return ret;
}




LRESULT CFtDevice::SyncIO(int mode, int con, IO_OP* op, BOOL bhwSync )
{
	if (m_ftHandle == INVALID_HANDLE_VALUE) Open();
	if (m_ftHandle == INVALID_HANDLE_VALUE) return ERROR_DEVICE_NOT_AVAILABLE;
	int gpiobase = con * 3+4;
	FT_STATUS ftStatus = FT_OK;
	
	if (op->ret != S_OK) return op->ret;
	IO_VAL* rawval = (IO_VAL*)&op->val;

	if (mode == CFtdiDriver::IO_WRITE) {
		if (rawval->fmt == CON_RAW ) {
			DWORD rawv = *(DWORD*)(&rawval->v.pin);
			rawv <<= 4;;
			m_lowByte &= ~0xF0;
			m_lowByte |= rawv & 0xF0;
			m_highByte = (rawv >> 8) & 0xFF;
			if( bhwSync) ftStatus = Write(3);
		}
		else {
			char* pin = op->val.v.pin;
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
			if (bhwSync) ftStatus = Write(3);
		}
	}
	else {
		op->ret = S_OK;

		if (bhwSync) op->ret = UpdateRaw();

		if (op->ext_val == NULL) op->ext_val = op->val.v.pin;
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
	return ftStatus== FT_OK ? S_OK : ftStatus;
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
	if (m_ftHandle != INVALID_HANDLE_VALUE) return S_OK;
	FT_STATUS ftStatus;
	// Open the UM232H module by it's description in the EEPROM
	// Note: See FT_OpenEX in the D2xx Programmers Guide for other options available
	//ftStatus = FT_OpenEx("Dual RS232-HS B", FT_OPEN_BY_DESCRIPTION, &ftHandle);
	ftStatus = FT_Open(m_OpenIndex, &m_ftHandle);

	// Check if Open was successful
	if (ftStatus != FT_OK)
	{
		logError(L"Can't open FT232H device! \n");
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

		if (m_ComPort == -1) {
			ftStatus = FT_GetComPortNumber(m_ftHandle, &m_ComPort);
			logInfo(L"Device COM port is: %d", m_ComPort);
		}
		logTrace(L"Set FDTI to serial mode....");
		UCHAR bInMpsseMode = isMpsseMode();
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
	logInfo(L"Device_%d opened", m_OpenIndex);

	return S_OK;
}

LRESULT CFtDevice::Close(void)
{
	if (m_ftHandle != INVALID_HANDLE_VALUE) {
		FT_Close(m_ftHandle);
		m_ftHandle = INVALID_HANDLE_VALUE;
		logInfo(L"Device %d closed", m_OpenIndex);
	}
	return LRESULT(0);
}

void CFtDevice::SetAttribute(char* attr, int id) {

}

void CFtDevice::RunScript(char* script)
{
}

LRESULT CFtDevice::Run(void* chip_op)
{
	return LRESULT(0);
}
