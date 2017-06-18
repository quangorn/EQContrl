#pragma once

#include <cstdint>
#include <vector>

class Factorizator {
public:
    typedef std::vector<uint16_t> T_Divisors;

    Factorizator();
    virtual ~Factorizator()
    {}

    bool IsPrime(uint16_t n) const;
    bool Factorize(uint32_t nComposite, T_Divisors &Divisors) const;
    bool FactorizeInTwoDivisors(uint32_t nComposite, uint16_t &nFirst, uint16_t &nSecond) const;
    bool FactorizeFloatInTwoDivisors(double fComposite, uint16_t &nFirst, uint16_t &nSecond) const;

private:
    T_Divisors m_PrimeNumbers;
};
