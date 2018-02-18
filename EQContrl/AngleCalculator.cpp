#include "AngleCalculator.h"
#include <math.h>

void AngleCalculator::Init(int16_t nMinX, int16_t nMaxX, int16_t nMinY, int16_t nMaxY, uint16_t(&CorrectionData)[ENCODER_CORRECTION_DATA_SIZE]) {
	m_nMinX = nMinX;
	m_nMaxX = nMaxX;
	m_nMinY = nMinY;
	m_nMaxY = nMaxY;
	InitRangeAndOffset();
	for (int i = 0; i < ENCODER_CORRECTION_DATA_SIZE; i++) {
		m_CorrectionData[i] = CorrectionData[i] * 2 * M_PI / 0x10000 - M_PI;
	}
}

void AngleCalculator::InitRangeAndOffset() {
	m_nRangeX = (double)(m_nMaxX - m_nMinX) / 2;
	m_nOffsetX = (double)(m_nMaxX + m_nMinX) / 2;
	m_nRangeY = (double)(m_nMaxY - m_nMinY) / 2;
	m_nOffsetY = (double)(m_nMaxY + m_nMinY) / 2;
}

double AngleCalculator::CalculateAngle(int16_t nValueX, int16_t nValueY) {
	//In case of sensor disconnect
	if ((nValueX == 0 && nValueY == 0) ||
		(nValueX == -1 && nValueY == -1)) {
		CDR(nValueX << ";" << nValueY << ";Error;");
		return m_nLastAngle;
	}
	
	/*if (nValueX < m_nMinX) {
		m_nMinX = nValueX;
		InitRangeAndOffset();
	} else if (nValueX > m_nMaxX) {
		m_nMaxX = nValueX;
		InitRangeAndOffset();
	}
	if (nValueY < m_nMinY) {
		m_nMinY = nValueY;
		InitRangeAndOffset();
	}
	else if (nValueY > m_nMaxY) {
		m_nMaxY = nValueY;
		InitRangeAndOffset();
	}*/

	double x = (nValueX - m_nOffsetX) / m_nRangeX;
	double y = (nValueY - m_nOffsetY) / m_nRangeY;
	double grad = std::atan2(y > -1 ? (y < 1 ? y : 1) : -1, x > -1 ? (x < 1 ? x : 1) : -1) * 180 / M_PI + 180;
	CDR(nValueX << ";" << nValueY << ";" << grad << ";");
	return grad;
}