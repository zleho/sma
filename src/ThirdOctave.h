#pragma once

#include "BandPass.h"
#include "RMSdB.h"

template <class Fixed>
class ThridOctave {
public:
    using FixedType = Fixed;
    static constexpr double max()
    {
        return RMSdB<Fixed>::max();
    }

    ThridOctave()
    {
    }

    explicit ThridOctave(std::size_t size)
    {
    }

    bool step(Fixed x, double& val)
    {
        return false;
    }
private:
    BandPass<Fixed> temp_;
};

