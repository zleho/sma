#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <iostream>

namespace fixie {
    template <
        typename Int,
        std::size_t Q,
        typename = std::enable_if_t<std::is_integral<Int>::value>,
        typename = std::enable_if_t<Q <= (sizeof(Int) << 3)>
    > struct Fixed {
        using IntType =  Int;
        static constexpr std::size_t power() { return Q; }
        static constexpr IntType denom() { return 1 << Q; }

        IntType repr;

        Fixed() : repr(0) 
        {
        }
        
        template <typename Floating, typename = std::enable_if_t<std::is_floating_point<Floating>::value>>
        explicit Fixed(Floating v) : repr(v * denom()) 
        {
        }
        
        template <typename Integer, typename = std::enable_if_t<std::is_integral<Integer>::value>>
        explicit Fixed(Integer v, bool scale = true) : repr(scale ? v << power() : v) 
        {
        }

        Fixed<Int, Q>& operator+=(const Fixed<Int, Q>& other)
        {
            repr += other.repr;
            return *this;
        }

        Fixed<Int, Q>& operator-=(const Fixed<Int, Q>& other)
        {
            repr -= other.repr;
            return *this;
        }

        Fixed<Int, Q>& operator*=(const Fixed<Int, Q>& other)
        {
            repr *= other.repr;
            repr >>= Q;
            return *this;
        }
        
        Fixed<Int, Q>& operator/=(const Fixed<Int, Q>& other)
        {
            repr <<= Q; 
            repr /= other.repr;
            return *this;
        }

        template <typename Int2, typename = std::enable_if_t<std::is_integral<Int2>::value>>
        Fixed<Int, Q>& operator*=(Int2 other)
        {
            repr *= other;
            return *this;
        }

        template <typename Int2, typename = std::enable_if_t<std::is_integral<Int2>::value>> 
        Fixed<Int, Q>& operator/=(Int2 other)
        {
            repr /= other;
            return *this;
        }

        template <typename Int2, typename = std::enable_if_t<std::is_integral<Int2>::value>> 
        Fixed<Int, Q>& operator<<=(Int2 other)
        {
            repr <<= other;
            return *this;
        }

        template <typename Int2, typename = std::enable_if_t<std::is_integral<Int2>::value>> 
        Fixed<Int, Q>& operator>>=(Int2 other)
        {
            repr >>= other;
            return *this;
        }

        bool operator<(const Fixed<Int, Q>& other) const
        {
            return repr < other.repr;
        }

        bool operator==(const Fixed<Int, Q>& other) const
        {
            return repr == other.repr;
        }

        template <typename Int2, typename = std::enable_if_t<std::is_integral<Int2>::value>>
        operator Int2() const
        {
            return repr >> Q;
        }
    };
    
    template <typename Int, std::size_t Q>
    inline Fixed<Int, Q> operator-(Fixed<Int, Q> f)
    {
        f *= -1;
        return f;
    }

    template <typename Int, std::size_t Q>
    inline Fixed<Int, Q> operator+(Fixed<Int, Q> lhs, const Fixed<Int, Q>& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    template <typename Int, std::size_t Q>
    inline Fixed<Int, Q> operator-(Fixed<Int, Q> lhs, const Fixed<Int, Q>& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    template <typename Int, std::size_t Q>
    inline Fixed<Int, Q> operator*(Fixed<Int, Q> lhs, const Fixed<Int, Q>& rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    template <typename Int, std::size_t Q>
    inline Fixed<Int, Q> operator/(Fixed<Int, Q> lhs, const Fixed<Int, Q>& rhs)
    {
        lhs /= rhs;
        return lhs;
    }

    template <typename Int, typename Int2, std::size_t Q>
    inline Fixed<Int, Q> operator*(Fixed<Int, Q> lhs, Int2 rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    template <typename Int, typename Int2, std::size_t Q>
    inline Fixed<Int, Q> operator/(Fixed<Int, Q> lhs, Int2 rhs)
    {
        lhs /= rhs;
        return lhs;
    }

    template <typename Int, typename Int2, std::size_t Q>
    inline Fixed<Int, Q> operator<<(Fixed<Int, Q> lhs, Int2 rhs)
    {
        lhs <<= rhs;
        return lhs;
    }

    template <typename Int, typename Int2, std::size_t Q>
    inline Fixed<Int, Q> operator>>(Fixed<Int, Q> lhs, Int2 rhs)
    {
        lhs >>= rhs;
        return lhs;
    }

    template <typename Int, std::size_t Q>
    bool operator !=(const Fixed<Int, Q>& lhs, const Fixed<Int, Q>& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename Int, std::size_t Q>
    bool operator <=(const Fixed<Int, Q>& lhs, const Fixed<Int, Q>& rhs)
    {
        return lhs < rhs || lhs == rhs;
    }

    template <typename Int, std::size_t Q>
    bool operator >=(const Fixed<Int, Q>& lhs, const Fixed<Int, Q>& rhs)
    {
        return !(lhs < rhs);
    }

    template <typename Int, std::size_t Q>
    bool operator >(const Fixed<Int, Q>& lhs, const Fixed<Int, Q>& rhs)
    {
        return !(lhs <= rhs);
    }

    inline int clz(int x)
    {
        return __builtin_clz(x);
    }

    inline long clz(long x)
    {
        return __builtin_clzl(x);
    }

    inline long long clz(long long x)
    {
        return __builtin_clzll(x);
    }

    template <typename Int, std::size_t Q>
    Fixed<Int, Q> log2(Fixed<Int, Q> x)
    {
        Int log2_floor = (sizeof(Int) << 3) - 1 - Q - clz(x.repr);
        auto y = Fixed<Int, Q>(log2_floor);
        auto b = Fixed<Int, Q>(1) >> 1;
        x >>= log2_floor;

        for (std::size_t i = 0; i < Q; ++i) {
            x *= x;
            if (x >= Fixed<Int, Q>(2)) {
                x >>= 1;
                y += b;
            }
            b >>= 1;
        }

        return y;
    }
}
