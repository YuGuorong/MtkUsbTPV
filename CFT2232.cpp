#include "pch.h"
#include "CFT2232.h"
#include "ftd2xx.h"



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


DWORD CFT2232::m_DevNum = 0;
DWORD CFT2232::m_LgoicDev = 0;
void* CFT2232::m_pDevList = NULL;
DWORD dwClockDivisor = 0x00C8;		// 100khz

CFT2232::CFT2232() 
{
}
CFT2232::~CFT2232()
{
}

 FT_STATUS FT_ReadGPIO(FT_HANDLE handle, BYTE * value)
{
	FT_STATUS status;
	BYTE buffer[2];
	DWORD bytesTransfered = 0;
	DWORD bytesToTransfer = 0;
	UCHAR readBuffer[10];

#if 1 //def FT800_232HM
	buffer[bytesToTransfer++] = MPSSE_CMD_GET_DATA_BITS_HIGHBYTE;
	buffer[bytesToTransfer++] = MPSSE_CMD_SEND_IMMEDIATE;
#else
	buffer[bytesToTransfer++] = MPSSE_CMD_GET_DATA_BITS_LOWBYTE;
	buffer[bytesToTransfer++] = MPSSE_CMD_SEND_IMMEDIATE;
#endif
	status = FT_Write(handle, buffer, bytesToTransfer, &bytesTransfered);
	CHECK_STATUS(status);
	DBG( "bytesToTransfer=0x%x bytesTransfered=0x%x\n", (unsigned)bytesToTransfer, (unsigned)bytesTransfered);
	bytesToTransfer = 1;
	bytesTransfered = 0;
	status = FT_Read(handle, readBuffer, bytesToTransfer, & bytesTransfered);
	CHECK_STATUS(status);
	DBG("bytesToTransfer=0x%x bytesTransfered=0x%x\n", \
		(unsigned)bytesToTransfer, (unsigned)bytesTransfered);
	if (bytesToTransfer != bytesTransfered)
		status = FT_IO_ERROR;
	*value = readBuffer[0];

	return status;
}
void CFT2232::GetGpio()
{

}


void CFT2232::SetGpio(BYTE lb, BYTE hb) {
	int dwNumBytesToSend = 0;			//Clear output buffer

	// Set the idle states for the AD lines
	OutputBuffer[dwNumBytesToSend++] = MID_SET_LOW_BYTE_DATA_BITS_CMD;	// Command to set directions of ADbus and data values for pins set as o/p
	OutputBuffer[dwNumBytesToSend++] = lb;    // Set all 8 lines to high level (only affects pins which are output)
	OutputBuffer[dwNumBytesToSend++] = 0xFF;	// Set all pins as output except bit 2 which is the data_in


	// Set the idle states for the AC lines
	OutputBuffer[dwNumBytesToSend++] = MID_SET_HIGH_BYTE_DATA_BITS_CMD;	// Command to set directions of ACbus and data values for pins set as o/p
	OutputBuffer[dwNumBytesToSend++] = hb;	// Set all 8 lines to high level (only affects pins which are output)
	OutputBuffer[dwNumBytesToSend++] = 0xff;	// Only bit 6 is output


	FT_STATUS ftStatus = FT_Write(ftHandle, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent);		//Send off the commands
	if (ftStatus == FT_OK) {
		m_lowByte = lb;
		m_highByte = hb;
	}
}


LRESULT CFT2232::Read(void)
{
	return S_OK;
}





LRESULT CFT2232::Set(int con, int home, int kcol, int pwr)
{
	return LRESULT(0);
}

LRESULT CFT2232::Set(int gpio)
{
	return LRESULT(0);
}

LRESULT CFT2232::SetAll(int raw)
{
	return LRESULT(0);
}

