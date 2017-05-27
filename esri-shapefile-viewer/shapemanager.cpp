#include "shapemanager.h"
#include <QColor>
#include <QTime>
#include <QPoint>
#include "mainwindow.h"
#include "shapedata.h"

#define COVER 0.9
#define EPS 1e-4

using namespace cl;

class cl::DataManagement::ShapeDocsPrivate
{
    friend class ShapeDocs;

private:
    ShapeDocsPrivate(ShapeDocs& refThis)
        : _refThis(refThis), _assistant(refThis), _rawObserver(nullptr) {}

    ShapeDocs& _refThis;

    std::vector<std::shared_ptr<Graphics::Shape>> _shpList;
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

void DataManagement::ShapeDocs::drawAllShapes(QPainter& painter) const
{
    //    if (isEmpty())
    //        return;

    // Refresh the QRect of the painting region on screen.
    _private->_assistant.setPainterRect(_private->_rawObserver->paintingRect());

    int countRecordsHit = 0;
    int countRecordsTotal = 0;
    for (auto item : _private->_shpList)
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

bool DataManagement::ShapeDocs::addShape(std::string const& path)
{
    std::shared_ptr<Graphics::Shape> shp = ShapeFactoryESRI::instance().createShape(path);
    if (!shp)
        return false;

    _private->_shpList.push_back(shp);
    if (_private->_shpList.size() == 1)
        _private->_assistant.zoomToLayer(*shp);
    refresh();
    return true;
}

void DataManagement::ShapeDocs::removeShape(std::string const& name)
{
    for (auto itr = _private->_shpList.begin(); itr < _private->_shpList.end(); ++itr)
    {
        if ((*itr)->name() == name)
        {
            _private->_shpList.erase(itr);
            break;
        }
    }
    refresh();
}

void DataManagement::ShapeDocs::refresh() const
{
    _private->_rawObserver->updateDisplay();
}

void DataManagement::ShapeDocs::clear()
{
    _private->_shpList.clear();
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
    return _private->_shpList.size() == 0 ? true : false;
}

std::string const& DataManagement::ShapeDocs::nameOf(int index) const
{
    return _private->_shpList.at(index)->name();
}

int DataManagement::ShapeDocs::listSize() const
{
    return _private->_shpList.size();
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
void Graphics::GraphicAssistant::zoomToLayer(Shape const& layer)
{
    _private->_mapOrigin = Pair<double>(layer.bounds().xCenter(),
                                        layer.bounds().yCenter());

    _private->_displayOrigin = Pair<int>::computeCenter(_private->_paintingRect);

    Pair<double> mapRange(layer.bounds().xRange(),
                          layer.bounds().yRange());

    Pair<float> scaleXY(float(_private->_paintingRect.width()) / mapRange.x(),
                        float(_private->_paintingRect.height()) / mapRange.y());

    // Ensure the objects to be fully covered.
    _private->_scaleToDisplay = COVER*  scaleXY.smaller();
}

Bounds Graphics::GraphicAssistant::computeMapHitBounds() const
{
    Pair<double> cornerHitMin = displayToMapXY(Pair<int>(_private->_paintingRect.left(), _private->_paintingRect.bottom()));
    Pair<double> cornerHitMax = displayToMapXY(Pair<int>(_private->_paintingRect.right(), _private->_paintingRect.top()));
    return Bounds(cornerHitMin, cornerHitMax);
}

std::shared_ptr<Graphics::Shape const> DataManagement::ShapeDocs::findByName(std::string const& name) const
{
    for (auto item : _private->_shpList)
        if (name == item->name())
            return item;

    return nullptr; // Name not found.
}

void Graphics::GraphicAssistant::zoomAtCursor(QPoint const& mousePos, float scaleFactor)
{
    Pair<int> displayOrigin(mousePos);
    Pair<double> mapOrigin = displayToMapXY(displayOrigin);
    _private->_displayOrigin = displayOrigin;
    _private->_mapOrigin = mapOrigin;
    _private->_scaleToDisplay *= scaleFactor;
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
}

Bounds DataManagement::ShapeDocs::computeGlobalBounds() const
{
    if (isEmpty())
        return Bounds();

    auto itr = _private->_shpList.begin();
    double xMin = (*itr)->bounds().xMin();
    double yMin = (*itr)->bounds().yMin();
    double xMax = (*itr)->bounds().xMax();
    double yMax = (*itr)->bounds().yMax();

    if (_private->_shpList.size() > 1)
        for (itr = _private->_shpList.begin() + 1; itr < _private->_shpList.end(); ++itr)
        {
            Bounds localBounds = (*itr)->bounds();
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

void Graphics::GraphicAssistant::moveStart(QPoint const& startPos)
{
    Pair<int> displayOriginNew(startPos);
    Pair<double> mapOriginNew = displayToMapXY(displayOriginNew);

    _private->_displayOrigin = displayOriginNew;
    _private->_mapOrigin = mapOriginNew;
}

void Graphics::GraphicAssistant::moveProcessing(QPoint const& currentPos)
{
    _private->_displayOrigin = Pair<int>(currentPos);
}
