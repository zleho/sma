#pragma once

#include "RMSdB.h"
#include "BiQuad.h"

template <class Fixed>
class ITUBS1770 {
public:
    using FixedType = Fixed;
    static constexpr double max()
    {
        return RMSdB<Fixed>::max();
    }

    ITUBS1770()
    {
    }

    explicit ITUBS1770(std::size_t size)
        : stage1_(
            Fixed(1.53512485958697),
            Fixed(-2.69169618940638),
            Fixed(1.19839281085285),
            Fixed(-1.69065929318241),
            Fixed(0.73248077421585)
          ),
          stage2_(
            Fixed(1),
            Fixed(-2),
            Fixed(1),
            Fixed(-1.99004745483398),
            Fixed(0.99007225036621)
          ),
          rms_(size)
    {
    }

    bool step(Fixed x, Fixed& val)
    {
        if (rms_.step(stage2_(stage1_(x)), val)) {
            stage1_.init();
            stage2_.init();
            return true;
        }

        return false;
    }
private:
    BiQuad<Fixed> stage1_;
    BiQuad<Fixed> stage2_;
    RMSdB<Fixed> rms_;
};


