#include "map.h"
#include <QPainter>
#include <QPen>

using namespace cl;

void Map::Map::draw(QPainter& painter)
{
    _shapeDoc.drawAllLayers(painter, _assistant);

    for (auto const& item : _elements)
        item->draw(painter, _assistant);

    refresh();
}

void Map::GridLine::draw(QPainter& painter, Graphics::GraphicAssistant const& assistant)
{
    QPen penPreserved = painter.pen();
    QBrush brushPreserved = painter.brush();
    painter.setPen(_lineStyle);
    painter.setBrush(Qt::NoBrush);

    Rect<int> const& rect = assistant.paintingRect();

    int gap = rect.yRange() / 15;

    for(int i = 0; i < rect.yRange() / gap - 1; ++i)
        for(int j = 0; j < rect.xRange() / gap - 1; ++j)
            painter.drawRect(QRect(QPoint(gap * j, gap * i), QSize(gap, gap)));

    painter.setPen(penPreserved);
    painter.setBrush(brushPreserved);
}

void Map::NorthPointer::Flat::draw(QPainter& painter, Graphics::GraphicAssistant const& assistant)
{
    QPen penPreserved = painter.pen();

    QPen pen(Qt::black);
    pen.setWidth(3);
    painter.setPen(pen);

    Rect<int> const& rect = assistant.paintingRect();

    painter.drawLine(QPoint(rect.xRange() - 20, 10), QPoint(rect.xRange() - 25, 15));
    painter.drawLine(QPoint(rect.xRange() - 20, 10), QPoint(rect.xRange() - 15, 15));
    painter.drawLine(QPoint(rect.xRange() - 20, 10), QPoint(rect.xRange() - 20, 30));

    painter.setPen(penPreserved);
}

void Map::NorthPointer::Style3D::draw(QPainter& painter, Graphics::GraphicAssistant const& assistant)
{
    QBrush brushPreserved = painter.brush();

    painter.setBrush(QBrush(Qt::black, Qt::SolidPattern));

    Rect<int> const& rect = assistant.paintingRect();

    QPoint points[3];
    points[0] = QPoint(rect.xRange() - 20, 10);
    points[1] = QPoint(rect.xRange() - 20, 15);
    points[2] = QPoint(rect.xRange() - 25, 30);
    painter.drawPolygon(points, 3);
    points[2] = QPoint(rect.xRange() - 15, 30);
    painter.drawPolygon(points, 3);

    painter.setBrush(brushPreserved);
}

void Map::ScaleBar::Text::draw(QPainter &painter, const Graphics::GraphicAssistant &assistant)
{
    Rect<int> const& rect = assistant.paintingRect();
    float scale = assistant.scale();

    QRect textCover(QPoint(rect.xRange() - 50, rect.yRange() - 25), QSize(50, 25));
    QString text = "1 : " + QString::number(1.0 / scale, 'g', 2);

    painter.drawText(textCover, text);
}

void Map::ScaleBar::Graphic::draw(QPainter &painter, const Graphics::GraphicAssistant &assistant)
{
    QBrush brushPreserved = painter.brush();

    Rect<int> const& rect = assistant.paintingRect();

    painter.setBrush(QBrush(Qt::red, Qt::SolidPattern));
    painter.drawRect(QRect(QPoint(rect.xRange() - 30, rect.yRange() - 10), QSize(10, 5)));
    painter.setBrush(QBrush(Qt::green, Qt::SolidPattern));
    painter.drawRect(QRect(QPoint(rect.xRange() - 20, rect.yRange() - 10), QSize(10,5)));

    painter.setBrush(brushPreserved);
}
