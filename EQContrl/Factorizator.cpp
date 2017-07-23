#include <iostream>
#include <cmath>
#include "Factorizator.h"

#define DIVISOR_LIMIT 0x10000

Factorizator::Factorizator() {
    for (uint16_t i = 2; i != 0; i++) {
        if (IsPrime(i))
            m_PrimeNumbers.push_back(i);
    }
    //std::cout << "\nPrime numbers size: " << m_PrimeNumbers.size;
}

bool Factorizator::IsPrime(uint16_t n) const {
    for (auto i : m_PrimeNumbers) {
        if (n % i == 0)
            return false;
    }
    return true;
}

bool Factorizator::Factorize(uint32_t nComposite, T_Divisors &Divisors) const {
    while (nComposite > 1) {
        bool lDivisorFound = false;
        for (auto i : m_PrimeNumbers) {
            if (nComposite % i == 0) {
                Divisors.push_back(i);
                nComposite /= i;
                lDivisorFound = true;
                break;
            }
        }
        if (!lDivisorFound)
            return false;
    }
    return true;
}

bool Factorizator::FactorizeFloatInTwoDivisors(double fComposite, uint16_t &nFirst, uint16_t &nSecond) const {
    uint32_t nComposite = (uint32_t)round(fComposite);
    if (FactorizeInTwoDivisors(nComposite, nFirst, nSecond))
        return true;
//    uint32_t nFloor = (uint32_t)floor(fComposite);
//    if (FactorizeInTwoDivisors(nFloor == nComposite ? (uint32_t)ceil(fComposite) : nFloor, nFirst, nSecond))
//        return true;
    int nSign = nComposite == (uint32_t)floor(fComposite) ? 1 : -1;
    for (uint32_t i = 1; i < 20; i++) {
        if (FactorizeInTwoDivisors(nComposite + i * nSign, nFirst, nSecond) ||
                FactorizeInTwoDivisors(nComposite - i * nSign, nFirst, nSecond))
            return true;
    }
    return false;
}

bool Factorizator::FactorizeInTwoDivisors(uint32_t nComposite, uint16_t &nFirstOut, uint16_t &nSecondOut) const {
    //std::cout << "Try to factorize: " << nComposite << std::endl;
    T_Divisors Divisors;
    if (!Factorize(nComposite, Divisors))
        return false;

    uint32_t nFirst = 1;
    uint32_t nSecond = 1;
    for (auto it = Divisors.rbegin(); it != Divisors.rend(); it++) { //divisors are sorted from small to large
        if (nFirst * *it <= DIVISOR_LIMIT)
            nFirst *= *it;
        else
            nSecond *= *it;
    }
	if (nFirst <= DIVISOR_LIMIT && nSecond <= DIVISOR_LIMIT) {
		nFirstOut = nFirst;
		nSecondOut = nSecond;
		return true;
	}
	return false;
}