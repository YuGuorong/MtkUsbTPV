#include "pch.h"
#include "CFdtiDriver.h"
#include "CFtDevice.h"

#define UNINIT_LCLID   (-597)


typedef int (*fLog)(const char* fmt, ...);
CFtdiDriver* CFtdiDriver::m_InstFtdi = NULL;

int dumylog(const char* fmt, ...) {
	return 0;
}


CFtdiDriver::CFtdiDriver()
{
	m_DevChannels = 0;
	m_pDevInfoList = NULL;

	m_InstFtdi = this;
}

CFtdiDriver::~CFtdiDriver()
{
	m_BoardList.clear();
	if (m_pDevInfoList) {
		free(m_pDevInfoList);
	}
	//if (m_InstFtdi != NULL) delete m_InstFtdi;

}

CFtdiDriver* CFtdiDriver::CetDriver() {
	return m_InstFtdi;
}

LRESULT CFtdiDriver::Read(IO_OP* op)
{
	return LRESULT(0);
}

LRESULT CFtdiDriver::Write(IO_OP* op)
{
	return LRESULT();
}

LRESULT CFtdiDriver::SetAll(BYTE val)
{
	return LRESULT(0);
}

void CFtdiDriver::ShowDevices(void)
{
	FT_STATUS ftStatus;
	fLog  plog =  printf;
	
	for (int i = 0; i < (int)m_DevChannels; i++) 
	{
		plog("Information on channel number %d:\n", (unsigned int)i);
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
		plog("		Flags=0x%x(%s)\n", m_pDevInfoList[i].Flags, m_pDevInfoList[i].Flags == FT_FLAGS_OPENED ? "FT_FLAGS_OPENED" : "FT_FLAGS_HISPEED");
		plog("		Type=0x%x(%s)\n", m_pDevInfoList[i].Type, stype[m_pDevInfoList[i].Type]);
		plog("		ID=0x%x\n", m_pDevInfoList[i].ID);
		plog("		LocId=0x%x\n", m_pDevInfoList[i].LocId);
		plog("		SerialNumber=%s\n", m_pDevInfoList[i].SerialNumber);
		plog("		Description=%s\n", m_pDevInfoList[i].Description);
		plog("		ftHandle=0x%x\n", (unsigned int)m_pDevInfoList[i].ftHandle);/* 0 if not open*/
	}
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

	// TODO: 在此处添加实现代码.
	map<int, CFtDevice*>::iterator iter;
	for (iter = m_Devices.begin(); iter != m_Devices.end(); iter++) {
		iter->second->bcheckMount = FALSE;
	}

	INT lastLocId = UNINIT_LCLID;
	int subChannels = 0;
	qsort(m_pDevInfoList, m_DevChannels, sizeof(FT_DEVICE_LIST_INFO_NODE), comp_dev);
	vector<int>* board = NULL;
	for (int i = 0; i < (int)m_DevChannels; i++) {

		fdtiid devID = (fdtiid)m_pDevInfoList[i].LocId;
		if (m_pDevInfoList[i].LocId - lastLocId == 0) {
			CString sinfo;
			int n = i == 0 ? 0 : i - 1;
			sinfo.Format(L"设备ID重名！\r\n设备序号：%d, 设备ID:%d \r\n设备序号：%d, 设备ID:%d ",
				i, devID, n, m_pDevInfoList[n].LocId);
			::MessageBox(NULL, sinfo, L"系统错误", MB_OK);
			return ERROR_INVALID_ADDRESS;
		}

		if (devID - lastLocId == 1) {
			if (subChannels) {
				if (board) board->push_back(devID);
				--subChannels;
			}
			lastLocId = (subChannels == 0) ? UNINIT_LCLID : devID;
		}
		else { //new board
			switch (m_pDevInfoList[i].Type) {
			case FT_DEVICE_2232H:
			case FT_DEVICE_2232C:
				subChannels = 1;
				break;

			case FT_DEVICE_4232H:
				subChannels = 3;
				break;
			default:
				subChannels = 0;
				break;
			}
			if (subChannels) {
				if (board) delete board;
				vector<fdtiid>* board = new vector<fdtiid>;
				board->push_back(devID);
				m_BoardList.push_back(board);
			}
			lastLocId = devID;
		}
		map<int, CFtDevice*>::iterator iter = m_Devices.find(devID);
		if (iter == m_Devices.end()) {
			CFtDevice * odev  = new CFtDevice(devID);
			m_Devices[devID] = odev;
			odev->bcheckMount = TRUE;
		}
		else {
			iter->second->bcheckMount = TRUE;
		}
	}

	iter = m_Devices.begin();
	while ( iter != m_Devices.end() ) {
		if (iter->second->bcheckMount == FALSE)
		{
			delete iter->second;
			m_Devices.erase(iter);
		}
		else {
			iter++;
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
		plog("Error in getting the number of devices\n");
		return FT_INSUFFICIENT_RESOURCES;
	}
	if (m_DevChannels < 1)// Exit if we don't see any
	{
		plog("There are no FTDI devices installed\n");
		return FT_DEVICE_NOT_FOUND;
	}
	plog("%d FTDI devices channel found\n", m_DevChannels);

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
		plog("Number of devices connected is: %d\n", m_DevChannels);
	}
	else {
		plog("FTDI SDK load fail\n");
	}
	return S_OK;
}
