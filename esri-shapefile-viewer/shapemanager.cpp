#include "shapemanager.h"
#include <QColor>
#include <QTime>
#include <QPoint>
#include "mainwindow.h"
#include "shapedata.h"

#define COVER 0.9
#define EPS 1E-4

using namespace cl;

class cl::Graphics::GraphicAssistant::Private
{
    friend class GraphicAssistant;

private:
    Private(GraphicAssistant& refThis, DataManagement::ShapeDoc const& refDoc)
        : _refThis(refThis), _refDoc(refDoc) {}

    GraphicAssistant& _refThis;

    DataManagement::ShapeDoc const& _refDoc;

    Pair<double> _mapOrigin;
    Pair<int> _displayOrigin;
    float _scaleToDisplay;
    Rect<int> _paintingRect;
};

QString DataManagement::ShapeDoc::drawAllLayers(QPainter& painter, Graphics::GraphicAssistant const& assistant) const
{
    //    if (isEmpty())
    //        return;

    int countRecordsHit = 0;
    int countRecordsTotal = 0;
    for (auto const& item : _layerList)
    {
        countRecordsHit += item->draw(painter, assistant);
        countRecordsTotal += item->recordCount();
    }

    float percentageHit = countRecordsHit / (countRecordsTotal + EPS);

    QString msgCountHit = "    Records Hit: " + QString::number(countRecordsHit);
    QString msgCountTotal = "    Records Total: " + QString::number(countRecordsTotal);
    QString msgPercentage = "    Percentage Hit: " + QString::number(percentageHit*  100, 'g', 4) + "%";

    return  msgCountHit + msgCountTotal + msgPercentage;
}

bool DataManagement::ShapeDoc::addLayer(std::string const& path)
{
    std::shared_ptr<Graphics::Shape> shp = ShapeFactoryEsri::instance().createShape(path);
    if (!shp)
        return false;

    _layerList.push_back(shp);

    return true;
}

void DataManagement::ShapeDoc::removeLayer(LayerIterator layerItr)
{
    _layerList.erase(layerItr);
}

void DataManagement::ShapeDoc::rearrangeLayer(LayerIterator fromItr, LayerIterator toItr)
{
    _layerList.insert(toItr, *fromItr);
    _layerList.erase(fromItr);
}

void DataManagement::ShapeDoc::clearAllLayers()
{
    _layerList.clear();
}

std::unique_ptr<DataManagement::ShapeView> DataManagement::ShapeView::_instance = nullptr;

DataManagement::ShapeView& DataManagement::ShapeView::instance()
{
    if (_instance == nullptr)
        _instance.reset(new DataManagement::ShapeView());
    return *_instance;
}

bool DataManagement::ShapeDoc::isEmpty() const
{
    return _layerList.empty() ? true : false;
}

int DataManagement::ShapeDoc::layerCount() const
{
    return _layerList.size();
}

Graphics::GraphicAssistant::GraphicAssistant(DataManagement::ShapeDoc const& refDoc)
    : _private(std::unique_ptr<Private>
               (new Private(*this, refDoc))) {}

void Graphics::GraphicAssistant::setPaintingRect(Rect<int> const& paintingRect)
{
    _private->_paintingRect = paintingRect;
}

