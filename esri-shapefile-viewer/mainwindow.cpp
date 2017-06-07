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
    cl::DataManagement::ShapeManager::data().setObserver(*this);

    // Connect the open file signal.
    connect(ui->actionOpen_Dataset, SIGNAL(triggered(bool)), this, SLOT(openDataset()));
    connect(ui->actionClose_All, SIGNAL(triggered(bool)), this, SLOT(closeAll()));
    connect(ui->actionRemove_Layer, SIGNAL(triggered(bool)), this, SLOT(removeLayer()));
    connect(ui->actionLayer_Up, SIGNAL(triggered(bool)), this, SLOT(layerUp()));
    connect(ui->actionLayer_Down, SIGNAL(triggered(bool)), this, SLOT(layerDown()));
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

cl::Rect<int> const MainWindow::paintingRect() const
{
    return cl::Rect<int>(_viewForm->rect());
}

void MainWindow::setLabel(QString const& msg)
{
    if (_msgLabel != nullptr)
        _msgLabel->setText(msg);
}

void MainWindow::openDataset()
{
    QFileDialog dialog(this, tr("Open ESRI Shape File:"), "", tr("*.shp"));
    dialog.setFileMode(QFileDialog::ExistingFiles); // Accept multiple selections.

    if (!dialog.exec())
        return;

    bool notInitialized = cl::DataManagement::ShapeManager::data().isEmpty();

    QStringList fileNames = dialog.selectedFiles();
    for (auto path : fileNames)
        cl::DataManagement::ShapeManager::data().addLayer(path.toStdString());

    if (notInitialized)
        cl::DataManagement::ShapeManager::data().assistant().zoomToAll();
}

void MainWindow::closeAll()
{
    cl::DataManagement::ShapeManager::data().clearAllLayers();
}

void MainWindow::removeLayer()
{
    QList<QListWidgetItem*> selection = _sidebar->listWidget().selectedItems();
    if (selection.empty())
        return;

    auto layerItr = cl::DataManagement::ShapeManager::data().findByName(selection.front()->text().toStdString());
    cl::DataManagement::ShapeManager::data().removeLayer(layerItr);
}

void MainWindow::layerUp()
{
    QList<QListWidgetItem*> selection = _sidebar->listWidget().selectedItems();
    if (selection.empty())
        return;
    QListWidgetItem* selectedItem = selection.front();
    if (selectedItem == _sidebar->listWidget().item(0))
        return;

    auto layerItr = cl::DataManagement::ShapeManager::data().findByName(selectedItem->text().toStdString());
    cl::DataManagement::ShapeManager::data().rearrangeLayer(layerItr, ++(++layerItr));
}

void MainWindow::layerDown()
{
    QList<QListWidgetItem*> selection = _sidebar->listWidget().selectedItems();
    if (selection.empty())
        return;
    QListWidgetItem* selectedItem = selection.front();
    if (selectedItem == _sidebar->listWidget().item(_sidebar->listWidget().count() - 1))
        return;

    auto layerItr = cl::DataManagement::ShapeManager::data().findByName(selectedItem->text().toStdString());
    cl::DataManagement::ShapeManager::data().rearrangeLayer(layerItr, --layerItr);
}
