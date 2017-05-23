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

namespace cl
{
class GraphicAssistant;
class Shape;

class ShapeDocsPrivate;
class ShapeDocs
{
public:
    ShapeDocs(MainWindow& observer);
    GraphicAssistant& assistant() const;
    void refresh() const;
    bool isEmpty() const;
    bool addShape(std::string path);
    void removeShape(std::string id);
    void drawAllShapes(QPainter& painter) const;
    void clear();
    std::string nameOf(int index) const;
    std::shared_ptr<Shape const> findByName(std::string name) const;
    int listSize() const;
    Bounds computeGlobalBounds() const;

private:
    std::unique_ptr<ShapeDocsPrivate> _private;
};

class ShapeManager
{
public:
    static void setData(MainWindow& observer);
    static ShapeDocs& data();

private:
    static std::unique_ptr<ShapeDocs> _data;
};

class Shape;
class GraphicAssistant
{
public:
    GraphicAssistant(ShapeDocs const& refDocs);
    void setPainterRect(QRect const& painterRect);
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
    ShapeDocs const& _refDocs;
    Pair<double> _mapOrigin;
    Pair<int> _displayOrigin;
    float _scaleToDisplay;
    QRect _painterRect;
};
}

#endif // SHAPEMANAGER_H
