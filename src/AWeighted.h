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
    }

    explicit AWeighted(std::size_t size)
    {
    }

    bool step(Fixed x, double& val)
    {
        return false;
    }
private:
    BandPass<Fixed> temp_;
};

