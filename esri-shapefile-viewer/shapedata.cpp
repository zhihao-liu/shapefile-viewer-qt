#include "shapedata.h"
#include <QPoint>
#include <QPainter>
#include <QColor>
#include <QFileInfo>
#include "shapemanager.h"

using namespace cl;

namespace cl
{
class ShapePrivate
{
    friend class Shape;
    friend class Point;
    friend class Polyline;
    friend class Polygon;

public:
    ~ShapePrivate()
    {
        if (_shpTree)
            SHPDestroyTree(_shpTree);
        if (_shpHandle)
            SHPClose(_shpHandle);
    }

private:
    ShapePrivate(Shape& refThis, SHPHandle shpHandle, std::string name)
        : _refThis(refThis), _shpHandle(shpHandle), _name(name)
    {
        _bounds.set(_shpHandle->adBoundsMin, _shpHandle->adBoundsMax);

        _borderColor = QColor::fromHsl(qrand()%360, qrand()%256, qrand()%200);
        _fillColor = QColor::fromHsl(qrand()%360, qrand()%256, qrand()%256);

        _shpTree = SHPCreateTree(_shpHandle, 2, 10, nullptr, nullptr);
        SHPTreeTrimExtraNodes(_shpTree);
    }

    Shape& _refThis;

    SHPHandle _shpHandle;
    SHPTree* _shpTree = nullptr;
    std::string _name;
    Bounds _bounds;
    QColor _borderColor, _fillColor; // Each object has a different but fixed color set.
};
}

Shape::~Shape() {}

std::string Shape::name() const
{
    return _private->_name;
}

Shape::Shape(SHPHandle shpHandle, std::string name)
    : _private(std::unique_ptr<ShapePrivate>(new ShapePrivate(*this, shpHandle, name)))
{ }

std::unique_ptr<ShapeFactory> ShapeFactoryESRI::_instance = nullptr;

ShapeFactory const& ShapeFactoryESRI::instance()
{
    if (_instance == nullptr)
        _instance.reset(new ShapeFactoryESRI());
    return *_instance;
}

std::shared_ptr<Shape> ShapeFactoryESRI::createShape(std::string path) const
{
    SHPHandle shpHandle = SHPOpen(path.c_str(), "rb+");
    QFileInfo fileInfo(QString::fromStdString(path));
    std::string name = fileInfo.baseName().toStdString();

    switch (shpHandle->nShapeType)
    {
    case SHPT_POINT:
    case SHPT_POINTZ:
    case SHPT_POINTM:
        return std::shared_ptr<Shape>(new Point(shpHandle, name));
        break;

    case SHPT_ARC:
    case SHPT_ARCZ:
    case SHPT_ARCM:
        return std::shared_ptr<Shape>(new Polyline(shpHandle, name));
        break;

    case SHPT_POLYGON:
    case SHPT_POLYGONZ:
    case SHPT_POLYGONM:
        return std::shared_ptr<Shape>(new Polygon(shpHandle, name));
        break;
    default:
        return nullptr;
        break;
    }
}

int Point::draw(QPainter& painter, GraphicAssistant const& assistant) const
{
    Bounds mapHitBounds = assistant.computeMapHitBounds();


    double mapHitBoundsMin[2] = {mapHitBounds.xMin(), mapHitBounds.yMin()};
    double mapHitBoundsMax[2] = {mapHitBounds.xMax(), mapHitBounds.yMax()};

    int countRecordsHit;
    int* recordsHit = SHPTreeFindLikelyShapes(_private->_shpTree, mapHitBoundsMin, mapHitBoundsMax, &countRecordsHit);

    painter.setPen(QPen(_private->_borderColor));
    painter.setBrush(QBrush(_private->_fillColor));

    for (int i = 0; i < countRecordsHit; ++i)
    {
        SHPObject* record = SHPReadObject(_private->_shpHandle, recordsHit[i]);
        QPoint point = assistant.computePointOnDisplay(*record, 0);

        int const r = 5;

        painter.drawEllipse(point, r, r);

        SHPDestroyObject(record);
    }

    return countRecordsHit;
}

int Polyline::draw(QPainter& painter, GraphicAssistant const& assistant) const
{
    Bounds mapHitBounds = assistant.computeMapHitBounds();


    double mapHitBoundsMin[2] = {mapHitBounds.xMin(), mapHitBounds.yMin()};
    double mapHitBoundsMax[2] = {mapHitBounds.xMax(), mapHitBounds.yMax()};

    int countRecordsHit;
    int* recordsHit = SHPTreeFindLikelyShapes(_private->_shpTree, mapHitBoundsMin, mapHitBoundsMax, &countRecordsHit);

    painter.setPen(QPen(_private->_borderColor));
    painter.setBrush(QBrush(_private->_fillColor));

    for (int i = 0; i < countRecordsHit; ++i)
    {
        SHPObject* record = SHPReadObject(_private->_shpHandle, recordsHit[i]);
        record->panPartStart[record->nParts] = record->nVertices;

        for (int partIndex = 0; partIndex < record->nParts; ++partIndex)
        {
            int nPartVertices = record->panPartStart[partIndex+1] - record->panPartStart[partIndex];
            QPoint partVertices[nPartVertices];

            int count = 0;
            for (int vtxIndex = record->panPartStart[partIndex]; vtxIndex < record->panPartStart[partIndex+1]; ++vtxIndex)
                partVertices[count++] = assistant.computePointOnDisplay(*record, vtxIndex);

            painter.drawPolyline(partVertices, nPartVertices);
        }

        SHPDestroyObject(record);
    }

    return countRecordsHit;
}

int Polygon::draw(QPainter& painter, GraphicAssistant const& assistant) const
{
    Bounds mapHitBounds = assistant.computeMapHitBounds();


    double mapHitBoundsMin[2] = {mapHitBounds.xMin(), mapHitBounds.yMin()};
    double mapHitBoundsMax[2] = {mapHitBounds.xMax(), mapHitBounds.yMax()};

    int countRecordsHit;
    int* recordsHit = SHPTreeFindLikelyShapes(_private->_shpTree, mapHitBoundsMin, mapHitBoundsMax, &countRecordsHit);

    painter.setPen(QPen(_private->_borderColor));
    painter.setBrush(QBrush(_private->_fillColor));

    for (int i = 0; i < countRecordsHit; ++i)
    {
        SHPObject* record = SHPReadObject(_private->_shpHandle, recordsHit[i]);
        record->panPartStart[record->nParts] = record->nVertices;

        for (int partIndex = 0; partIndex < record->nParts; ++partIndex)
        {
            int nPartVertices = record->panPartStart[partIndex+1] - record->panPartStart[partIndex];
            QPoint partVertices[nPartVertices];

            int count = 0;
            for (int vtxIndex = record->panPartStart[partIndex]; vtxIndex < record->panPartStart[partIndex+1]; ++vtxIndex)
                partVertices[count++] = assistant.computePointOnDisplay(*record, vtxIndex);

            painter.drawPolygon(partVertices, nPartVertices);
        }

        SHPDestroyObject(record);
    }

    return countRecordsHit;
}

Bounds const& Shape::bounds() const
{
    return _private->_bounds;
}

SHPTree const& Shape::tree() const
{
    return *_private->_shpTree;
}

int Shape::countRecords() const
{
    return _private->_shpHandle->nRecords;
}
