#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gui/GameWidget.h"
#include "logica/JuegoException.h"

#include <QLabel>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    try {
        GameWidget* juego = new GameWidget(this);
        setCentralWidget(juego);
    }
    catch (const JuegoException& error) {
        QLabel* aviso = new QLabel("No fue posible iniciar el juego.\n\n" + QString::fromStdString(error.what()), this);
        aviso->setAlignment(Qt::AlignCenter);
        setCentralWidget(aviso);
        QMessageBox::critical(this, "Error de recursos", error.what());
    }

    resize(1280, 720);
    setWindowTitle("Clavados en Ciudad Academia");
}

MainWindow::~MainWindow()
{
    delete ui;
}
