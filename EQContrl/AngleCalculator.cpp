#include "AngleCalculator.h"

void AngleCalculator::CalculateAngle(int16_t nValueX, int16_t nValueY) {
	//double a = std::atan2(y, x);
	/*if (nValueX < m_nMinValueX)
		m_nMinValueX = nValueX;
	if (nValueX > m_nMaxValueX)
		m_nMaxValueX = nValueX;
	if (nValueY < m_nMinValueY)
		m_nMinValueY = nValueY;
	if (nValueY > m_nMaxValueY)
		m_nMaxValueY = nValueY;*/

	/*ENC("X: " << nValueX << "\t(" << m_nMinValueX << "-" << m_nMaxValueX << 
		")\nY: " << nValueY << "\t(" << m_nMinValueY << "-" << m_nMaxValueY << ")");
	if (nValueX < 10000 && nValueY < 10000)*/
	CDR(nValueX << ";" << nValueY << ";");
}