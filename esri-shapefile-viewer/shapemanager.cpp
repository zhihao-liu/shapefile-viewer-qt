#include "shapemanager.h"
#include <QColor>
#include <QTime>
#include <QPoint>
#include "mainwindow.h"
#include "shapedata.h"

#define COVER 0.9
#define EPS 1E-4

using namespace cl;

class cl::DataManagement::ShapeDocsPrivate
{
    friend class ShapeDocs;

private:
    ShapeDocsPrivate(ShapeDocs& refThis)
        : _refThis(refThis), _assistant(refThis), _rawObserver(nullptr) {}

    ShapeDocs& _refThis;

    std::list<std::shared_ptr<Graphics::Shape const>> _layerList;
    Graphics::GraphicAssistant _assistant;
    ShapeDocsObserver* _rawObserver;
};

class cl::Graphics::GraphicAssistantPrivate
{
    friend class GraphicAssistant;

private:
    GraphicAssistantPrivate(GraphicAssistant& refThis, DataManagement::ShapeDocs const& refDocs)
        : _refThis(refThis), _refDocs(refDocs) {}

    GraphicAssistant& _refThis;

    DataManagement::ShapeDocs const& _refDocs;
    Pair<double> _mapOrigin;
    Pair<int> _displayOrigin;
    float _scaleToDisplay;
    QRect _paintingRect;
};

DataManagement::ShapeDocs::ShapeDocs()
    : _private(std::unique_ptr<ShapeDocsPrivate>
               (new ShapeDocsPrivate(*this))) {}

void DataManagement::ShapeDocs::setObserver(ShapeDocsObserver& observer)
{
    _private->_rawObserver =& observer;
}

void DataManagement::ShapeDocs::paintAllLayers(QPainter& painter) const
{
    //    if (isEmpty())
    //        return;

    // Refresh the QRect of the painting region on screen.
    _private->_assistant.setPainterRect(_private->_rawObserver->paintingRect());

    int countRecordsHit = 0;
    int countRecordsTotal = 0;
    for (auto const& item : _private->_layerList)
    {
        countRecordsHit += item->draw(painter, _private->_assistant);
        countRecordsTotal += item->recordCount();
    }

    float percentageHit = countRecordsHit / (countRecordsTotal + EPS);

    QString msgPercentage = QString::number(percentageHit*  100, 'g', 4);
    QString msgCountHit = QString::number(countRecordsHit);
    QString msgCountTotal = QString::number(countRecordsTotal);

    _private->_rawObserver->setLabel("\t\t Records Hit: " + msgCountHit +
                                     "\t\t Records Total: " + msgCountTotal +
                                     "\t\t Percentage Hit: " + msgPercentage +"%");
}

bool DataManagement::ShapeDocs::addLayer(std::string const& path)
{
    std::shared_ptr<Graphics::Shape> shp = ShapeFactoryESRI::instance().createShape(path);
    if (!shp)
        return false;

    _private->_layerList.push_back(shp);
    if (_private->_layerList.size() == 1)
        _private->_assistant.zoomToLayer(_private->_layerList.begin());

    refresh();
    return true;
}

void DataManagement::ShapeDocs::removeLayer(LayerIterator layerItr)
{
    _private->_layerList.erase(layerItr);

    refresh();
}

void DataManagement::ShapeDocs::rearrangeLayer(LayerIterator fromItr, LayerIterator toItr)
{
    _private->_layerList.insert(toItr, *fromItr);
    _private->_layerList.erase(fromItr);

    refresh();
}

void DataManagement::ShapeDocs::clearAllLayers()
{
    _private->_layerList.clear();

    refresh();
}

void DataManagement::ShapeDocs::refresh() const
{
    _private->_rawObserver->updateDisplay();
}

void DataManagement::ShapeDocs::clear()
{
    _private->_layerList.clear();
}

std::unique_ptr<DataManagement::ShapeDocs> DataManagement::ShapeManager::_data = nullptr;

DataManagement::ShapeDocs& DataManagement::ShapeManager::data()
{
    if (_data == nullptr)
        _data.reset(new DataManagement::ShapeDocs());
    return *_data;
}

bool DataManagement::ShapeDocs::isEmpty() const
{
    return _private->_layerList.empty() ? true : false;
}

int DataManagement::ShapeDocs::listSize() const
{
    return _private->_layerList.size();
}

Graphics::GraphicAssistant& DataManagement::ShapeDocs::assistant() const
{
    return _private->_assistant;
}

Graphics::GraphicAssistant::GraphicAssistant(DataManagement::ShapeDocs const& refDocs)
    : _private(std::unique_ptr<GraphicAssistantPrivate>
               (new GraphicAssistantPrivate(*this, refDocs))) {}

void Graphics::GraphicAssistant::setPainterRect(QRect const& paintingRect)
{
    _private->_paintingRect = paintingRect;
}

QPoint Graphics::GraphicAssistant::computePointOnDisplay(SHPObject const& record, int ptIndex) const
{
    Pair<double> mapXY(record.padfX[ptIndex], record.padfY[ptIndex]);
    Pair<int> displayXY = mapToDisplayXY(mapXY);
    return displayXY.toPoint();
}

