#ifndef SHAPEMANAGER_H
#define SHAPEMANAGER_H

#include <string>
#include <memory>
#include <vector>
#include <list>
#include "../shapelib/shapefil.h"
#include "nsdef.h"
#include "support.h"

class MainWindow;
class QPainter;
class QColor;
class QTime;
class QPoint;
class QString;

class cl::DataManagement::ShapeDoc
{
public:
    bool isEmpty() const;
    QString drawAllLayers(QPainter& painter, Graphics::GraphicAssistant const& assistant) const;

    bool addLayer(std::string const& path);
    void removeLayer(LayerIterator layerItr);
    void rearrangeLayer(LayerIterator fromItr, LayerIterator toItr);
    void clearAllLayers();

    std::vector<std::string const*> rawNameList() const;
    LayerIterator findByName(std::string const& name); // Cannot be marked as const.
    bool layerNotFound(LayerIterator layerItr) const;
    int layerCount() const;
    Rect<double> computeGlobalBounds() const;

private:
    std::list<std::shared_ptr<Graphics::Shape>> _layerList;
};

class cl::Graphics::GraphicAssistant
{
public:
    ~GraphicAssistant();
    GraphicAssistant(DataManagement::ShapeDoc const& refDoc);

    void setPaintingRect(Rect<int> const& paintingRect);
    Pair<int> mapToDisplayXY(Pair<double> const& mapXY) const;
    Pair<double> displayToMapXY(Pair<int> const& displayXY) const;
    Pair<int> computePointOnDisplay(SHPObject const& record, int ptIndex) const;
    Rect<double> computeMapHitBounds() const;

    void zoomToAll();
    void zoomToLayer(LayerIterator layerItr);
    void zoomAtCursor(Pair<int> const& mousePos, float scaleFactor);
    void translationStart(Pair<int> const& startPos);
    void translationProcessing(Pair<int> const& currentPos);

    Rect<int> const& paintingRect() const;
    float scale() const;

private:
    class GraphicAssistantPrivate;
    std::unique_ptr<GraphicAssistantPrivate> _private;
};

class cl::DataManagement::Observer
{
public:
    virtual void updateDisplay() {}
};

class cl::DataManagement::DisplayManager
{
public:
    void setObserver(Observer& observer) { _rawObserver = &observer; }
    void setPaintingRect(Rect<int> const& paintingRect) { _assistant.setPaintingRect(paintingRect); }

    void refresh() const { _rawObserver->updateDisplay(); }

    virtual void draw(QPainter& painter) = 0;

    bool isEmpty() { return _shapeDoc.isEmpty(); }

protected:
    DisplayManager();

    DataManagement::ShapeDoc _shapeDoc;
    Graphics::GraphicAssistant _assistant;
    Observer* _rawObserver;
};

class cl::DataManagement::ShapeViewObserver : public DataManagement::Observer
{
public:
    virtual void setLabel(QString const&) {}
};

class cl::DataManagement::ShapeView : public DataManagement::DisplayManager
{
public:
    static ShapeView& instance();

    void draw(QPainter& painter) override;

    bool addLayer(std::string const& path) { bool flag = _shapeDoc.addLayer(path); if (flag) refresh(); return flag; }
    void removeLayer(LayerIterator layerItr) { _shapeDoc.removeLayer(layerItr); refresh(); }
    void rearrangeLayer(LayerIterator fromItr, LayerIterator toItr) { _shapeDoc.rearrangeLayer(fromItr, toItr); refresh(); }
    void clearAllLayers() { _shapeDoc.clearAllLayers(); refresh(); }
    LayerIterator findByName(std::string const& name) { return _shapeDoc.findByName(name); }
    bool layerNotFound(LayerIterator layerItr) const { return _shapeDoc.layerNotFound(layerItr); }
    std::vector<std::string const*> rawNameList() const { return _shapeDoc.rawNameList(); }

    void zoomToAll() { _assistant.zoomToAll(); refresh(); }
    void zoomToLayer(LayerIterator layerItr) { _assistant.zoomToLayer(layerItr); refresh(); }
    void zoomAtCursor(Pair<int> const& mousePos, float scaleFactor) { _assistant.zoomAtCursor(mousePos, scaleFactor); refresh(); }
    void translationStart(Pair<int> const& startPos) { _assistant.translationStart(startPos); refresh(); }
    void translationProcessing(Pair<int> const& currentPos) { _assistant.translationProcessing(currentPos); refresh(); }

    ShapeDoc const& shapeDoc() { return _shapeDoc; }

private:
    ShapeView() = default;

    static std::unique_ptr<ShapeView> _instance;
};

#endif // SHAPEMANAGER_H
