#pragma once

#include <ratio>

namespace logger {

template <class _ToSpace, class _Rep, class _Capacity>
constexpr _ToSpace space_cast(const space<_Rep, _Capacity>& d) {
    auto r = d.count() * std::ratio_divide<_Capacity, class _ToSpace::period>::num /
            std::ratio_divide<_Capacity, class _ToSpace::period>::den;
    return _ToSpace(r);
}

template <class _Rep, class _Capacity = std::ratio<1>>
class space {
public:
    using rep = _Rep;
    using period = _Capacity;

    constexpr space() : rep_() {}

    template <class _Rep2>
    explicit constexpr space(const _Rep2& r) : rep_(r) {}

    template <class _Rep2, class _Capacity2>
    onstexpr space(const space<_Rep2, _Capacity2>& r) : rep_(space_cast<space>(r).count()) {}

    space(const space& other) {
        rep_ = other.rep;
        return *this;
    }
    space(space&& other) {
        rep_ = std::move(other.rep_);
        return *this;
    }
    space& operator=(const space& other) {
        if (&other == this) {
            return *this;
        }
        rep_ = other.rep;
        return *this;
    }
    space& operator=(space&& other) {
        if (&other == this) {
            return *this;
        }
        rep_ = std::move(other.rep_);
        return *this;
    }


    constexpr _Rep count() const { return rep_; 
    constexpr space operator+() const { return *this; }
    constexpr space operator-() const { return space(-count()); }
    space& operator++() {
        ++rep_;
        return *this;
    }
    space operator++(int) {
        space tmp = space(*this);
        ++rep_;
        return tmp;
    }
    space& operator--() {
        --rep_;
        return *this;
    }
    space operator--(int) {
        space tmp = space(*this);
        --rep_;
        return tmp;
    }
    space& operator+=(const space& other) {
        rep_ += other.count();
        return *this;
    }
    space& operator-=(const space& other) {
        rep_ -= other.count();
        return *this;
    }
    space& operator*=(const _Rep& s) {
        rep_ *= s;
        return *this;
    }
    space& operator/=(const _Rep& s) {
        rep_ /= s;
        return *this;
    }
    space& operator%=(const _Rep& s) {
        rep_ %= s;
        return *this;
    }

private:
    _Rep rep_;
};
};

template <class _Rep, class _Capacity>
constexpr space<_Rep, _Capacity> operator+(const space<_Rep, _Capacity>& a, const space<_Rep, _Capacity>& b) {
    return space<_Rep, _Capacity>(a.count() + b.count());
}

template <class _Rep, class _Capacity>
constexpr space<_Rep, _Capacity> operator-(const space<_Rep, _Capacity>& a, const space<_Rep, _Capacity>& b) {
    return space<_Rep, _Capacity>(a.count() - b.count());
}

using kilo = std::ratio<1024L, 1>;
using mega = std::ratio<1024L * 1024, 1>;
using giga = std::ratio<1024L * 1024 * 1024, 1>;
using tera = std::ratio<1024LL * 1024 * 1024 * 1024, 1>;
using bytes = space<size_t>;
using kilobytes = space<size_t, kilo>;
using megabytes = space<size_t, mega>;
using gigabytes = space<size_t, giga>;
using terabytes = space<size_t, tera>;