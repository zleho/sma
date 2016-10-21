#pragma once

#include <cmath>

/*
   20*log10(RMS/(2^(-Q))
    = 20*Q*log10(2) + 20*log10(RMS)
    = C1 + 10*log10(SUM/N), where C1 = 20*16*log10(2)
    = C1 + -10*log10(N) + 10*log10(SUM)
    = C1 + C2 + (10/log2(10))*log2(SUM), where C2 = -10*log10(N)
    = C1 + C2 + C3 * log2(SUM), where C3 = 10/log2(10)

*/

template <class Fixed>
class RMSdB {
public:
    static double max() 
    {
        return max_;
    }

    using FixedType = Fixed;

    RMSdB() {}

    explicit RMSdB(size_t n) 
        : size_(n),
          c1(max()),
          c2(-10*std::log10(size_)),
          c3(10/std::log2(10))
    {
        init();
    }

    void init()
    {
        sum_ = Fixed();
        curr_ = 0;
    }

    bool step(Fixed x, double& ret)
    {
        x *= x;
        sum_ += x;

        if (++curr_ == size_) {
            ret = 0.0;
            if (sum_.repr) {
                sum_ = fixie::log2(sum_);
                sum_ *= c3;
                sum_ += c1 + c2;
                ret = static_cast<double>(sum_);
            }
            init();
            return true;
        }
        return false;
    }
private:
    static double max_;
    size_t curr_;
    size_t size_;
    Fixed sum_;
    Fixed c1, c2, c3;
};

template <class Fixed> double RMSdB<Fixed>::max_ = 20*Fixed::power()*std::log10(2);
