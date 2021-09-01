#pragma once
class CPca9505
{
	static const int chipaddr = 0x20;
	static const int IOGroup = 5;
	BYTE m_RegOP[IOGroup];  //Output REG cache
	BYTE m_RegICO[IOGroup]; //Config Reg cache, 0:Output ,1:Input

public:
	CPca9505();
	~CPca9505();
	BOOL CheckConfig();

};

