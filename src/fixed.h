#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <iostream>

namespace fixie {
    using size_type = std::size_t;
    using int8 = std::int8_t;
    using int16 = std::int16_t;
    using int32 = std::int32_t;
    using int64 = std::int64_t;

    template <
        typename Int,
        std::size_t Q,
        typename = std::enable_if_t<std::is_integral<Int>::value>,
        typename = std::enable_if_t<Q <= (sizeof(Int) << 3)>
    > struct fixed {
        using int_type =  Int;
        static constexpr size_type power() { return Q; }
        static constexpr int_type denom() { return 1 << Q; }

        int_type repr;

        fixed() : repr(0) 
        {
        }
        
        template <typename Floating, typename = std::enable_if_t<std::is_floating_point<Floating>::value>>
        explicit fixed(Floating v) : repr(v * denom()) 
        {
        }
        
        template <typename Integer, typename = std::enable_if_t<std::is_integral<Integer>::value>>
        explicit fixed(Integer v, bool scale = true) : repr(scale ? v << power() : v) 
        {
        }

        fixed<Int, Q>& operator+=(const fixed<Int, Q>& other)
        {
            repr += other.repr;
            return *this;
        }

        fixed<Int, Q>& operator-=(const fixed<Int, Q>& other)
        {
            repr -= other.repr;
            return *this;
        }

        fixed<Int, Q>& operator*=(const fixed<Int, Q>& other)
        {
            repr *= other.repr;
            repr >>= Q;
            return *this;
        }
        
        fixed<Int, Q>& operator/=(const fixed<Int, Q>& other)
        {
            repr <<= Q; 
            repr /= other.repr;
            return *this;
        }

        template <typename Int2, typename = std::enable_if_t<std::is_integral<Int2>::value>>
        fixed<Int, Q>& operator*=(Int2 other)
        {
            repr *= other;
            return *this;
        }

        template <typename Int2, typename = std::enable_if_t<std::is_integral<Int2>::value>> 
        fixed<Int, Q>& operator/=(Int2 other)
        {
            repr /= other;
            return *this;
        }

        template <typename Int2, typename = std::enable_if_t<std::is_integral<Int2>::value>> 
        fixed<Int, Q>& operator<<=(Int2 other)
        {
            repr <<= other;
            return *this;
        }

        template <typename Int2, typename = std::enable_if_t<std::is_integral<Int2>::value>> 
        fixed<Int, Q>& operator>>=(Int2 other)
        {
            repr >>= other;
            return *this;
        }

        bool operator<(const fixed<Int, Q>& other) const
        {
            return repr < other.repr;
        }

        bool operator==(const fixed<Int, Q>& other) const
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
    inline fixed<Int, Q> operator-(fixed<Int, Q> f)
    {
        f *= -1;
        return f;
    }

    template <typename Int, std::size_t Q>
    inline fixed<Int, Q> operator+(fixed<Int, Q> lhs, const fixed<Int, Q>& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    template <typename Int, std::size_t Q>
    inline fixed<Int, Q> operator-(fixed<Int, Q> lhs, const fixed<Int, Q>& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    template <typename Int, std::size_t Q>
    inline fixed<Int, Q> operator*(fixed<Int, Q> lhs, const fixed<Int, Q>& rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    template <typename Int, std::size_t Q>
    inline fixed<Int, Q> operator/(fixed<Int, Q> lhs, const fixed<Int, Q>& rhs)
    {
        lhs /= rhs;
        return lhs;
    }

    template <typename Int, typename Int2, std::size_t Q>
    inline fixed<Int, Q> operator*(fixed<Int, Q> lhs, Int2 rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    template <typename Int, typename Int2, std::size_t Q>
    inline fixed<Int, Q> operator/(fixed<Int, Q> lhs, Int2 rhs)
    {
        lhs /= rhs;
        return lhs;
    }

    template <typename Int, typename Int2, std::size_t Q>
    inline fixed<Int, Q> operator<<(fixed<Int, Q> lhs, Int2 rhs)
    {
        lhs <<= rhs;
        return lhs;
    }

    template <typename Int, typename Int2, std::size_t Q>
    inline fixed<Int, Q> operator>>(fixed<Int, Q> lhs, Int2 rhs)
    {
        lhs >>= rhs;
        return lhs;
    }

    template <typename Int, std::size_t Q>
    bool operator !=(const fixed<Int, Q>& lhs, const fixed<Int, Q>& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename Int, std::size_t Q>
    bool operator <=(const fixed<Int, Q>& lhs, const fixed<Int, Q>& rhs)
    {
        return lhs < rhs || lhs == rhs;
    }

    template <typename Int, std::size_t Q>
    bool operator >=(const fixed<Int, Q>& lhs, const fixed<Int, Q>& rhs)
    {
        return !(lhs < rhs);
    }

    template <typename Int, std::size_t Q>
    bool operator >(const fixed<Int, Q>& lhs, const fixed<Int, Q>& rhs)
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
    fixed<Int, Q> log2(fixed<Int, Q> x)
    {
        Int log2_floor = (sizeof(Int) << 3) - 1 - Q - clz(x.repr);
        auto y = fixed<Int, Q>(log2_floor);
        auto b = fixed<Int, Q>(1) >> 1;
        x >>= log2_floor;

        for (std::size_t i = 0; i < Q; ++i) {
            x *= x;
            if (x >= fixed<Int, Q>(2)) {
                x >>= 1;
                y += b;
            }
            b >>= 1;
        }

        return y;
    }

    using fix15s = fixed<int16, 15>;
    using fix16i = fixed<int32, 16>;
    using fix16ll = fixed<int64, 16>;
    using fix32ll = fixed<int64, 32>;
    using fix15ll = fixed<int64, 15>;
    namespace test {
        inline void fixed_test() {
            auto x = fix16ll(1, false);
            auto y = fix16ll(-16); 
            while (y < fix16ll(16)) {
                assert(log2(x) == y);
                x <<= 1; 
                y += fix16ll(1);
            }

            assert(log2(fix16ll(10)).repr == 217705);
        }
    }
}
