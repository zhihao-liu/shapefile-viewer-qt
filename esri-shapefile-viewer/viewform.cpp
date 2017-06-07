#include "viewform.h"
#include "ui_viewform.h"
#include <QPainter>
#include <QWheelEvent>
#include "shapemanager.h"

ViewForm::ViewForm(QWidget* parent)
    : QWidget(parent), ui(new Ui::ViewForm)
{
    ui->setupUi(this);

    setCursor(QCursor(Qt::CursorShape::OpenHandCursor));
}

ViewForm::~ViewForm() {}

void ViewForm::paintEvent(QPaintEvent*)
{    
    cl::DataManagement::ShapeView::instance().setPaintingRect(rect());

    QPainter painter(this);
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    cl::DataManagement::ShapeView::instance().draw(painter);

    painter.end();
}

void ViewForm::wheelEvent(QWheelEvent* event)
{
    cl::Pair<int> mousePos(event->pos());

    // Zoom in once everytime the wheel turns 90 degrees.
    float scaleFactor = 1 + (float(event->delta()) / 8 / 90);

    cl::DataManagement::ShapeView::instance().zoomAtCursor(mousePos, scaleFactor);
}

void ViewForm::mouseDoubleClickEvent(QMouseEvent*)
{
    cl::DataManagement::ShapeView::instance().zoomToAll();
}

void ViewForm::mousePressEvent(QMouseEvent* event)
{
    _mouseDragging = true;
    setCursor(QCursor(Qt::CursorShape::ClosedHandCursor));
    cl::DataManagement::ShapeView::instance().translationStart(event->pos());
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
        cl::DataManagement::ShapeView::instance().translationProcessing(event->pos());
    }
}
