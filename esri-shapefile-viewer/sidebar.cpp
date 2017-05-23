#include "sidebar.h"
#include "ui_sidebar.h"
#include <QListWidgetItem>
#include "shapemanager.h"
#include "shapedata.h"

Sidebar::Sidebar(QWidget *parent)
    : QDockWidget(parent), ui(new Ui::Sidebar)
{
    ui->setupUi(this);

    // connect the item double-click signal
    connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(doubleClickItem(QListWidgetItem*)));
}

Sidebar::~Sidebar() {}

void Sidebar::updateListView()
{
    ui->listWidget->clear();

    for (int i = 0; i < int(cl::ShapeManager::data().listSize()); ++i)
    {
        QString name = QString::fromStdString(cl::ShapeManager::data().nameOf(i));

        ui->listWidget->insertItem(i, name);
    }
}

// zoom to specified layer when double clicking it
void Sidebar::doubleClickItem(QListWidgetItem* itemClicked)
{
    QString layerName = itemClicked->text();
    std::shared_ptr<cl::Shape const> layer = cl::ShapeManager::data().findByName(layerName.toStdString());
    if (layer == nullptr)
        return;

    cl::ShapeManager::data().assistant().zoomToLayer(*layer);
    cl::ShapeManager::data().refresh();
}
