#pragma once

#include "Definitions.h"
#include "Factorizator.h"

class RateCalculator {
public:
	typedef std::vector<uint16_t> T_Divisors;

	RateCalculator()
	{}
	virtual ~RateCalculator()
	{}

	double GetRate(EQ::En_TrackRate nTrackRate) const;
	bool CalculatePrescalersFromRate(uint32_t nTotalMicrostepCount, double fRate, uint16_t &nFirst, uint16_t &nSecond) const;

private:
	Factorizator m_Factorizator;
};