Pair<int> Graphics::GraphicAssistant::mapToDisplayXY(Pair<double> const& mapXY) const
{
    return (mapXY - _private->_mapOrigin) * Pair<double>(1, -1) * _private->_scaleToDisplay + Pair<double>(_private->_displayOrigin);
}

Pair<double> Graphics::GraphicAssistant::displayToMapXY(Pair<int> const& displayXY) const
{
    return Pair<double>(displayXY - _private->_displayOrigin) / _private->_scaleToDisplay / Pair<double>(1, -1) + _private->_mapOrigin;
}

// make a specified layer fully displayed and centered
void Graphics::GraphicAssistant::zoomToLayer(std::list<std::shared_ptr<Shape const>>::iterator layerItr)
{
    Shape const& layer = **layerItr;
    _private->_mapOrigin = Pair<double>(layer.bounds().xCenter(),
                                        layer.bounds().yCenter());

    _private->_displayOrigin = Pair<int>::computeCenter(_private->_paintingRect);

    Pair<double> mapRange(layer.bounds().xRange(),
                          layer.bounds().yRange());

    Pair<float> scaleXY(float(_private->_paintingRect.width()) / mapRange.x(),
                        float(_private->_paintingRect.height()) / mapRange.y());

    // Ensure the objects to be fully covered.
    _private->_scaleToDisplay = COVER*  scaleXY.smaller();

    _private->_refDocs.refresh();
}

Bounds Graphics::GraphicAssistant::computeMapHitBounds() const
{
    Pair<double> cornerHitMin = displayToMapXY(Pair<int>(_private->_paintingRect.left(), _private->_paintingRect.bottom()));
    Pair<double> cornerHitMax = displayToMapXY(Pair<int>(_private->_paintingRect.right(), _private->_paintingRect.top()));
    return Bounds(cornerHitMin, cornerHitMax);
}

DataManagement::ShapeDocs::LayerIterator DataManagement::ShapeDocs::findByName(std::string const& name) const
{
    for (auto itr = _private->_layerList.begin(); itr != _private->_layerList.end(); ++itr)
        if (name == (*itr)->name())
            return itr;
}

bool DataManagement::ShapeDocs::layerNotFound(LayerIterator itr) const
{
    return itr == _private->_layerList.end() ? true : false;
}

void Graphics::GraphicAssistant::zoomAtCursor(QPoint const& mousePos, float scaleFactor)
{
    Pair<int> displayOrigin(mousePos);
    Pair<double> mapOrigin = displayToMapXY(displayOrigin);
    _private->_displayOrigin = displayOrigin;
    _private->_mapOrigin = mapOrigin;
    _private->_scaleToDisplay *= scaleFactor;

    _private->_refDocs.refresh();
}

void Graphics::GraphicAssistant::zoomToAll()
{
    Bounds globalBounds = _private->_refDocs.computeGlobalBounds();

    _private->_mapOrigin = Pair<double>(globalBounds.xCenter(),
                                        globalBounds.yCenter());

    _private->_displayOrigin = Pair<int>::computeCenter(_private->_paintingRect);

    Pair<double> mapRange(globalBounds.xRange(),
                          globalBounds.yRange());

    Pair<float> scaleXY(float(_private->_paintingRect.width()) / mapRange.x(),
                        float(_private->_paintingRect.height()) / mapRange.y());

    _private->_scaleToDisplay = COVER*  scaleXY.smaller();

    _private->_refDocs.refresh();
}

Bounds DataManagement::ShapeDocs::computeGlobalBounds() const
{
    if (isEmpty())
        return Bounds();

    double xMin = _private->_layerList.front()->bounds().xMin();
    double yMin = _private->_layerList.front()->bounds().yMin();
    double xMax = _private->_layerList.front()->bounds().xMax();
    double yMax = _private->_layerList.front()->bounds().yMax();

    if (_private->_layerList.size() > 1)
        for (auto const& item : _private->_layerList)
        {
            Bounds localBounds = item->bounds();
            if (localBounds.xMin() < xMin)
                xMin = localBounds.xMin();
            if (localBounds.yMin() < yMin)
                yMin = localBounds.yMin();
            if (localBounds.xMax() > xMax)
                xMax = localBounds.xMax();
            if (localBounds.yMax() > yMax)
                yMax = localBounds.yMax();
        }

    return Bounds(xMin, yMin, xMax, yMax);
}

void Graphics::GraphicAssistant::translationStart(QPoint const& startPos)
{
    Pair<int> displayOriginNew(startPos);
    Pair<double> mapOriginNew = displayToMapXY(displayOriginNew);

    _private->_displayOrigin = displayOriginNew;
    _private->_mapOrigin = mapOriginNew;
}

void Graphics::GraphicAssistant::translationProcessing(QPoint const& currentPos)
{
    _private->_displayOrigin = Pair<int>(currentPos);

    _private->_refDocs.refresh();
}

std::vector<std::string const*> DataManagement::ShapeDocs::rawNameList() const
{
    std::vector<std::string const*> rawNameList;

    // Painting is in the reverse order of the list,
    // thus we have to get a reverse list for display.
    for (auto itr = _private->_layerList.rbegin(); itr != _private->_layerList.rend(); ++itr)
        rawNameList.push_back(&(*itr)->name());

    return rawNameList;
}