Pair<int> Graphics::GraphicAssistant::computePointOnDisplay(SHPObject const& record, int ptIndex) const
{
    Pair<double> mapXY(record.padfX[ptIndex], record.padfY[ptIndex]);
    Pair<int> displayXY = mapToDisplayXY(mapXY);
    return displayXY;
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
void Graphics::GraphicAssistant::zoomToLayer(LayerIterator layerItr)
{
    _private->_mapOrigin = (*layerItr)->bounds().center();

    _private->_displayOrigin = _private->_paintingRect.center();

    Pair<float> scaleXY(Pair<float>(_private->_paintingRect.range()) / (*layerItr)->bounds().range());

    // Ensure the objects to be fully covered.
    _private->_scaleToDisplay = COVER * scaleXY.smaller();
}

Rect<double> Graphics::GraphicAssistant::computeMapHitBounds() const
{
    Pair<double> cornerHitMin = displayToMapXY(Pair<int>(_private->_paintingRect.xMin(), _private->_paintingRect.yMax()));
    Pair<double> cornerHitMax = displayToMapXY(Pair<int>(_private->_paintingRect.xMax(), _private->_paintingRect.yMin()));
    return Rect<double>(cornerHitMin, cornerHitMax);
}

LayerIterator DataManagement::ShapeDoc::findByName(std::string const& name)
{
    auto itr = _layerList.begin();
    while (itr != _layerList.end())
    {
        if (name == (*itr)->name())
            break;
        ++itr;
    }

    return itr; // If the name is not found, std::list.end() will be returned.
}

bool DataManagement::ShapeDoc::layerNotFound(LayerIterator itr) const
{
    return itr == _layerList.end() ? true : false;
}

void Graphics::GraphicAssistant::zoomAtCursor(Pair<int> const& mousePos, float scaleFactor)
{
    Pair<int> displayOrigin(mousePos);
    Pair<double> mapOrigin = displayToMapXY(displayOrigin);
    _private->_displayOrigin = displayOrigin;
    _private->_mapOrigin = mapOrigin;
    _private->_scaleToDisplay *= scaleFactor;
}

void Graphics::GraphicAssistant::zoomToAll()
{
    Rect<double> globalBounds = _private->_refDoc.computeGlobalBounds();

    _private->_mapOrigin = globalBounds.center();

    _private->_displayOrigin = _private->_paintingRect.center();

    Pair<float> scaleXY(Pair<float>(_private->_paintingRect.range()) / globalBounds.range());

    _private->_scaleToDisplay = COVER * scaleXY.smaller();
}

Rect<double> DataManagement::ShapeDoc::computeGlobalBounds() const
{
    if (isEmpty())
        return Rect<double>();

    double xMin = _layerList.front()->bounds().xMin();
    double yMin = _layerList.front()->bounds().yMin();
    double xMax = _layerList.front()->bounds().xMax();
    double yMax = _layerList.front()->bounds().yMax();

    if (_layerList.size() > 1)
        for (auto const& item : _layerList)
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

void Graphics::GraphicAssistant::translationStart(Pair<int> const& startPos)
{
    Pair<int> displayOriginNew(startPos);
    Pair<double> mapOriginNew = displayToMapXY(displayOriginNew);

    _private->_displayOrigin = displayOriginNew;
    _private->_mapOrigin = mapOriginNew;
}

void Graphics::GraphicAssistant::translationProcessing(Pair<int> const& currentPos)
{
    _private->_displayOrigin = Pair<int>(currentPos);
}

std::vector<std::string const*> DataManagement::ShapeDoc::rawNameList() const
{
    std::vector<std::string const*> rawNameList;

    // Painting is in the reverse order of the list,
    // thus we have to get a reverse list for display.
    for (auto itr = _layerList.rbegin(); itr != _layerList.rend(); ++itr)
        rawNameList.push_back(&(*itr)->name());

    return rawNameList;
}

void DataManagement::ShapeView::draw(QPainter& painter)
{
    QString recordStat = _shapeDoc.drawAllLayers(painter, _assistant);
    dynamic_cast<ShapeViewObserver*>(_rawObserver)->setLabel(recordStat);
}

DataManagement::DisplayManager::DisplayManager()
    : _shapeDoc(), _assistant(_shapeDoc), _rawObserver(nullptr) {}

Rect<int> const& Graphics::GraphicAssistant::paintingRect() const
{
    return _private->_paintingRect;
}

float Graphics::GraphicAssistant::scale() const
{
    return _private->_scaleToDisplay;
}

// Defined here to ensure the unique pointer of Private to be destructed properly.
Graphics::GraphicAssistant::~GraphicAssistant() {}

DataManagement::ShapeDoc DataManagement::ShapeDoc::clone() const
{
    ShapeDoc docCopy;

    for (auto item : _layerList)
        docCopy._layerList.push_back(item->clone());

    return docCopy;
}
