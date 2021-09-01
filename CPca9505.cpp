#include "pch.h"
#include "CPca9505.h"

CPca9505::CPca9505()
{
	memset(m_RegICO, 0xFF, sizeof(m_RegOP));
	memset(m_RegICO, 0x88, sizeof(m_RegICO));
}

CPca9505::~CPca9505()
{
}

BOOL CPca9505::CheckConfig()
{
	return 0;
}
