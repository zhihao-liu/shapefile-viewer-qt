#include "sidebar.h"
#include "ui_sidebar.h"
#include <QListWidgetItem>
#include "shapemanager.h"
#include "shapedata.h"

Sidebar::Sidebar(QWidget* parent)
    : QDockWidget(parent), ui(new Ui::Sidebar)
{
    ui->setupUi(this);

    // Connect the item double-click signal.
    connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(doubleClickItem(QListWidgetItem*)));
}

Sidebar::~Sidebar() {}

void Sidebar::updateList()
{
    ui->listWidget->clear();

    for (int i = 0; i < int(cl::DataManagement::ShapeManager::data().listSize()); ++i)
    {
        QString name = QString::fromStdString(cl::DataManagement::ShapeManager::data().nameOf(i));

        ui->listWidget->insertItem(i, name);
    }
}

// Zoom to specified layer when double clicking it.
void Sidebar::doubleClickItem(QListWidgetItem* itemClicked)
{
    QString layerName = itemClicked->text();
    std::shared_ptr<cl::Graphics::Shape const> layer =
            cl::DataManagement::ShapeManager::data().findByName(layerName.toStdString());
    if (layer == nullptr)
        return;

    cl::DataManagement::ShapeManager::data().assistant().zoomToLayer(*layer);
    cl::DataManagement::ShapeManager::data().refresh();
}
