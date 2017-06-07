#ifndef MAP_H
#define MAP_H

#include <vector>
#include "nsdef.h"
#include "shapemanager.h"

class QPainter;

enum class cl::Map::MapStyle
{
    FullElements = 0,
    NoGridLine
};

class cl::Map::Map : public DataManagement::DisplayManager
{
    friend class MapBuilder;

public:
    virtual ~Map() = default;

    virtual void draw(QPainter& painter) override;

    void zoomToAll() { _assistant.zoomToAll(); refresh(); }

    // This is a temporary solution to be refined.
    void setShapeDoc(DataManagement::ShapeDoc const& shapeDoc) { _shapeDoc = shapeDoc; }

protected:
    std::vector< std::unique_ptr<MapElement> > _elements;
};

class cl::Map::MapObserver : public DataManagement::Observer
{

};

class cl::Map::MapElement
{
public:
    virtual ~MapElement() = default;
    virtual void draw(QPainter& painter, Graphics::GraphicAssistant const& assistant) = 0;
};

class cl::Map::GridLine : public MapElement
{
public:
    class Solid;
    class Dot;

    virtual ~GridLine() = default;

    virtual void draw(QPainter& painter, Graphics::GraphicAssistant const& assistant) override;

protected:
    GridLine(Qt::PenStyle lineStyle) : _lineStyle(lineStyle) {}
    Qt::PenStyle _lineStyle;
};

class cl::Map::GridLine::Solid : public GridLine
{
public:
    virtual ~Solid() = default;

    Solid() : GridLine(Qt::SolidLine) {}
};

class cl::Map::GridLine::Dot : public GridLine
{
public:
    virtual ~Dot() = default;

    Dot() : GridLine(Qt::DotLine) {}
};

class cl::Map::NorthPointer : public MapElement
{
public:
    class Flat;
    class Style3D;

    virtual ~NorthPointer() = default;

    virtual void draw(QPainter& painter, Graphics::GraphicAssistant const& assistant) override = 0;

protected:
    NorthPointer() = default;
};

class cl::Map::NorthPointer::Flat : public NorthPointer
{
public:
    virtual ~Flat() = default;

    virtual void draw(QPainter& painter, Graphics::GraphicAssistant const& assistant) override;
};

class cl::Map::NorthPointer::Style3D : public NorthPointer
{
public:
    virtual ~Style3D() = default;

    virtual void draw(QPainter& painter, Graphics::GraphicAssistant const& assistant) override;
};

class cl::Map::ScaleBar : public MapElement
{
public:
    class Text;
    class Graphic;

    virtual ~ScaleBar() = default;

    virtual void draw(QPainter &painter, const Graphics::GraphicAssistant &assistant) override = 0;

protected:
    ScaleBar() = default;
};

class cl::Map::ScaleBar::Text : public ScaleBar
{
public:
    virtual ~Text() = default;

    virtual void draw(QPainter &painter, const Graphics::GraphicAssistant &assistant) override;
};

class cl::Map::ScaleBar::Graphic : public ScaleBar
{
public:
    virtual ~Graphic() = default;

    virtual void draw(QPainter &painter, const Graphics::GraphicAssistant &assistant) override;
};

class cl::Map::MapBuilder
{
    friend class MapDirector;

public:
    class NoGridLine;
    class FullElements;

    virtual ~MapBuilder() = default;

    virtual void buildMap() { _map.reset(new Map()); }
    virtual void buildGridLine() = 0;
    virtual void buildScaleBar() = 0;
    virtual void buildNorthPointer() = 0;

protected:
    MapBuilder() = default;
    std::shared_ptr<Map> _map;
};

class cl::Map::MapBuilder::NoGridLine : public MapBuilder
{
public:
    virtual ~NoGridLine() = default;

    virtual void buildGridLine() override {}

    virtual void buildNorthPointer() override
    {
        _map->_elements.push_back(std::unique_ptr<MapElement>(new NorthPointer::Style3D()));
    }

    virtual void buildScaleBar() override
    {
        _map->_elements.push_back(std::unique_ptr<MapElement>(new ScaleBar::Graphic()));
    }
};

class cl::Map::MapBuilder::FullElements : public MapBuilder
{
public:
    virtual ~FullElements() = default;

    virtual void buildGridLine() override
    {
        _map->_elements.push_back(std::unique_ptr<MapElement>(new GridLine::Solid()));
    }

    virtual void buildNorthPointer() override
    {
        _map->_elements.push_back(std::unique_ptr<MapElement>(new NorthPointer::Flat()));
    }

    virtual void buildScaleBar() override
    {
        _map->_elements.push_back(std::unique_ptr<MapElement>(new ScaleBar::Text()));
    }
};

class cl::Map::MapDirector
{
public:
    MapDirector(MapBuilder* builder) : _builder(builder) {}

    std::shared_ptr<Map> constructMap()
    {
        _builder->buildMap();
        _builder->buildGridLine();
        _builder->buildNorthPointer();
        _builder->buildScaleBar();

        return _builder->_map;
    }

private:
    std::unique_ptr<MapBuilder> _builder;
};

#endif // MAP_H
