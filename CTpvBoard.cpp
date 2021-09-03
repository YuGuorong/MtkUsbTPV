#include "pch.h"
#include "CTpvBoard.h"
#include "CFtDevice.h"
#include "ftd2xx.h"
#include "util.h"


CTpvBoard::CTpvBoard(int idx) {
	m_index = idx;
	m_deviceNum = 0;
	m_strPortInfo = L"";
	m_BoardID = (int)(((DWORD)-1) >> 1);
}

CTpvBoard::~CTpvBoard()
{
	for (int i = 0; i < m_deviceNum; i++) {
		delete m_Devices[i];
	}
	m_deviceNum = 0;
}

void CTpvBoard::AddDevice(FT_DEVICE_LIST_INFO_NODE* pdevInfo, int open_idx)
{
	m_Devices[m_deviceNum] = new CFtDevice(pdevInfo, open_idx);
	if (m_deviceNum == 0) {
		m_BoardID = (DWORD)m_Devices[m_deviceNum];
	}
	m_deviceNum++;
}

void CTpvBoard::AddDevice(CFtDevice* pdev)
{
	m_Devices[m_deviceNum] = pdev;
	m_deviceNum++;
}

void CTpvBoard::DoMount(BOOL bMount)
{
	if (bMount != m_bMounted)
	{
		//m_strPortInfo.Empty();
		TCHAR szInfo[32];
		for (int i = 0; i < m_deviceNum; i++) {
			if (bMount) {
				m_Devices[i]->UpdateRaw();
				_stprintf_s(szInfo, 32, L"%s/%d", m_strPortInfo, m_Devices[i]->m_ComPort);
				m_strPortInfo = szInfo;
			}
			else
			{
				m_Devices[i]->Close();
			}
		}
		m_bMounted = bMount;
	}
}

void CTpvBoard::Close()
{
	for (int i = 0; i < m_deviceNum; i++) {
		m_Devices[i]->Close();
	}
}

LRESULT CTpvBoard::SyncIO(vector<IO_OP>& io_reqQue)
{
	LRESULT ret = S_OK;
	for (int i = 0; i < (int)io_reqQue.size(); i++) {
		LRESULT err = S_OK;
		int con = io_reqQue[i].val.con;

		//Lowbyte : con0,con1
		err = SyncIO(con, &io_reqQue[i]);		
		if (err != FT_OK) {
			ret = err;
			break;
		}
	}
	if (ret == FT_IO_ERROR) {
		this->DoMount(false);
	}
	return ret;
}

LRESULT CTpvBoard::SyncIO(int con, IO_OP* op, BOOL bSyncHW )
{
	LRESULT ret = S_OK;
	if (con == IO_ALL_CON) {
		for (int c = 0; c < IO_CON_MAX; c++) {
			if (SyncIO(c, op, c == (IO_CON_MAX - 1)) != S_OK) break;
		}
	}
	else {
		op->ret = S_OK;
		LRESULT err = m_Devices[0]->SyncIO(CFtdiDriver::IO_WRITE, con , op, bSyncHW);
		if (err != S_OK) {
			logError(L"Process  IO Req faile with:%d(%s)", err, (LPCTSTR)ErrorString(err));
			ret = err;
		}
		if (op->ret != S_OK) return op->ret;
		op->ret = err;
	}

	return ret;
}


LRESULT CTpvBoard::Display(int con, IO_VAL* io_val, int* items)
{
	if (con == IO_CON_GPIO) {
		*items = 1;
	}
	else  if (con == IO_ALL_CON) {
		*items = 1 + 8;
	}
	else {
		*items = 1;
	}
	if (io_val == NULL) return ERROR_INVALID_ADDRESS;

	if (con == IO_ALL_CON) {
		BYTE val[4];
		m_Devices[0]->UpdateRaw();
		for (int c = IO_CON0; c < IO_CON_MAX; c++) {
			io_val[c].con = c;
			char* pkey = (char*)io_val[c].v.pin;
			IO_OP op = { m_index, pkey, (char)c, {0} };
			m_Devices[0]->SyncIO(CFtdiDriver::IO_READ, c, &op, FALSE);
		}
		IO_OP op = { m_index, NULL, (char)IO_CON_GPIO, {0} };
		m_Devices[0]->SyncIO(CFtdiDriver::IO_READ, IO_CON_GPIO, &op, FALSE);
		IO_VAL* vgpio = (IO_VAL*)&io_val[IO_CON_MAX];
		*vgpio = op.val;
	}
	else {
		char* pkey = (char*)io_val[0].v.pin;
		IO_OP op = { m_index, pkey, con, {0} };
		m_Devices[0]->SyncIO(CFtdiDriver::IO_READ, con, &op);
	}
	return S_OK;
}


