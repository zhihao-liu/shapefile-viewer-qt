#include "shapemanager.h"
#include <QColor>
#include <QTime>
#include <QPoint>
#include "mainwindow.h"
#include "shapedata.h"

#define COVER 0.9
#define EPS 1e-4

using namespace cl;

namespace cl
{
class ShapeDocsPrivate
{
    friend class ShapeDocs;

private:
    ShapeDocsPrivate(ShapeDocs& refThis)
        : _refThis(refThis), _assistant(refThis), _rawObserver(nullptr) {}

    ShapeDocs& _refThis;

    std::vector<std::shared_ptr<Shape>> _shpList;
    GraphicAssistant _assistant;
    ShapeDocsObserver* _rawObserver;
};

class GraphicAssistantPrivate
{
    friend class GraphicAssistant;

private:
    GraphicAssistantPrivate(GraphicAssistant& refThis, ShapeDocs const& refDocs)
        : _refThis(refThis), _refDocs(refDocs) {}

    GraphicAssistant& _refThis;

    ShapeDocs const& _refDocs;
    Pair<double> _mapOrigin;
    Pair<int> _displayOrigin;
    float _scaleToDisplay;
    QRect _paintingRect;
};
}

ShapeDocs::ShapeDocs()
    : _private(std::unique_ptr<ShapeDocsPrivate>
               (new ShapeDocsPrivate(*this))) {}

void ShapeDocs::setObserver(ShapeDocsObserver& observer)
{
    _private->_rawObserver =& observer;
}

void ShapeDocs::drawAllShapes(QPainter& painter) const
{
    //    if (isEmpty())
    //        return;

    // Refresh the QRect of the painting region on screen.
    _private->_assistant.setPainterRect(_private->_rawObserver->paintingRect());

    int countRecordsHit = 0;
    int countRecordsTotal = 0;
    for (std::vector<std::shared_ptr<Shape>>::iterator itr = _private->_shpList.begin();
         itr < _private->_shpList.end(); ++itr)
    {
        countRecordsHit += (*itr)->draw(painter, _private->_assistant);
        countRecordsTotal += (*itr)->recordCount();
    }

    float percentageHit = countRecordsHit / (countRecordsTotal + EPS);

    QString msgPercentage = QString::number(percentageHit*  100, 'g', 4);
    QString msgCountHit = QString::number(countRecordsHit);
    QString msgCountTotal = QString::number(countRecordsTotal);

    _private->_rawObserver->setLabel("\t\t Records Hit: " + msgCountHit +
                                       "\t\t Records Total: " + msgCountTotal +
                                       "\t\t Percentage Hit: " + msgPercentage +"%");
}

bool ShapeDocs::addShape(std::string const& path)
{
    std::shared_ptr<Shape> shp = ShapeFactoryESRI::instance().createShape(path);
    if (!shp)
        return false;

    _private->_shpList.push_back(shp);
    if (_private->_shpList.size() == 1)
        _private->_assistant.zoomToLayer(*shp);
    refresh();
    return true;
}

void ShapeDocs::removeShape(std::string const& name)
{
    for (std::vector<std::shared_ptr<Shape>>::iterator itr = _private->_shpList.begin();
         itr < _private->_shpList.end(); ++itr)
    {
        std::string shpName = (*itr)->name();
        if (shpName == name)
        {
            _private->_shpList.erase(itr);
            break;
        }
    }
    refresh();
}

void ShapeDocs::refresh() const
{
    _private->_rawObserver->updateDisplay();
}

void ShapeDocs::clear()
{
    _private->_shpList.clear();
}

std::unique_ptr<ShapeDocs> ShapeManager::_data = nullptr;

ShapeDocs& ShapeManager::data()
{
    if (_data == nullptr)
        _data.reset(new ShapeDocs());
    return *_data;
}

bool ShapeDocs::isEmpty() const
{
    return _private->_shpList.size() == 0 ? true : false;
}

std::string const& ShapeDocs::nameOf(int index) const
{
    return _private->_shpList.at(index)->name();
}

int ShapeDocs::listSize() const
{
    return _private->_shpList.size();
}

GraphicAssistant& ShapeDocs::assistant() const
{
    return _private->_assistant;
}

GraphicAssistant::GraphicAssistant(ShapeDocs const& refDocs)
    : _private(std::unique_ptr<GraphicAssistantPrivate>
               (new GraphicAssistantPrivate(*this, refDocs))) {}

void GraphicAssistant::setPainterRect(QRect const& paintingRect)
{
    _private->_paintingRect = paintingRect;
}

QPoint GraphicAssistant::computePointOnDisplay(SHPObject const& record, int ptIndex) const
{
    Pair<double> mapXY(record.padfX[ptIndex], record.padfY[ptIndex]);
    Pair<int> displayXY = mapToDisplayXY(mapXY);
    return displayXY.toPoint();
}

Pair<int> GraphicAssistant::mapToDisplayXY(Pair<double> mapXY) const
{
    return (mapXY - _private->_mapOrigin)*  Pair<double>(1, -1)*  _private->_scaleToDisplay + Pair<double>(_private->_displayOrigin);
}

Pair<double> GraphicAssistant::displayToMapXY(Pair<int> displayXY) const
{
    return Pair<double>(displayXY - _private->_displayOrigin) / _private->_scaleToDisplay / Pair<double>(1, -1) + _private->_mapOrigin;
}

// make a specified layer fully displayed and centered
void GraphicAssistant::zoomToLayer(Shape const& layer)
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

Bounds GraphicAssistant::computeMapHitBounds() const
{
    Pair<double> cornerHitMin = displayToMapXY(Pair<int>(_private->_paintingRect.left(), _private->_paintingRect.bottom()));
    Pair<double> cornerHitMax = displayToMapXY(Pair<int>(_private->_paintingRect.right(), _private->_paintingRect.top()));
    return Bounds(cornerHitMin, cornerHitMax);
}

std::shared_ptr<Shape const> ShapeDocs::findByName(std::string const& name) const
{
    for (std::vector<std::shared_ptr<Shape>>::iterator itr = _private->_shpList.begin();
         itr < _private->_shpList.end(); ++itr)
        if (name == (*itr)->name())
            return *itr;

    return nullptr; // Name not found.
}

void GraphicAssistant::zoomAtCursor(QPoint const& mousePos, float scaleFactor)
{
    Pair<int> displayOrigin(mousePos);
    Pair<double> mapOrigin = displayToMapXY(displayOrigin);
    _private->_displayOrigin = displayOrigin;
    _private->_mapOrigin = mapOrigin;
    _private->_scaleToDisplay *= scaleFactor;
}

void GraphicAssistant::zoomToAll()
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

Bounds ShapeDocs::computeGlobalBounds() const
{
    if (isEmpty())
        return Bounds();

    std::vector<std::shared_ptr<Shape>>::iterator itr = _private->_shpList.begin();
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

void GraphicAssistant::moveStart(QPoint const& startPos)
{
    Pair<int> displayOriginNew(startPos);
    Pair<double> mapOriginNew = displayToMapXY(displayOriginNew);

    _private->_displayOrigin = displayOriginNew;
    _private->_mapOrigin = mapOriginNew;
}

void GraphicAssistant::moveProcessing(QPoint const& currentPos)
{
    _private->_displayOrigin = Pair<int>(currentPos);
}
