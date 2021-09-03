#include "pch.h"
#include "CFdtiDriver.h"
#include "CFtDevice.h"
#include "CTpvBoard.h"
#include "CPca9505.h"
#include "util.h"

#define UNINIT_LCLID   (-597)


const IO_OP def_request = { -1, NULL, {(BYTE)IO_CON_UNKNOWN ,CON_CTL , {(DWORD)-1}}, S_OK };

CFtdiDriver* CFtdiDriver::m_InstFtdi = NULL;

int dumylog(const char* fmt, ...) {
	return 0;
}


CFtdiDriver::CFtdiDriver()
{
	m_DevChannels = 0;
	m_pDevInfoList = NULL;

	m_InstFtdi = this;
	m_BoardIdxNum = 0;

	Load();
}

CFtdiDriver::~CFtdiDriver()
{
	Save();
	if (m_pDevInfoList) {
		free(m_pDevInfoList);
	}
	map<int, CTpvBoard*>::iterator itor = m_BoardList.begin();
	while (itor != m_BoardList.end()) {
		delete itor->second;
		//m_BoardList.erase(itor);
		itor++;
	}
	m_BoardList.clear();
	//if (m_InstFtdi != NULL) delete m_InstFtdi;
}

CFtdiDriver* CFtdiDriver::GetDriver() {
	return m_InstFtdi;
}

LRESULT CFtdiDriver::Read(IO_OP* op)
{
	if (op == NULL) return ERROR_BAD_ARGUMENTS;
	this->SyncIO(CFtdiDriver::IO_READ, op);
	return LRESULT(0);
}

LRESULT CFtdiDriver::Write(IO_OP* op)
{
	if (op == NULL) return ERROR_BAD_ARGUMENTS;
	this->SyncIO(CFtdiDriver::IO_READ, op);
	return LRESULT();
}

LRESULT CFtdiDriver::SetAll(int index, BYTE val)
{
	
	return LRESULT(0);
}
//#include "libjson/json/json.h"
void CFtdiDriver::Load()
{
	//ifstream ifs("", ios::in);
}

void CFtdiDriver::Save()
{

}

CTpvBoard * CFtdiDriver::FindBoard(fdtiid id, int index, LRESULT * err)
{
	for (map<int, CTpvBoard*>::iterator it = m_BoardList.begin(); it != m_BoardList.end(); it++) {
		if ((index != -1 && it->second->m_index == index) || (id != 0 && it->second->m_BoardID == id)) {
			if (err) {
				*err = (it->second->m_bMounted == TRUE) ? S_OK : ERROR_MOUNT_POINT_NOT_RESOLVED;
			}
			return it->second;
		}
	}
	if( err ) *err = ERROR_NOT_FOUND;
	return NULL;
}

void CFtdiDriver::ShowDevices(void)
{
	for (map<int, CTpvBoard*>::iterator it = m_BoardList.begin(); it != m_BoardList.end(); it++) {

		printf("Board index: %d\r\n", it->second->m_index);
		printf("      %s\r\n", it->second->m_bMounted ? "Mounted" : "Unmount");
		printf("      ID: %d[%XH] \r\n", it->second->m_BoardID, it->second->m_BoardID);
		printf("      Port num: %d \r\n", it->second->m_deviceNum);
		printf("\r\n");
		
	}
	
}

LRESULT CFtdiDriver::SyncIO(int mode, IO_OP* op)
{

	return LRESULT();
}



int __cdecl comp_dev(void const* p1, void const* p2)
{
	FT_DEVICE_LIST_INFO_NODE* dev1 = (FT_DEVICE_LIST_INFO_NODE*)p1;
	FT_DEVICE_LIST_INFO_NODE* dev2 = (FT_DEVICE_LIST_INFO_NODE*)p2;
	return dev1->LocId > dev2->LocId;
}

LRESULT CFtdiDriver::MountDevices()
{
	m_BoardList.clear();

	INT lastLocId = UNINIT_LCLID;
	int subChannels = 0;

	CTpvBoard * board = NULL;

	for (int i = 0; i < (int)m_DevChannels; i++) {
		if ( strcmp( m_pDevInfoList[i].SerialNumber ,"A") == 0) {
			CPca9505* pdev = new CPca9505(&m_pDevInfoList[i], i);
			if (pdev->CheckConfig()) {
				board = new CTpvBoard(m_BoardIdxNum);
				m_BoardList[m_BoardIdxNum] = board;
				m_BoardIdxNum++;
				board->AddDevice(pdev);
				board->DoMount(TRUE);
			}
			else
				delete pdev;
		}

	}

	return LRESULT(0);
}

LRESULT  CFtdiDriver::Scan(BOOL blog)
{
	FT_STATUS ftStatus;
	fLog  plog = (blog) ? printf : dumylog;

	// ---------------------------------------------------------- -
		// Does an FTDI device exist?
		// -----------------------------------------------------------
	ftStatus = FT_CreateDeviceInfoList(&m_DevChannels);// Get the number of FTDI devices
	if (ftStatus != FT_OK)// Did the command execute OK?
	{
		logError(L"Error in getting the number of devices\n");
		return FT_INSUFFICIENT_RESOURCES;
	}
	if (m_DevChannels < 1)// Exit if we don't see any
	{
		logError(L"There are no FTDI devices installed\n");
		return FT_DEVICE_NOT_FOUND;
	}
	logInfo(L"%d FTDI devices channel found\n", m_DevChannels);

	if (ftStatus == FT_OK) {
		DWORD tempNumChannels = m_DevChannels + 1;

		if (m_pDevInfoList != NULL) free(m_pDevInfoList);
		m_pDevInfoList = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE) * (tempNumChannels + 1));
		if (NULL == m_pDevInfoList)
		{
			return FT_INSUFFICIENT_RESOURCES;
		}
		/*get the devices information(FT_GetDeviceInfoList)*/
		ftStatus = FT_GetDeviceInfoList(m_pDevInfoList, &m_DevChannels);

		// FT_ListDevices OK, product descriptions are in Buffer1 and Buffer2, and
		// m_DevNum contains the number of devices connected
		logInfo(L"Number of devices connected is: %d\n", m_DevChannels);
	}
	else {
		logError(L"FTDI SDK load fail\n");
	}


	if (g_dbgLogConsole) {
		for (int i = 0; i < (int)m_DevChannels; i++)
		{
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
			printf("		Flags=0x%x(%s)\n", m_pDevInfoList[i].Flags, m_pDevInfoList[i].Flags == FT_FLAGS_OPENED ? "FT_FLAGS_OPENED" : "FT_FLAGS_HISPEED");
			printf("		Type=0x%x(%s)\n", m_pDevInfoList[i].Type, stype[m_pDevInfoList[i].Type]);
			printf("		ID=0x%x\n", m_pDevInfoList[i].ID);
			printf("		LocId=0x%x\n", m_pDevInfoList[i].LocId);
			printf("		SerialNumber=%s\n", m_pDevInfoList[i].SerialNumber);
			printf("		Description=%s\n", m_pDevInfoList[i].Description);
			printf("		ftHandle=0x%x\n", (unsigned int)m_pDevInfoList[i].ftHandle);/* 0 if not open*/
		}
	}
	return S_OK;
}
