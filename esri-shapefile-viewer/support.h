#ifndef SUPPORT_H
#define SUPPORT_H

#include <QRect>
#include <QPoint>
#include <QSize>
#include "nsdef.h"

template<typename T>
class cl::Pair
{
public:
    ~Pair<T>() = default;
    Pair<T>() = default;

    Pair<T>(T const& x, T const& y)
        : _x(x), _y(y) {}

    Pair<T>(QPoint const& qPoint)
        : _x(qPoint.x()), _y(qPoint.y()) {}

    Pair<T>(QSize const& qSize)
        : _x(qSize.width()), _y(qSize.height()) {}

    template<typename U>
    Pair<T>(Pair<U> const& other)
        : _x(other.x()), _y(other.y()) {}

    T const& x() const { return _x; }
    T const& y() const { return _y; }

    QPoint toQPoint() const { return QPoint(_x, _y); }
    QSize toQSize() const { return QSize(_x, _y); }

    template<typename U>
    Pair<T> operator+ (Pair<U> const& other) const
    { return Pair<T>(_x + other.x(), _y + other.y()); }

    template<typename U>
    Pair<T> operator- (Pair<U> const& other) const
    { return Pair<T>(_x - other.x(), _y - other.y()); }

    template<typename U>
    Pair<T> operator* (Pair<U> const& other) const
    { return Pair<T>(_x*  other.x(), _y*  other.y()); }

    template<typename U>
    Pair<T> operator/ (Pair<U> const& other) const
    { return Pair<T>(_x / other.x(), _y / other.y()); }

    template<typename NumT>
    Pair<T> operator* (NumT const& number) const
    { return Pair(_x * number, _y * number); }

    template<typename NumT>
    Pair<T> operator/ (NumT const& number) const
    { return Pair(_x / number, _y / number); }

    T const& smaller()
    { return _x < _y ? _x : _y; }

    T const& larger()
    { return _x > _y ? _x : _y; }

private:
    T _x, _y;
};

template <typename T>
class cl::Rect
{
public:
    ~Rect<T>() = default;
    Rect<T>() = default;

    Rect<T>(T const& xMin, T const& yMin, T const& xMax, T const& yMax)
        : _cornerMin(xMin, yMin), _cornerMax(xMax, yMax) {}
    Rect<T>(Pair<T> const& cornerMin, Pair<T> const& cornerMax)
        : _cornerMin(cornerMin), _cornerMax(cornerMax) {}

    Rect<T>(T const* xyMin, T const* xyMax)
    {
        if (xyMin == nullptr || xyMax == nullptr)
            return;

        _cornerMin = Pair<T>(xyMin[0], xyMin[1]);
        _cornerMax = Pair<T>(xyMax[0], xyMax[1]);
    }

    Rect<T>(QRect const& qRect)
        : _cornerMin(qRect.topLeft()), _cornerMax(qRect.bottomRight()) {}

    QRect toQRect() { return QRect(_cornerMin.toQPoint(), _cornerMax.toQPoint()); }


    T const& xMin() const { return _cornerMin.x(); }
    T const& yMin() const { return _cornerMin.y(); }
    T const& xMax() const { return _cornerMax.x(); }
    T const& yMax() const { return _cornerMax.y(); }

    Pair<T> center() const { return (_cornerMin + _cornerMax) * 0.5; }
    Pair<T> range() const { return _cornerMax - _cornerMin; }

    T xRange() const { return _cornerMax.x() - _cornerMin.x(); }
    T yRange() const { return _cornerMax.y() - _cornerMin.y(); }

private:
    Pair<T> _cornerMin, _cornerMax;
};

#endif // SUPPORT_H
