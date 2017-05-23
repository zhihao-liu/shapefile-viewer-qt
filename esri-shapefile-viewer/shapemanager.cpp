#include "shapemanager.h"
#include <QColor>
#include <QTime>
#include <QPoint>
#include "mainwindow.h"
#include "shapedata.h"

#define COVER 0.9
#define MINI 1e-4

using namespace cl;

namespace cl
{
class ShapeDocsPrivate
{
    friend class ShapeDocs;

private:
    ShapeDocsPrivate(ShapeDocs& refThis, MainWindow& observer)
        : _refThis(refThis), _observer(observer), _assistant(refThis) {}

    ShapeDocs& _refThis;

    std::vector<std::shared_ptr<Shape>> _shpList;
    MainWindow& _observer;
    GraphicAssistant _assistant;
};
}

ShapeDocs::ShapeDocs(MainWindow& observer)
    : _private(std::unique_ptr<ShapeDocsPrivate>
               (new ShapeDocsPrivate(*this, observer))) {}

void ShapeDocs::drawAllShapes(QPainter& painter) const
{
//    // painting stops working after adding this?
//    if (isEmpty())
//        return;

    // refresh the QRect of the painting region on screen
    _private->_assistant.setPainterRect(_private->_observer.viewRect());

    int countRecordsHit = 0;
    int countRecordsTotal = 0;

    std::vector<std::shared_ptr<Shape>>::iterator itr;
    for (itr = _private->_shpList.begin(); itr < _private->_shpList.end(); ++itr)
    {
        countRecordsHit += (*itr)->draw(painter, _private->_assistant);
        countRecordsTotal += (*itr)->countRecords();
    }

    float percentageHit = countRecordsHit / (countRecordsTotal + MINI);

    QString msgPercentage = QString::number(percentageHit * 100, 'g', 4);
    QString msgCountHit = QString::number(countRecordsHit);
    QString msgCountTotal = QString::number(countRecordsTotal);

    _private->_observer.setStatusLabel("\t\t Records Hit: " + msgCountHit +
                                       "\t\t Records Total: " + msgCountTotal +
                                       "\t\t Percentage Hit: " + msgPercentage +"%");
}

bool ShapeDocs::addShape(std::string path)
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

void ShapeDocs::removeShape(std::string Name)
{
    std::vector<std::shared_ptr<Shape>>::iterator itr;
    for (itr = _private->_shpList.begin(); itr < _private->_shpList.end(); ++itr)
    {
        std::string shpName = (*itr)->name();
        if (shpName == Name)
        {
            _private->_shpList.erase(itr);
            break;
        }
    }
    refresh();
}

void ShapeDocs::refresh() const
{
    _private->_observer.updateWidgets();
}

void ShapeDocs::clear()
{
    _private->_shpList.clear();
}

std::unique_ptr<ShapeDocs> ShapeManager::_data = nullptr;

void ShapeManager::setData(MainWindow& observer)
{
    _data.reset(new ShapeDocs(observer));
}

ShapeDocs& ShapeManager::data()
{
    return *_data;
}

bool ShapeDocs::isEmpty() const
{
    return _private->_shpList.size() == 0 ? true : false;
}

std::string ShapeDocs::nameOf(int index) const
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
    : _refDocs(refDocs) {}

void GraphicAssistant::setPainterRect(QRect const& painterRect)
{
    _painterRect = painterRect;
}

QPoint GraphicAssistant::computePointOnDisplay(SHPObject const& record, int ptIndex) const
{
    Pair<double> mapXY(record.padfX[ptIndex], record.padfY[ptIndex]);
    Pair<int> displayXY = mapToDisplayXY(mapXY);
    return displayXY.toPoint();
}

Pair<int> GraphicAssistant::mapToDisplayXY(Pair<double> mapXY) const
{
    return (mapXY - _mapOrigin) * Pair<double>(1, -1) * _scaleToDisplay + Pair<double>(_displayOrigin);
}

Pair<double> GraphicAssistant::displayToMapXY(Pair<int> displayXY) const
{
    return Pair<double>(displayXY - _displayOrigin) / _scaleToDisplay / Pair<double>(1, -1) + _mapOrigin;
}

// make a specified layer fully displayed and centered
void GraphicAssistant::zoomToLayer(Shape const& layer)
{
    _mapOrigin = Pair<double>(layer.bounds().xCenter(),
                              layer.bounds().yCenter());

    _displayOrigin = Pair<int>::computeCenter(_painterRect);

    Pair<double> mapRange(layer.bounds().xRange(),
                          layer.bounds().yRange());

    Pair<float> scaleXY(float(_painterRect.width()) / mapRange.x(),
                        float(_painterRect.height()) / mapRange.y());

    // ensure the objects to be fully covered
    _scaleToDisplay = COVER * scaleXY.smaller();
}

Bounds GraphicAssistant::computeMapHitBounds() const
{
    Pair<double> cornerHitMin = displayToMapXY(Pair<int>(_painterRect.left(), _painterRect.bottom()));
    Pair<double> cornerHitMax = displayToMapXY(Pair<int>(_painterRect.right(), _painterRect.top()));
    return Bounds(cornerHitMin, cornerHitMax);
}

std::shared_ptr<Shape const> ShapeDocs::findByName(std::string name) const
{
    std::vector<std::shared_ptr<Shape>>::iterator itr;
    for (itr = _private->_shpList.begin(); itr < _private->_shpList.end(); ++itr)
        if (name == (*itr)->name())
            return *itr;

    return nullptr; // name not found
}

void GraphicAssistant::zoomAtCursor(QPoint const& mousePos, float scaleFactor)
{
    Pair<int> displayOrigin(mousePos);
    Pair<double> mapOrigin = displayToMapXY(displayOrigin);
    _displayOrigin = displayOrigin;
    _mapOrigin = mapOrigin;
    _scaleToDisplay *= scaleFactor;
}

void GraphicAssistant::zoomToAll()
{
    Bounds globalBounds = _refDocs.computeGlobalBounds();

    _mapOrigin = Pair<double>(globalBounds.xCenter(),
                              globalBounds.yCenter());

    _displayOrigin = Pair<int>::computeCenter(_painterRect);

    Pair<double> mapRange(globalBounds.xRange(),
                          globalBounds.yRange());

    Pair<float> scaleXY(float(_painterRect.width()) / mapRange.x(),
                        float(_painterRect.height()) / mapRange.y());

    _scaleToDisplay = COVER * scaleXY.smaller();
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

    _displayOrigin = displayOriginNew;
    _mapOrigin = mapOriginNew;
}

void GraphicAssistant::moveProcessing(QPoint const& currentPos)
{
    _displayOrigin = Pair<int>(currentPos);
}
