#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QTime>
#include <QLabel>
#include <QListWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Shape Viewer");

    qsrand(QTime::currentTime().second());

    // Initialize the view widget of shapes.
    _viewForm.reset(new ViewForm(this));
    setCentralWidget(_viewForm.get());

    // Initialize the sidebar widget.
    _sidebar.reset(new Sidebar(this));
    addDockWidget(Qt::LeftDockWidgetArea, _sidebar.get(), Qt::Vertical);

    // Initialize the status label.
    _msgLabel.reset(new QLabel());
    statusBar()->setStyleSheet(QString("QStatusBar::item{border: 0px}"));
    statusBar()->addWidget(_msgLabel.get());

    // Bind the singleton dataset with this form as its observer.
    cl::DataManagement::ShapeView::instance().setObserver(*this);

    // Connect the open file signal.
    connect(ui->actionOpen_Dataset, SIGNAL(triggered(bool)), this, SLOT(openDataset()));
    connect(ui->actionClose_All, SIGNAL(triggered(bool)), this, SLOT(closeAll()));
    connect(ui->actionRemove_Layer, SIGNAL(triggered(bool)), this, SLOT(removeLayer()));
    connect(ui->actionLayer_Up, SIGNAL(triggered(bool)), this, SLOT(layerUp()));
    connect(ui->actionLayer_Down, SIGNAL(triggered(bool)), this, SLOT(layerDown()));
    connect(ui->actionFull_Elements, SIGNAL(triggered(bool)), this, SLOT(createMapFullElements()));
    connect(ui->actionNo_Grid_Line, SIGNAL(triggered(bool)), this, SLOT(createMapNoGridLine()));
    // If the slot function name is wrong,
    // without any error prompts the connection will not work.

    // Show the main window.
    show();
}

MainWindow::~MainWindow() {}

void MainWindow::updateDisplay()
{
    _viewForm->update();
    _sidebar->updateList();
    update();
}

void MainWindow::setLabel(QString const& msg)
{
    if (_msgLabel != nullptr)
        _msgLabel->setText(msg);
}

void MainWindow::openDataset()
{
    using namespace cl::DataManagement;

    QFileDialog dialog(this, tr("Open ESRI Shape File:"), "", tr("*.shp"));
    dialog.setFileMode(QFileDialog::ExistingFiles); // Accept multiple selections.
    dialog.setDirectory("/users/liuzhihao/workstation/programs/esri-shapefile-viewer/sample data");

    if (!dialog.exec())
        return;

    bool notInitialized = ShapeView::instance().isEmpty();

    QStringList fileNames = dialog.selectedFiles();
    for (auto path : fileNames)
        ShapeView::instance().addLayer(path.toStdString());

    if (notInitialized)
        ShapeView::instance().zoomToAll();
}

void MainWindow::closeAll()
{
    cl::DataManagement::ShapeView::instance().clearAllLayers();
}

void MainWindow::removeLayer()
{
    using namespace cl::DataManagement;

    QList<QListWidgetItem*> selection = _sidebar->listSelection();
    if (selection.empty())
        return;

    auto layerItr = ShapeView::instance().findByName(selection.front()->text().toStdString());
    if (ShapeView::instance().layerNotFound(layerItr))
        return;

    ShapeView::instance().removeLayer(layerItr);
}

void MainWindow::layerUp()
{
    using namespace cl::DataManagement;

    QList<QListWidgetItem*> selection = _sidebar->listSelection();
    if (selection.empty())
        return;
    QListWidgetItem* selectedItem = selection.front();
    if (selectedItem == _sidebar->listFirst())
        return;

    auto layerItr = ShapeView::instance().findByName(selectedItem->text().toStdString());
    if (ShapeView::instance().layerNotFound(layerItr))
        return;

    ShapeView::instance().rearrangeLayer(layerItr, ++(++layerItr));
}

void MainWindow::layerDown()
{
    using namespace cl::DataManagement;

    QList<QListWidgetItem*> selection = _sidebar->listSelection();
    if (selection.empty())
        return;
    QListWidgetItem* selectedItem = selection.front();
    if (selectedItem == _sidebar->listLast())
        return;

    auto layerItr = ShapeView::instance().findByName(selectedItem->text().toStdString());
    if (ShapeView::instance().layerNotFound(layerItr))
        return;

    ShapeView::instance().rearrangeLayer(layerItr, --layerItr);
}

void MainWindow::createMap(cl::Map::MapStyle mapStyle)
{
    using namespace cl::Map;
    using namespace cl::DataManagement;

    _mapWindow.reset(new MapWindow(this));
    _mapWindow->show();

    std::unique_ptr<MapDirector> mapDirector;

    switch (mapStyle)
    {
    case MapStyle::FullElements:
        mapDirector.reset(new MapDirector(new MapBuilder::FullElements()));
        break;
    case MapStyle::NoGridLine:
        mapDirector.reset(new MapDirector(new MapBuilder::NoGridLine()));
        break;
    default:
        return;
    }

    std::shared_ptr<Map> map = mapDirector->constructMap(ShapeView::instance().shapeDoc());

    _mapWindow->setMap(map);
}

void MainWindow::createMapFullElements()
{
    createMap(cl::Map::MapStyle::FullElements);
}

void MainWindow::createMapNoGridLine()
{
    createMap(cl::Map::MapStyle::NoGridLine);
}
