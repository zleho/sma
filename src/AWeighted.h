#pragma once

#include "BandPass.h"
#include "RMSdB.h"

template <class Fixed>
class AWeighted {
public:
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
            double d = high - low / 3;
            double l = low;
            for (int j = 0; l < high; l += d, ++j) {
                bands_[3*i + j] = BandPass<Fixed>(l + d / 2, d, Q);
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

    bool step(Fixed x, double& val)
    {
        Fixed xx = Fixed(0);
        for (auto& band : bands_)
            xx += band.weight() * band(x);

        if (rms_.step(xx, val)) {
            init();
            return true;
        }

        return false;
    }
private:
    BandPass<Fixed> bands_[30]; // 3*10 third octave
    RMSdB<Fixed> rms_;
};

