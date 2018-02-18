#pragma once

#include <stdint.h>
#include "Definitions.h"
#include "Utils.h"

class AngleCalculator {
public:

	AngleCalculator() :
		m_lIsInited(false),
		m_nLastAngle(0)
	{}

	void Init(int16_t nMinX, int16_t nMaxX,	int16_t nMinY, int16_t nMaxY, uint16_t(&CorrectionData)[ENCODER_CORRECTION_DATA_SIZE]);

	bool IsInited() {
		return m_lIsInited;
	}

	/*
	Returns angle 0 - 360 
	*/
	double CalculateAngle(int16_t nValueX, int16_t nValueY);

private:
	void InitRangeAndOffset();

public:
	bool m_lIsInited;
	int16_t m_nMinX;
	int16_t m_nMaxX;
	int16_t m_nMinY;
	int16_t m_nMaxY;
	double m_nRangeX;
	double m_nRangeY;
	double m_nOffsetX;
	double m_nOffsetY;
	double m_nLastAngle;
	double m_CorrectionData[ENCODER_CORRECTION_DATA_SIZE];
};