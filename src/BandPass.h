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
        double b0 = K * K * N;
        double b1 = 2 * b0;
        double b2 = b0;
        double a1 = 2 * (K*K - 1) * N;
        double a2 = (1 - K/Q + K*K) * N;
        
        BiQuad<Fixed>(b0, b1, b2, a1, a2);
        BiQuad<Fixed>::init();
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
        double b0 = N;
        double b1 = -2 * b0;
        double b2 = b0;
        double a1 = 2 * (K*K - 1) * N;
        double a2 = (1 - K/Q + K*K) * N;
        
        BiQuad<Fixed>(b0, b1, b2, a1, a2);
        BiQuad<Fixed>::init();
    }
};

template <class Fixed, std::size_t sampleRate = 48000>
class BandPass {
    static Fixed calcAWeight(double Fc)
    {
        double f = Fc * Fc;
        double a = 148840000.0 * f * f;
        double b = (f + 424.36) * std::sqrt((f + 11599.29)*(f + 544496.41)) * (f + 148840000.0);
        return a / b;
    }
public:
    BandPass()
    {
    }

    BandPass(double center, double bandwidth, double Q)
        : lowPass_(center + bandwidth/2, Q), 
          highPass_(center - bandwidth/2, Q),
          weight_(calcAWeight(center))
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

    Fixed weight() const
    {
        return weight_;
    }
private:
    LowPass<Fixed, sampleRate> lowPass_;
    HighPass<Fixed, sampleRate> highPass_;
    Fixed weight_;
};