LRESULT CFT2232::Open()
{
	DWORD dwCount;
	FT_STATUS ftStatus;
	// Open the UM232H module by it's description in the EEPROM
	// Note: See FT_OpenEX in the D2xx Programmers Guide for other options available
	//ftStatus = FT_OpenEx("Dual RS232-HS B", FT_OPEN_BY_DESCRIPTION, &ftHandle);
	ftStatus = FT_Open(m_chnId, &ftHandle);

	// Check if Open was successful
	if (ftStatus != FT_OK)
	{
		printf("Can't open FT232H device! \n");
		return 1;
	}
	else
	{
		// #########################################################################################
		// After opening the device, Put it into MPSSE mode
		// #########################################################################################
		// Print message to show port opened successfully
		printf("Successfully opened FT232H device! \n");

		// Reset the FT232H
		ftStatus |= FT_ResetDevice(ftHandle);

		// Purge USB receive buffer ... Get the number of bytes in the FT232H receive buffer and then read them
		ftStatus |= FT_GetQueueStatus(ftHandle, &dwNumInputBuffer);
		if ((ftStatus == FT_OK) && (dwNumInputBuffer > 0))
		{
			FT_Read(ftHandle, &InputBuffer, dwNumInputBuffer, &dwNumBytesRead);
		}

		ftStatus |= FT_SetUSBParameters(ftHandle, 65536, 65535);			// Set USB request transfer sizes
		ftStatus |= FT_SetChars(ftHandle, false, 0, false, 0);				// Disable event and error characters
		ftStatus |= FT_SetTimeouts(ftHandle, 5000, 5000);					// Set the read and write timeouts to 5 seconds
		ftStatus |= FT_SetLatencyTimer(ftHandle, 16);						// Keep the latency timer at default of 16ms
		ftStatus |= FT_SetBitMode(ftHandle, 0x0, 0x00); 					// Reset the mode to whatever is set in EEPROM
		ftStatus |= FT_SetBitMode(ftHandle, 0x0, 0x02);	 					// Enable MPSSE mode

		// Inform the user if any errors were encountered
		if (ftStatus != FT_OK)
		{
			printf("failure to initialize FT232H device! \n");
			return 1;
		}

		Sleep(50);

		// #########################################################################################
		// Synchronise the MPSSE by sending bad command AB to it
		// #########################################################################################

		int dwNumBytesToSend = 0;																// Used as an index to the buffer
		OutputBuffer[dwNumBytesToSend++] = 0xAB;											// Add an invalid command 0xAB
		ftStatus = FT_Write(ftHandle, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent);		// Send off the invalid command

		// Check if the bytes were sent off OK
		if (dwNumBytesToSend != dwNumBytesSent)
		{
			printf("Write timed out! \n");
			return 1;
		}


		// Now read the response from the FT232H. It should return error code 0xFA followed by the actual bad command 0xAA
		// Wait for the two bytes to come back 

		dwNumInputBuffer = 0;
		ReadTimeoutCounter = 0;

		ftStatus = FT_GetQueueStatus(ftHandle, &dwNumInputBuffer);						// Get number of bytes in the input buffer
		while ((dwNumInputBuffer < 2) && (ftStatus == FT_OK) && (ReadTimeoutCounter < 500))
		{
			ftStatus = FT_GetQueueStatus(ftHandle, &dwNumInputBuffer);	// Get number of bytes in the input buffer
			ReadTimeoutCounter++;
			Sleep(1);													// short delay
		}

		// If the loop above exited due to the byte coming back (not an error code and not a timeout)
		// then read the bytes available and check for the error code followed by the invalid character
		if ((ftStatus == FT_OK) && (ReadTimeoutCounter < 500))
		{
			ftStatus = FT_Read(ftHandle, &InputBuffer, dwNumInputBuffer, &dwNumBytesRead); // Now read the data

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
			printf("fail to synchronize MPSSE with command 0xAB \n");
			return 1;
		}


		printf("MPSSE synchronized with BAD command \n");


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

		ftStatus = FT_Write(ftHandle, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent);	// Send off the commands

		// Now configure the dividers to set the SCLK frequency which we will use
		// The SCLK clock frequency can be worked out by the algorithm (when divide-by-5 is off)
		// SCLK frequency  = 60MHz /((1 +  [(1 +0xValueH*256) OR 0xValueL])*2)
		dwNumBytesToSend = 0;													// Clear index to zero
		OutputBuffer[dwNumBytesToSend++] = 0x86; 								// Command to set clock divisor
		OutputBuffer[dwNumBytesToSend++] = dwClockDivisor & 0xFF;				// Set 0xValueL of clock divisor
		OutputBuffer[dwNumBytesToSend++] = (dwClockDivisor >> 8) & 0xFF;		// Set 0xValueH of clock divisor
		ftStatus = FT_Write(ftHandle, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent);	// Send off the commands

		Sleep(20);																// Short delay 	


		Sleep(30);		//Delay for a while
	}

	BOOL bSucceed = TRUE;




	SetGpio(0x00, 0x00);
	SetGpio(0xFF, 0xFF);

	FT_Close(ftHandle);

	return S_OK;
}

