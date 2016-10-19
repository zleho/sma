#pragma once

#include <cmath>
#include "BiQuad.h"

template <class Fixed, std::size_t sampleRate = 48000>
class BandPass {
public:
    BandPass()
    {
    }

    BandPass(double center, double bandwidth, double Q)
    {
        double K = std::tan(M_PI * center / sampleRate);
        double N = 1 / (1 + K/Q + K*K);
        double l_b0 = K * K * N;
        double l_b1 = 2 * l_b0;
        double l_b2 = l_b0;
        double h_b0 = N;
        double h_b1 = -2 * h_b0;
        double h_b2 = h_b0;
        double a1 = 2 * (K*K - 1) * N;
        double a2 = (1 - K/Q + K*K) * N;

        lowPass_ = BiQuad<Fixed>(l_b0, l_b1, l_b2, a1, a2);
        highPass_ = BiQuad<Fixed>(h_b0, h_b1, h_b2, a1, a2);

        init();
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
    BiQuad<Fixed> lowPass_;
    BiQuad<Fixed> highPass_;
};
