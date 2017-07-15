#include "RateCalculator.h"

#define ARCSEC_TOTAL_COUNT 60 * 60 * 360

const double nSiderealDayLength = 86164.090530833; //seconds
const double nSiderealRate = ARCSEC_TOTAL_COUNT / nSiderealDayLength; //arcsec/sec
//const double nRA_SiderealFreqPrescaler = MCU_TIMER_FREQ * nSiderealDayLength / TOTAL_MICROSTEP_COUNT_RA;

const double nSolarDayLength = 86400; //seconds
const double nSolarRate = ARCSEC_TOTAL_COUNT / nSolarDayLength; //arcsec/sec
//const double nRA_SolarFreqPrescaler = MCU_TIMER_FREQ * nSolarDayLength / TOTAL_MICROSTEP_COUNT_RA;

const double nLunarMonth = 27.321661; //days
const double nLunarFreq = (1 / nSiderealDayLength) - (1 / (24 * 3600 * nLunarMonth));
const double nLunarDayLength = 1 / nLunarFreq; //seconds
const double nLunarRate = ARCSEC_TOTAL_COUNT / nLunarDayLength; //arcsec/sec
//const double nRA_LunarFreqPrescaler = MCU_TIMER_FREQ * nLunarDayLength / TOTAL_MICROSTEP_COUNT_RA;

double RateCalculator::GetRate(EQ::En_TrackRate nTrackRate) const {
	switch (nTrackRate) {
	case EQ::TR_SIDEREAL:
		return nSiderealRate;
	case EQ::TR_SOLAR:
		return nSolarRate;
	case EQ::TR_LUNAR:
		return nLunarRate;
	}
	return 0;
}

bool RateCalculator::CalculatePrescalersFromRate(uint32_t nTotalMicrostepCount, double fRate, uint16_t &nFirst, uint16_t &nSecond) const {
	if (!fRate)
		return false;
	double fPrescaler = ((double)MCU_TIMER_FREQ * (double)ARCSEC_TOTAL_COUNT) /
		(fRate * nTotalMicrostepCount);
	return m_Factorizator.FactorizeFloatInTwoDivisors(fPrescaler, nFirst, nSecond);
}
