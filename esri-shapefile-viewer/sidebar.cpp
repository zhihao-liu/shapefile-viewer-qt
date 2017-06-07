#include "sidebar.h"
#include "ui_sidebar.h"
#include <QListWidget>
#include <QListWidgetItem>
#include <QMouseEvent>
#include "shapemanager.h"
#include "shapedata.h"

Sidebar::Sidebar(QWidget* parent)
    : QDockWidget(parent), ui(new Ui::Sidebar)
{
    ui->setupUi(this);

    setCursor(QCursor(Qt::CursorShape::PointingHandCursor));

    // Connect the item double-click signal.
    connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(doubleClickItem(QListWidgetItem*)));
}

Sidebar::~Sidebar() {}

QListWidget const& Sidebar::listWidget()
{
    return *ui->listWidget;
}

void Sidebar::updateList()
{
    ui->listWidget->clear();

    auto rawNameList = cl::DataManagement::ShapeView::instance().rawNameList();

    int i = 0;
    for (auto item : rawNameList)
    {
        QString name = QString::fromStdString(*item);
        ui->listWidget->insertItem(i++, name);
    }
}

// Zoom to specified layer when double clicking it.
void Sidebar::doubleClickItem(QListWidgetItem* clickedItem)
{
    using namespace cl::DataManagement;

    QString layerName = clickedItem->text();
    auto layerItr = ShapeView::instance().findByName(layerName.toStdString());
    if (ShapeView::instance().layerNotFound(layerItr))
        return;

    ShapeView::instance().zoomToLayer(layerItr);
}
