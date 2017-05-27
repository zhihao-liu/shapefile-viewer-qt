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

    setCursor(QCursor(Qt::CursorShape::OpenHandCursor));
}

ViewForm::~ViewForm() {}

void ViewForm::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    cl::DataManagement::ShapeManager::data().paintAllLayers(painter);

    painter.end();

}

void ViewForm::wheelEvent(QWheelEvent* event)
{
    QPoint mousePos = event->pos();

    // Zoom in once everytime the wheel turns 90 degrees.
    float scaleFactor = 1 + (float(event->delta()) / 8 / 90);

    cl::DataManagement::ShapeManager::data().assistant().zoomAtCursor(mousePos, scaleFactor);
}

void ViewForm::mouseDoubleClickEvent(QMouseEvent*)
{
    cl::DataManagement::ShapeManager::data().assistant().zoomToAll();
}

void ViewForm::mousePressEvent(QMouseEvent* event)
{
    _mouseDragging = true;
    setCursor(QCursor(Qt::CursorShape::ClosedHandCursor));
    cl::DataManagement::ShapeManager::data().assistant().translationStart(event->pos());
}

void ViewForm::mouseReleaseEvent(QMouseEvent*)
{
    _mouseDragging = false;
    setCursor(QCursor(Qt::CursorShape::OpenHandCursor));
}

void ViewForm::mouseMoveEvent(QMouseEvent* event)
{
    if (_mouseDragging)
    {
        cl::DataManagement::ShapeManager::data().assistant().translationProcessing(event->pos());
    }
}