LRESULT  CFT2232::Scan(void)
{
	DWORD devIndex = 0;
	FT_STATUS ftStatus;

	// ---------------------------------------------------------- -
		// Does an FTDI device exist?
		// -----------------------------------------------------------
	printf("Checking for FTDI devices...\n");
	ftStatus = FT_CreateDeviceInfoList(&m_LgoicDev);// Get the number of FTDI devices
	if (ftStatus != FT_OK)// Did the command execute OK?
	{
		printf("Error in getting the number of devices\n");
		return FT_INSUFFICIENT_RESOURCES;	 
	}
	if (m_LgoicDev < 1)// Exit if we don't see any
	{
		printf("There are no FTDI devices installed\n");
		return FT_DEVICE_NOT_FOUND;		 
	}
	printf("%d FTDI devices channel found\n", m_LgoicDev);
	printf("=========================================\n");

	if (ftStatus == FT_OK) {
		DWORD tempNumChannels = m_LgoicDev + 1;
		FT_DEVICE_LIST_INFO_NODE* pDeviceList ;

		pDeviceList = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE) * (tempNumChannels + 1));
		if (NULL == pDeviceList)
		{
			return FT_INSUFFICIENT_RESOURCES;
		}
		/*get the devices information(FT_GetDeviceInfoList)*/
		int combineDev = -1;
		m_DevNum = 0;
		ftStatus = FT_GetDeviceInfoList(pDeviceList, &tempNumChannels);
		for (int i = 0; i < (int)tempNumChannels; i++) {
			switch (pDeviceList[i].Type) {
			case FT_DEVICE_2232H:
			case FT_DEVICE_2232C:
				if (combineDev < 0) {
					printf("Device %d\n", m_DevNum);/* 0 if not open*/
					combineDev = 2;
					m_DevNum++;
				}
			case FT_DEVICE_4232H:
				if (combineDev < 0) {
					printf("Device %d\n", m_DevNum);/* 0 if not open*/
					combineDev = 4;
					m_DevNum++;
				}
			}
			printf("Information on channel number %d:\n", (unsigned int)i);
			const char* stype[] = {
					"FT_DEVICE_232BM",
					"FT_DEVICE_232AM",
					"FT_DEVICE_100AX",
					"FT_DEVICE_UNKNOWN",
					"FT_DEVICE_2232C",
					"FT_DEVICE_232R",
					"FT_DEVICE_2232H",
					"FT_DEVICE_4232H",
					"FT_DEVICE_232H",
					"FT_DEVICE_X_SERIES",
			};

			/*print the dev info*/
			printf("		Flags=0x%x(%s)\n", pDeviceList[i].Flags, pDeviceList[i].Flags == FT_FLAGS_OPENED ? "FT_FLAGS_OPENED" : "FT_FLAGS_HISPEED");
			printf("		Type=0x%x(%s)\n", pDeviceList[i].Type, stype[pDeviceList[i].Type]);
			printf("		ID=0x%x\n", pDeviceList[i].ID);
			printf("		LocId=0x%x\n", pDeviceList[i].LocId);
			printf("		SerialNumber=%s\n", pDeviceList[i].SerialNumber);
			printf("		Description=%s\n", pDeviceList[i].Description);
			printf("		ftHandle=0x%x\n", (unsigned int)pDeviceList[i].ftHandle);/* 0 if not open*/
			if (--combineDev <= 0) {
				combineDev = -1;
				printf("=========================================\n");
			}

		}
		free(pDeviceList);

		// FT_ListDevices OK, product descriptions are in Buffer1 and Buffer2, and
		// m_DevNum contains the number of devices connected
		printf("Number of devices connected is: %d\n", m_DevNum);
	}
	else {
		// FT_ListDevices failed
	}
	return S_OK;
}

