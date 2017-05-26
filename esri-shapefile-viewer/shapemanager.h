#ifndef SHAPEMANAGER_H
#define SHAPEMANAGER_H

#include <memory>
#include <string>
#include <vector>
#include "../shapelib/shapefil.h"
#include "supports.h"

class MainWindow;
class QPainter;
class QColor;
class QTime;
class QPoint;
class QString;

namespace cl
{
class GraphicAssistant;
class Shape;

class ShapeDocsObserver
{
public:
    virtual void updateDisplay() = 0;
    virtual QRect const paintingRect() const = 0;
    virtual void setLabel(QString const& ) {}

protected:
    ShapeDocsObserver() {}
};

class ShapeDocsPrivate;
class ShapeDocs
{
public:
    ShapeDocs();
    GraphicAssistant& assistant() const;
    void setObserver(ShapeDocsObserver& observer);
    void refresh() const;
    bool isEmpty() const;
    bool addShape(std::string const& path);
    void removeShape(std::string const& name);
    void drawAllShapes(QPainter& painter) const;
    void clear();
    std::string const& nameOf(int index) const;
    std::shared_ptr<Shape const> findByName(std::string const& name) const;
    int listSize() const;
    Bounds computeGlobalBounds() const;

private:
    std::unique_ptr<ShapeDocsPrivate> _private;
};

class ShapeManager
{
public:
    static ShapeDocs& data();

private:
    static std::unique_ptr<ShapeDocs> _data;
};

class GraphicAssistantPrivate;
class GraphicAssistant
{
public:
    GraphicAssistant(ShapeDocs const& refDocs);
    void setPainterRect(QRect const& paintingRect);
    Pair<int> mapToDisplayXY(Pair<double> mapXY) const;
    Pair<double> displayToMapXY(Pair<int> displayXY) const;
    QPoint computePointOnDisplay(SHPObject const& record, int ptIndex) const;
    Bounds computeMapHitBounds() const;
    void zoomToAll();
    void zoomToLayer(Shape const& layer);
    void zoomAtCursor(QPoint const& mousePos, float scaleFactor);
    void moveStart(QPoint const& startPos);
    void moveProcessing(QPoint const& currentPos);

private:
    std::unique_ptr<GraphicAssistantPrivate> _private;
};
}

#endif // SHAPEMANAGER_H
