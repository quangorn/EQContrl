#include "AngleCalculator.h"
#include <math.h>

void AngleCalculator::Init(int16_t nMinX, int16_t nMaxX, int16_t nMinY, int16_t nMaxY, uint16_t(&CorrectionData)[ENCODER_CORRECTION_DATA_SIZE]) {
	if (nMinX == -1 && nMaxX == -1 && nMinY == -1 && nMaxY == -1) {
		return;
	}
	CreateDirectoryA(CDR_DIR, NULL);
	m_lIsInited = true;
	m_nMinX = nMinX;
	m_nMaxX = nMaxX;
	m_nMinY = nMinY;
	m_nMaxY = nMaxY;
	InitRangeAndOffset();
	for (int i = 0; i < ENCODER_CORRECTION_DATA_SIZE; i++) {
		m_CorrectionData[i + 1] = CorrectionData[i] * 2 * M_PI / 0x10000 - M_PI;
	}
	m_CorrectionData[0] = -m_CorrectionData[1] - 2 * M_PI;
	m_CorrectionData[m_nSize - 1] = -m_CorrectionData[m_nSize - 2] + 2 * M_PI;
}

void AngleCalculator::InitRangeAndOffset() {
	m_nRangeX = (double)(m_nMaxX - m_nMinX) / 2;
	m_nOffsetX = (double)(m_nMaxX + m_nMinX) / 2;
	m_nRangeY = (double)(m_nMaxY - m_nMinY) / 2;
	m_nOffsetY = (double)(m_nMaxY + m_nMinY) / 2;
}

double AngleCalculator::CalculateAngle(uint32_t nMicrostepCount, uint32_t nWormCount, int16_t nValueX, int16_t nValueY) {
	if (!m_lIsInited) {
		return 0;
	}

	bool lConnected = false;
	//Ensure sensor not disconnected
	if ((nValueX != 0 || nValueY != 0) && (nValueX != -1 || nValueY != -1)) {
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

		lConnected = true;
		double x = (nValueX - m_nOffsetX) / m_nRangeX;
		double y = (nValueY - m_nOffsetY) / m_nRangeY;
		double radAngle = std::atan2(y > -1 ? (y < 1 ? y : 1) : -1, x > -1 ? (x < 1 ? x : 1) : -1);
		m_nLastAngle = CorrectAngle(radAngle);
	}
	
	SYSTEMTIME t;
	GetLocalTime(&t);
	if (t.wSecond != m_nLastCdrWriteTime.wSecond) {
		m_nLastCdrWriteTime = t;
		CDR(nMicrostepCount << ";" << nWormCount << ";" << nValueX << ";" << nValueY << ";" << m_nLastAngle
			<< (lConnected ? ";" : ";Error;"));
	}
	return m_nLastAngle;
}

double AngleCalculator::CorrectAngle(double radAngle) {
	ChangeCorrectionPosition(radAngle);
	double underPoint = m_CorrectionData[m_nLastCorrectionPos - 1];
	double overPoint = m_CorrectionData[m_nLastCorrectionPos];
	double betweenPointsDelta = (radAngle - underPoint) / (overPoint - underPoint);
	return (betweenPointsDelta + m_nLastCorrectionPos - 1.5) * 360 / ENCODER_CORRECTION_DATA_SIZE;
}

void AngleCalculator::ChangeCorrectionPosition(double radAngle) {
	if ((radAngle - m_CorrectionData[m_nLastCorrectionPos]) > M_PI) {
		m_nLastCorrectionPos = m_nSize - 1;
	} else if ((radAngle - m_CorrectionData[m_nLastCorrectionPos]) < -M_PI) {
		m_nLastCorrectionPos = 1;
	}
		
	while (radAngle > m_CorrectionData[m_nLastCorrectionPos]) {
		m_nLastCorrectionPos++;
	}
	while (radAngle < m_CorrectionData[m_nLastCorrectionPos - 1]) {
		m_nLastCorrectionPos--;
	}
}
