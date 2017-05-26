#include "viewform.h"
#include "ui_viewform.h"
#include <QPainter>
#include <QWheelEvent>
#include "shapemanager.h"

using namespace cl;

ViewForm::ViewForm(QWidget* parent)
    : QWidget(parent), ui(new Ui::ViewForm)
{
    ui->setupUi(this);
}

ViewForm::~ViewForm() {}

void ViewForm::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    cl::ShapeManager::data().drawAllShapes(painter);

    painter.end();

}

void ViewForm::wheelEvent(QWheelEvent* event)
{
    QPoint mousePos = event->pos();

    // Zoom in once everytime the wheel turns 90 degrees.
    float scaleFactor = 1 + (float(event->delta()) / 8 / 90);

    cl::ShapeManager::data().assistant().zoomAtCursor(mousePos, scaleFactor);
    cl::ShapeManager::data().refresh();
}

void ViewForm::mouseDoubleClickEvent(QMouseEvent*)
{
    cl::ShapeManager::data().assistant().zoomToAll();
    cl::ShapeManager::data().refresh();
}

void ViewForm::mousePressEvent(QMouseEvent* event)
{
    _mouseDragging = true;
    cl::ShapeManager::data().assistant().moveStart(event->pos());
}

void ViewForm::mouseReleaseEvent(QMouseEvent*)
{
    _mouseDragging = false;
}

void ViewForm::mouseMoveEvent(QMouseEvent* event)
{
    if (_mouseDragging)
    {
        cl::ShapeManager::data().assistant().moveProcessing(event->pos());
        cl::ShapeManager::data().refresh();
    }
}
