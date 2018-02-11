#pragma once

#include <stdint.h>
#include "Definitions.h"
#include "Utils.h"

class AngleCalculator {
public:

	AngleCalculator()
	{
	}

	void CalculateAngle(int16_t nValueX, int16_t nValueY);

public:
	int16_t m_nMinX;
	int16_t m_nMaxX;
	int16_t m_nMinY;
	int16_t m_nMaxY;
	uint16_t m_CorrectionData[ENCODER_CORRECTION_DATA_SIZE];
};