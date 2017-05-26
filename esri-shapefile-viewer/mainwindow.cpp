#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QTime>
#include <QLabel>

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
    cl::ShapeManager::data().setObserver(*this);

    // Connect the open file signal.
    connect(ui->actionOpen, SIGNAL(triggered(bool)), this, SLOT(openFile()));
    // If the slot function name is wrong,
    // without any error prompts the connection will not work.

    // Show the main window.
    show();
}

MainWindow::~MainWindow() {}

void MainWindow::openFile()
{
    QFileDialog dialog(this, tr("Open ESRI Shape File:"), "", tr("*.shp"));
    if (!dialog.exec())
        return;

    QStringList fileNames = dialog.selectedFiles();
    QString qsPath = fileNames.at(0);
    std::string sPath = qsPath.toStdString();

    cl::ShapeManager::data().addShape(sPath);
}

void MainWindow::updateDisplay()
{
    _viewForm->update();
    _sidebar->updateList();
    update();
}

QRect const MainWindow::paintingRect() const
{
    return _viewForm->rect();
}

void MainWindow::setLabel(QString const& msg)
{
    if (_msgLabel != nullptr)
        _msgLabel->setText(msg);
}
