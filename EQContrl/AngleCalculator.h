#pragma once

#include <stdint.h>
#include "Definitions.h"
#include "Utils.h"

class AngleCalculator {
public:

	AngleCalculator() :
		m_nMinValueX(5000),
		m_nMaxValueX(8000),
		m_nMinValueY(5000),
		m_nMaxValueY(8000)
	{
		//m_fout.open(CDR_FILE, std::ios_base::app);
	}

	virtual ~AngleCalculator()
	{
		/*std::ofstream m_fout;
		m_fout.close();*/
	}

	void CalculateAngle(uint32_t nValueX, uint32_t nValueY);

private:
	uint32_t m_nMinValueX;
	uint32_t m_nMaxValueX;
	uint32_t m_nMinValueY;
	uint32_t m_nMaxValueY;

	//std::stringstream m_fout;
};