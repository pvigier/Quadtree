#pragma once

#include <cmath>

namespace math
{

template<typename T>
class Vector2;
template<typename T>
constexpr Vector2<T> operator-(Vector2<T> lhs, const Vector2<T>& rhs) noexcept;

template<typename T>
class Vector2
{
public:
    T x;
    T y;

    constexpr Vector2<T>(T X = 0, T Y = 0) noexcept : x(X), y(Y)
    {

    }

    template<typename U>
    explicit constexpr Vector2<T>(const Vector2<U>& other) noexcept :
        x(static_cast<T>(other.x)), y(static_cast<T>(other.y))
    {

    }

    constexpr bool operator==(const Vector2<T>& other) const noexcept
    {
        return x == other.x && y == other.y;
    }

    constexpr bool operator!=(const Vector2<T>& other) const noexcept
    {
        return !(*this == other);
    }

    constexpr Vector2<T> operator-() const noexcept
    {
        return Vector2<T>(-x, -y);
    }

    constexpr Vector2<T>& operator+=(const Vector2<T>& other) noexcept
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Vector2<T>& operator-=(const Vector2<T>& other) noexcept
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr Vector2<T>& operator*=(T t) noexcept
    {
        x *= t;
        y *= t;
        return *this;
    }

    constexpr Vector2<T>& operator/=(T t) noexcept
    {
        x /= t;
        y /= t;
        return *this;
    }

    constexpr Vector2<T> getOrthogonal() const noexcept
    {
        return Vector2<T>(-y, x);
    }

    constexpr T dot(const Vector2<T>& other) const noexcept
    {
        return x * other.x + y * other.y;
    }

    constexpr T getSquaredNorm() const noexcept
    {
        return dot(*this);
    }

    auto getNorm() const noexcept
    {
        return std::sqrt(getSquaredNorm());
    }

    void normalize() noexcept
    {
        operator/=(getNorm());
    }

    constexpr T getSquaredDistance(const Vector2<T>& other) const noexcept
    {
        return (*this - other).getSquaredNorm();
    }

    auto getDistance(const Vector2<T>& other) const noexcept
    {
        return (*this - other).getNorm();
    }

    constexpr T getDet(const Vector2<T>& other) const noexcept
    {
        return x * other.y - y * other.x;
    }

    constexpr Vector2<T> getMin(const Vector2<T>& other) const noexcept
    {
        return Vector2<T>(std::min(x, other.x), std::min(y, other.y));
    }

    constexpr Vector2<T> getMax(const Vector2<T>& other) const noexcept
    {
        return Vector2<T>(std::max(x, other.x), std::max(y, other.y));
    }
};

template<typename T>
constexpr Vector2<T> operator+(Vector2<T> lhs, const Vector2<T>& rhs) noexcept
{
    lhs += rhs;
    return lhs;
}

template<typename T>
constexpr Vector2<T> operator-(Vector2<T> lhs, const Vector2<T>& rhs) noexcept
{
    lhs -= rhs;
    return lhs;
}

template<typename T>
constexpr Vector2<T> operator*(T t, Vector2<T> vec) noexcept
{
    vec *= t;
    return vec;
}

template<typename T>
constexpr Vector2<T> operator*(Vector2<T> vec, T t) noexcept
{
    return t * vec;
}

template<typename T>
constexpr Vector2<T> operator/(Vector2<T> vec, T t) noexcept
{
    vec /= t;
    return vec;
}

using Vector2i = Vector2<int>;
using Vector2u = Vector2<unsigned int>;
using Vector2s = Vector2<std::size_t>;
using Vector2f = Vector2<float>;
using Vector2d = Vector2<double>;

}
