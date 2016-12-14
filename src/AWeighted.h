#pragma once

#include "BandPass.h"
#include "RMSdB.h"

template <class Fixed>
class AWeighted {
public:
    static double calcWeight(double Fc)
    {
        double f = Fc * Fc;
        double a = 148840000.0 * f * f;
        double b = (f + 424.36) * std::sqrt((f + 11599.29)*(f + 544496.41)) * (f + 148840000.0);
        return a / b;
    }

    using FixedType = Fixed;
    static constexpr double max()
    {
        return RMSdB<Fixed>::max();
    }

    AWeighted()
    {
        double Q = 1 / std::sqrt(2);
        double low = 20.0;
        for (int i = 0; i < 10; ++i, low *= 2) {
            double high = 2 * low;
            double d = (high - low) / 3;
            double l = low;
            for (int j = 0; l < high; l += d, ++j) {
                double center = l + d / 2;
                auto index = 3*i + j;
                bands_[index] = BandPass<Fixed>(center, d, Q);
                weights_[index] = Fixed(calcWeight(center));
            }
        }
    }

    explicit AWeighted(std::size_t size) : AWeighted()
    {
        rms_ = RMSdB<Fixed>(size);
        init();
    }

    void init()
    {
        for (auto& band : bands_)
            band.init();
    }

    bool step(Fixed x, Fixed& val)
    {
        Fixed xx = Fixed(0);
        for (auto i = 0u; i < bands_.size(); ++i)
            xx += weights_[i] * bands_[i](x);

        if (rms_.step(xx, val)) {
            init();
            return true;
        }

        return false;
    }
private:
    std::array<BandPass<Fixed>, 30> bands_; // 3*10 third octave
    std::array<Fixed, 30> weights_;
    RMSdB<Fixed> rms_;
};

