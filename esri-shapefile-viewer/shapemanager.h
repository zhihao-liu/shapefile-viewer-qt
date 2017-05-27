#ifndef SHAPEMANAGER_H
#define SHAPEMANAGER_H

#include <string>
#include <memory>
#include <vector>
#include <list>
#include "../shapelib/shapefil.h"
#include "nsdef.h"
#include "supports.h"

class MainWindow;
class QPainter;
class QColor;
class QTime;
class QPoint;
class QString;

class cl::DataManagement::ShapeDocsObserver
{
public:
    virtual void updateDisplay() = 0;
    virtual QRect const paintingRect() const = 0;
    virtual void setLabel(QString const& ) {}

protected:
    ShapeDocsObserver() {}
};

class cl::DataManagement::ShapeDocs
{
public:
    typedef std::list<std::shared_ptr<Graphics::Shape const>>::iterator LayerIterator;

    ShapeDocs();
    Graphics::GraphicAssistant& assistant() const;
    void setObserver(ShapeDocsObserver& observer);
    void refresh() const;
    bool isEmpty() const;
    bool addLayer(std::string const& path);
    void removeLayer(LayerIterator layerItr);
    void rearrangeLayer(LayerIterator fromItr, LayerIterator toItr);
    void clearAllLayers();
    void paintAllLayers(QPainter& painter) const;
    void clear();
    std::vector<std::string const*> rawNameList() const;
    LayerIterator findByName(std::string const& name) const;
    bool layerNotFound(LayerIterator layerItr) const;
    int listSize() const;
    Bounds computeGlobalBounds() const;

private:
    std::unique_ptr<ShapeDocsPrivate> _private;
};

class cl::DataManagement::ShapeManager
{
public:
    static ShapeDocs& data();

private:
    static std::unique_ptr<ShapeDocs> _data;
};

class cl::Graphics::GraphicAssistant
{
public:
    GraphicAssistant(DataManagement::ShapeDocs const& refDocs);
    void setPainterRect(QRect const& paintingRect);
    Pair<int> mapToDisplayXY(Pair<double> const& mapXY) const;
    Pair<double> displayToMapXY(Pair<int> const& displayXY) const;
    QPoint computePointOnDisplay(SHPObject const& record, int ptIndex) const;
    Bounds computeMapHitBounds() const;
    void zoomToAll();
    void zoomToLayer(DataManagement::ShapeDocs::LayerIterator layerItr);
    void zoomAtCursor(QPoint const& mousePos, float scaleFactor);
    void translationStart(QPoint const& startPos);
    void translationProcessing(QPoint const& currentPos);

private:
    std::unique_ptr<GraphicAssistantPrivate> _private;
};

#endif // SHAPEMANAGER_H
