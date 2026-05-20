#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gui/GameWidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    GameWidget* juego = new GameWidget(this);
    setCentralWidget(juego);

    resize(800, 600);
    setWindowTitle("Clavados en Ciudad Academia");
}

MainWindow::~MainWindow()
{
    delete ui;
}