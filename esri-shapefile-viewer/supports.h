#ifndef SUPPORTS_H
#define SUPPORTS_H

#include <QRect>
#include <QPoint>

namespace cl
{
template<typename Type>
class Pair
{
public:
    Pair() {}

    Pair(Type x, Type y)
        : _x(x), _y(y) {}

    Pair(QPoint const& point)
        : _x(point.x()), _y(point.y()) {}

    template<typename thatType>
    Pair(Pair<thatType> const& that)
        : _x(that.x()), _y(that.y()) {}

    Type const& x() const { return _x; }
    Type const& y() const { return _y; }

    QPoint toPoint() const { return QPoint(_x, _y); }

    Pair operator+ (Pair const& that) const
    { return Pair(_x + that._x, _y + that._y); }

    Pair operator- (Pair const& that) const
    { return Pair(_x - that._x, _y - that._y); }

    Pair operator* (Pair const& that) const
    { return Pair(_x*  that._x, _y*  that._y); }

    Pair operator/ (Pair const& that) const
    { return Pair(_x / that._x, _y / that._y); }

    template<typename numType>
    Pair operator* (numType const& number) const
    { return Pair(_x*  number, _y*  number); }

    template<typename numType>
    Pair operator/ (numType const& number) const
    { return Pair(_x / number, _y / number); }

    Type smaller()
    { return _x < _y ? _x : _y; }

    Type larger()
    { return _x > _y ? _x : _y; }

    static Pair<int> computeCenter(QRect const& rect)
    {
        return Pair<int>((rect.left() + rect.right())*  0.5,
                         (rect.top() + rect.bottom())*  0.5);
    }

private:
    Type _x, _y;
};

class Bounds
{
public:
    Bounds() {}
    Bounds(double xMin, double yMin, double xMax, double yMax)
        : _xMin(xMin), _yMin(yMin), _xMax(xMax), _yMax(yMax) {}
    Bounds(Pair<double> xyMin, Pair<double> xyMax)
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
}

#endif // SUPPORTS_H
