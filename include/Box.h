#pragma once

#include "Vector2.h"

namespace math
{

template<typename T>
class Box
{
public:
    T left;
    T top;
    T width; // Must be positive
    T height; // Must be positive

    constexpr Box(T Left = 0, T Top = 0, T Width = 0, T Height = 0) noexcept :
        left(Left), top(Top), width(Width), height(Height)
    {

    }

    constexpr Box(const Vector2<T>& position, const Vector2<T>& size) noexcept :
        left(position.x), top(position.y), width(size.x), height(size.y)
    {

    }

    constexpr T getRight() const noexcept
    {
        return left + width;
    }

    constexpr T getBottom() const noexcept
    {
        return top + height;
    }

    constexpr Vector2<T> getTopLeft() const noexcept
    {
        return Vector2<T>(left, top);
    }

    constexpr Vector2<T> getBottomRight() const noexcept
    {
        return Vector2<T>(left + width, top + height);
    }

    constexpr Vector2<T> getCenter() const noexcept
    {
        return Vector2<T>(left + width / 2, top + height / 2);
    }

    constexpr Vector2<T> getSize() const noexcept
    {
        return Vector2<T>(width, height);
    }

    constexpr T getArea() const noexcept
    {
        return width * height;
    }

    constexpr bool contains(const Vector2<T>& point) const noexcept
    {
        // TODO: assertions to check that width and height are positive
        return left <= point.x && point.x < getRight() &&
            top <= point.y && point.y < getBottom();
    }

    constexpr bool almostContains(const Vector2<T>& point) const noexcept
    {
        // TODO: assertions to check that width and height are positive
        // Check that the point is inside the closure of the box
        return left <= point.x && point.x <= getRight() &&
            top <= point.y && point.y <= getBottom();
    }

    constexpr bool contains(const Box<T>& box) const noexcept
    {
        // TODO: assertions to check that width and height are positive
        return left <= box.left && box.getRight() <= getRight() &&
            top <= box.top && box.getBottom() <= getBottom();
    }

    constexpr bool intersects(const Box<T>& box) const noexcept
    {
        return !(left >= box.getRight() || getRight() <= box.left ||
            top >= box.getBottom() || getBottom() <= box.top);
    }

    constexpr bool almostIntersects(const Box<T>& box) const noexcept
    {
        return !(left > box.getRight() || getRight() < box.left ||
            top > box.getBottom() || getBottom() < box.top);
    }
};

}