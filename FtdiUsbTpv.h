#pragma once

#include "resource.h"

enum {
	SERVER_NOT_READY = 0,
	SERVER_START,
	SERVER_INIT,
	SERVER_READY,
	SCAN_DEVICES,
	SCAN_DONE,
	BOARD_NOT_FOUND ,
	BOARD_READY = 0x100,
};

#define MSG_EVENT_NAME L"Meadiatek_test_suite_event"
#define NAME_WND_MSG_MAIN L"Meadiatek_test_suite"