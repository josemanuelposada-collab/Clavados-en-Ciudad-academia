#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include <QVector>
#include "../logica/NivelJuego.h"

class GameWidget : public QWidget
{
    Q_OBJECT

private:
    QTimer* timer;
    QVector<NivelJuego*> niveles;
    int nivelActual;
    bool mostrarAyuda;

    NivelJuego* nivel();
    void cargarNiveles();
    void avanzarNivel();

public:
    explicit GameWidget(QWidget* parent = nullptr);
    ~GameWidget();

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private slots:
    void actualizar();
};

#endif
