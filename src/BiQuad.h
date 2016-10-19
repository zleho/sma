#pragma once

template <class Fixed>
class BiQuad {
public:
    BiQuad()
    {
    }

    BiQuad(Fixed b0, Fixed b1, Fixed b2, Fixed a1, Fixed a2)
        : b0_(b0), b1_(b1), b2_(b2), a1_(a1), a2_(a2)
    {
        init();
    }

    void init()
    {
        w_ = w1_ = w2_ = Fixed(0);
    }

    Fixed operator()(Fixed x)
    {
        w_ = x - a1_*w1_ - a2_*w2_;
        auto y = b0_*w_ + b1_*w1_ + b2_*w2_;
        w2_ = w1_;
        w1_ = w_;
        return y;
    }
private:
    Fixed b0_, b1_, b2_;
    Fixed a1_, a2_;
    Fixed w_, w1_, w2_;
};

