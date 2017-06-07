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
    Rect<int> _paintingRect;
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

    // Refresh the rect of the painting region on screen.
    _private->_assistant.setPaintingRect(_private->_rawObserver->paintingRect());

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

void Graphics::GraphicAssistant::setPaintingRect(Rect<int> const& paintingRect)
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
    _private->_mapOrigin = (*layerItr)->bounds().center();

    _private->_displayOrigin = _private->_paintingRect.center();

    Pair<float> scaleXY(Pair<float>(_private->_paintingRect.range()) / (*layerItr)->bounds().range());

    // Ensure the objects to be fully covered.
    _private->_scaleToDisplay = COVER * scaleXY.smaller();

    _private->_refDocs.refresh();
}

Rect<double> Graphics::GraphicAssistant::computeMapHitBounds() const
{
    Pair<double> cornerHitMin = displayToMapXY(Pair<int>(_private->_paintingRect.xMin(), _private->_paintingRect.yMax()));
    Pair<double> cornerHitMax = displayToMapXY(Pair<int>(_private->_paintingRect.xMax(), _private->_paintingRect.yMin()));
    return Rect<double>(cornerHitMin, cornerHitMax);
}

DataManagement::ShapeDocs::LayerIterator DataManagement::ShapeDocs::findByName(std::string const& name) const
{
    auto itr = _private->_layerList.begin();
    while (itr != _private->_layerList.end())
    {
        if (name == (*itr)->name())
            break;
        ++itr;
    }

    return itr; // If the name is not found, std::list.end() will be returned.
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
    Rect<double> globalBounds = _private->_refDocs.computeGlobalBounds();

    _private->_mapOrigin = globalBounds.center();

    _private->_displayOrigin = _private->_paintingRect.center();

    Pair<float> scaleXY(Pair<float>(_private->_paintingRect.range()) / globalBounds.range());

    _private->_scaleToDisplay = COVER * scaleXY.smaller();

    _private->_refDocs.refresh();
}

Rect<double> DataManagement::ShapeDocs::computeGlobalBounds() const
{
    if (isEmpty())
        return Rect<double>();

    double xMin = _private->_layerList.front()->bounds().xMin();
    double yMin = _private->_layerList.front()->bounds().yMin();
    double xMax = _private->_layerList.front()->bounds().xMax();
    double yMax = _private->_layerList.front()->bounds().yMax();

    if (_private->_layerList.size() > 1)
        for (auto const& item : _private->_layerList)
        {
            Rect<double> localBounds = item->bounds();
            if (localBounds.xMin() < xMin)
                xMin = localBounds.xMin();
            if (localBounds.yMin() < yMin)
                yMin = localBounds.yMin();
            if (localBounds.xMax() > xMax)
                xMax = localBounds.xMax();
            if (localBounds.yMax() > yMax)
                yMax = localBounds.yMax();
        }

    return Rect<double>(xMin, yMin, xMax, yMax);
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
