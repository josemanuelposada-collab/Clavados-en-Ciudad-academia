#include "GameWidget.h"
#include <QPainter>
#include <QApplication>
#include "../logica/NivelPiscinaEntrenamiento.h"
#include "../logica/NivelRutaAnillos.h"
#include "../logica/JuegoException.h"

GameWidget::GameWidget(QWidget* parent)
    : QWidget(parent),
      timer(nullptr),
      nivelActual(0),
      mostrarAyuda(true)
{
    setFocusPolicy(Qt::StrongFocus);

    cargarNiveles();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GameWidget::actualizar);
    timer->start(16);
}

GameWidget::~GameWidget()
{
    for (NivelJuego* nivelJuego : niveles) {
        delete nivelJuego;
    }
}

NivelJuego* GameWidget::nivel()
{
    if (niveles.isEmpty()) {
        throw JuegoException("No hay niveles cargados.");
    }

    return niveles[nivelActual];
}

void GameWidget::cargarNiveles()
{
    niveles.push_back(new NivelPiscinaEntrenamiento());
    niveles.push_back(new NivelRutaAnillos());
}

void GameWidget::avanzarNivel()
{
    if (nivel()->estaSuperado() && nivelActual < niveles.size() - 1) {
        nivelActual++;
        nivel()->reiniciarNivel();
        QApplication::beep();
    }
}

void GameWidget::actualizar()
{
    try {
        nivel()->actualizar(0.016f);
    }
    catch (const JuegoException&) {
        QApplication::beep();
    }

    update();
}

void GameWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    try {
        nivel()->dibujar(painter);

        painter.setPen(Qt::white);
        painter.drawText(610, 30, "Nivel " + QString::number(nivelActual + 1) + "/" + QString::number(niveles.size()));
        painter.drawText(610, 55, "Tab: cambiar nivel");
        painter.drawText(610, 80, "H: ayuda");
        if (nivel()->estaSuperado() && nivelActual < niveles.size() - 1) {
            painter.drawText(520, 105, "Enter: siguiente nivel");
        }

        if (mostrarAyuda) {
            painter.setBrush(QColor(0, 0, 0, 135));
            painter.setPen(Qt::NoPen);
            painter.drawRoundedRect(QRectF(515, 500, 265, 72), 6, 6);
            painter.setPen(Qt::white);
            painter.drawText(535, 528, "Objetivo: supera ambos niveles.");
            painter.drawText(535, 552, "Dificultad configurable con 1, 2 y 3.");
        }
    }
    catch (const JuegoException& error) {
        painter.fillRect(rect(), QColor(25, 25, 25));
        painter.setPen(Qt::white);
        painter.drawText(60, 80, "Error del juego:");
        painter.drawText(60, 110, error.what());
    }
}

void GameWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Tab) {
        nivelActual = (nivelActual + 1) % niveles.size();
        QApplication::beep();
        return;
    }

    if (event->key() == Qt::Key_H) {
        mostrarAyuda = !mostrarAyuda;
        return;
    }

    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && nivel()->estaSuperado()) {
        avanzarNivel();
        return;
    }

    nivel()->teclaPresionada(event->key());
}

void GameWidget::keyReleaseEvent(QKeyEvent* event)
{
    nivel()->teclaLiberada(event->key());
}
