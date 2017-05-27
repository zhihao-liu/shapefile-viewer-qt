#ifndef SUPPORTS_H
#define SUPPORTS_H

#include <QRect>
#include <QPoint>
#include <QPainter>
#include "nsdef.h"

template<typename T>
class cl::Pair
{
public:
    Pair<T>() {}

    Pair<T>(T x, T y)
        : _x(x), _y(y) {}

    Pair<T>(QPoint const& point)
        : _x(point.x()), _y(point.y()) {}

    template<typename U>
    Pair<T>(Pair<U> const& other)
        : _x(other.x()), _y(other.y()) {}

    T const& x() const { return _x; }
    T const& y() const { return _y; }

    QPoint toPoint() const { return QPoint(_x, _y); }

    Pair<T> operator+ (Pair const& other) const
    { return Pair(_x + other._x, _y + other._y); }

    Pair<T> operator- (Pair const& other) const
    { return Pair(_x - other._x, _y - other._y); }

    Pair<T> operator* (Pair const& other) const
    { return Pair(_x*  other._x, _y*  other._y); }

    Pair<T> operator/ (Pair const& other) const
    { return Pair(_x / other._x, _y / other._y); }

    template<typename NumT>
    Pair<T> operator* (NumT const& number) const
    { return Pair(_x*  number, _y*  number); }

    template<typename NumT>
    Pair<T> operator/ (NumT const& number) const
    { return Pair(_x / number, _y / number); }

    T smaller()
    { return _x < _y ? _x : _y; }

    T larger()
    { return _x > _y ? _x : _y; }

    static Pair<int> computeCenter(QRect const& rect)
    {
        return Pair<int>((rect.left() + rect.right())*  0.5,
                         (rect.top() + rect.bottom())*  0.5);
    }

private:
    T _x, _y;
};

class cl::Bounds
{
public:
    Bounds() {}
    Bounds(double xMin, double yMin, double xMax, double yMax)
        : _xMin(xMin), _yMin(yMin), _xMax(xMax), _yMax(yMax) {}
    Bounds(Pair<double> const& xyMin, Pair<double> const& xyMax)
        : _xMin(xyMin.x()), _yMin(xyMin.y()), _xMax(xyMax.x()), _yMax(xyMax.y()) {}

    void set(double const* xyMin, double const* xyMax)
    {
        if (xyMin == nullptr || xyMax == nullptr)
            return;
        _xMin = xyMin[0]; _yMin = xyMin[1];
        _xMax = xyMax[0]; _yMax = xyMax[1];
    }


    double xMin() const { return _xMin; }
    double yMin() const { return _yMin; }
    double xMax() const { return _xMax; }
    double yMax() const { return _yMax; }

    double xRange() const { return _xMax - _xMin; }
    double yRange() const { return _yMax - _yMin; }

    double xCenter() const { return (_xMin + _xMax)*  0.5; }
    double yCenter() const { return (_yMin + _yMax)*  0.5; }

private:
    double _xMin, _yMin, _xMax, _yMax;
};

#endif // SUPPORTS_H
