#pragma once

#include <cmath>
#include "BiQuad.h"

template <class Fixed, std::size_t sampleRate = 48000>
class LowPass : public BiQuad<Fixed> {
public:
    LowPass()
    {
    }

    LowPass(double F, double Q)
    {
        double K = std::tan(M_PI * F / sampleRate);
        double N = 1 / (1 + K/Q + K*K);
        Fixed b0 = Fixed(K * K * N);
        Fixed b1 = b0 << 1;
        Fixed b2 = b0;
        Fixed a1 = Fixed(2 * (K*K - 1) * N);
        Fixed a2 = Fixed((1 - K/Q + K*K) * N);
        
        BiQuad<Fixed>::init(b0, b1, b2, a1, a2);
    }
};

template <class Fixed, std::size_t sampleRate = 48000>
class HighPass : public BiQuad<Fixed> {
public:
    HighPass()
    {
    }

    HighPass(double F, double Q)
    {
        double K = std::tan(M_PI * F / sampleRate);
        double N = 1 / (1 + K/Q + K*K);
        Fixed b0 = Fixed(N);
        Fixed b1 = -(b0 << 1);
        Fixed b2 = b0;
        Fixed a1 = Fixed(2 * (K*K - 1) * N);
        Fixed a2 = Fixed((1 - K/Q + K*K) * N);
        
        BiQuad<Fixed>::init(b0, b1, b2, a1, a2);
    }
};

template <class Fixed, std::size_t sampleRate = 48000>
class BandPass {
public:
    BandPass()
    {
    }

    BandPass(double center, double bandwidth, double Q)
        : lowPass_(center + bandwidth/2, Q), 
          highPass_(center - bandwidth/2, Q)
    {
    }

    void init()
    {
        lowPass_.init();
        highPass_.init();
    }

    Fixed operator()(Fixed x)
    {
        return lowPass_(highPass_(x));
    }
private:
    LowPass<Fixed, sampleRate> lowPass_;
    HighPass<Fixed, sampleRate> highPass_;
};
