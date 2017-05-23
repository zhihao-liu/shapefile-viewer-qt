#ifndef SHAPEDATA_H
#define SHAPEDATA_H

#include <memory>
#include <string>
#include "../shapelib/shapefil.h"
#include "supports.h"

class QPoint;
class QPainter;
class QColor;

namespace cl
{
class GraphicAssistant;

class ShapePrivate;
class Shape
{
public:
    virtual ~Shape();
    std::string name() const;
    int countRecords() const;
    Bounds const& bounds() const;
    SHPHandle const& handle() const ;
    SHPTree const& tree() const;

    // return the number of records hit according to the index tree
    virtual int draw(QPainter& painter, GraphicAssistant const& assistant) const = 0;

protected:
    Shape(SHPHandle shpHandle, std::string name);
    std::unique_ptr<ShapePrivate> _private;
};

class Point: public Shape
{
public:
    Point(SHPHandle shpHandle, std::string name): Shape(shpHandle, name) {}
    virtual ~Point() {}
    virtual int draw(QPainter& painter, GraphicAssistant const& assistant) const;
};

class Polyline: public Shape
{
public:
    Polyline(SHPHandle shpHandle, std::string name): Shape(shpHandle, name) {}
    virtual ~Polyline() {}
    virtual int draw(QPainter& painter, GraphicAssistant const& assistant) const;
};

class Polygon: public Shape
{
public:
    Polygon(SHPHandle shpHandle, std::string name): Shape(shpHandle, name) {}
    virtual ~Polygon() {}
    virtual int draw(QPainter& painter, GraphicAssistant const& assistant) const;
};

enum SHAPE_PROVIDER
{ESRI = 0, AUTODESK, OTHERS};

class ShapeFactory
{
public:
    virtual ~ShapeFactory() {}
    virtual std::shared_ptr<Shape> createShape(std::string path) const = 0;

protected:
    ShapeFactory() {}
};

class ShapeFactoryESRI: ShapeFactory
{
public:
    virtual ~ShapeFactoryESRI() {}
    virtual std::shared_ptr<Shape> createShape(std::string path) const;
    static ShapeFactory const& instance();

private:
    ShapeFactoryESRI() {}
    static std::unique_ptr<ShapeFactory> _instance;
};
}

#endif // SHAPEDATA_H
