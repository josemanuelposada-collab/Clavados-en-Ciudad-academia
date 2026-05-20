#include "NivelPiscinaEntrenamiento.h"

NivelPiscinaEntrenamiento::NivelPiscinaEntrenamiento()
{
    jugador = new Personaje();
    plataforma = new Plataforma();

    piscina = QRectF(260, 480, 260, 70);

    puntaje = 0;
    intentoTerminado = false;

    jugador->colocarEn(plataforma->getX() + 40, plataforma->getY() - 55);
}

NivelPiscinaEntrenamiento::~NivelPiscinaEntrenamiento()
{
    delete jugador;
    delete plataforma;
}

void NivelPiscinaEntrenamiento::actualizar(float dt)
{
    plataforma->actualizar(dt);

    if (!jugador->estaEnAire()) {
        jugador->colocarEn(plataforma->getX() + 40, plataforma->getY() - 55);
    }

    if (!intentoTerminado) {
        jugador->actualizar(dt);
    }

    verificarPiscina();
}

void NivelPiscinaEntrenamiento::dibujar(QPainter& painter)
{
    painter.fillRect(QRectF(0, 0, 800, 600), QColor(170, 220, 255));

    painter.setPen(Qt::black);
    painter.drawText(20, 30, "Nivel 1: Piscina de entrenamiento");
    painter.drawText(20, 55, "Espacio: saltar | A/D: mover | R: reiniciar");
    painter.drawText(20, 80, "Puntaje: " + QString::number(puntaje));

    painter.setBrush(QBrush(QColor(0, 120, 255)));
    painter.drawRect(piscina);

    painter.setPen(Qt::white);
    painter.drawText(piscina.x() + 90, piscina.y() + 40, "Piscina");

    painter.setPen(Qt::black);

    plataforma->dibujar(painter);
    jugador->dibujar(painter);

    if (intentoTerminado) {
        painter.setBrush(QBrush(Qt::white));
        painter.drawRect(QRectF(230, 220, 330, 90));

        painter.setPen(Qt::black);
        painter.drawText(270, 260, "Intento terminado");
        painter.drawText(270, 290, "Presiona R para reiniciar");
    }
}

void NivelPiscinaEntrenamiento::teclaPresionada(int tecla)
{
    if (tecla == Qt::Key_Space) {
        jugador->saltar();
    }

    if (tecla == Qt::Key_A) {
        jugador->moverIzquierda(true);
    }

    if (tecla == Qt::Key_D) {
        jugador->moverDerecha(true);
    }

    if (tecla == Qt::Key_R) {
        reiniciar();
    }
}

void NivelPiscinaEntrenamiento::teclaLiberada(int tecla)
{
    if (tecla == Qt::Key_A) {
        jugador->moverIzquierda(false);
    }

    if (tecla == Qt::Key_D) {
        jugador->moverDerecha(false);
    }
}

void NivelPiscinaEntrenamiento::verificarPiscina()
{
    if (intentoTerminado) {
        return;
    }

    if (jugador->getY() > piscina.y()) {
        intentoTerminado = true;

        if (piscina.intersects(jugador->rect())) {
            puntaje = 100;
        } else {
            puntaje = 0;
        }

        jugador->setVX(0);
        jugador->setVY(0);
    }
}

void NivelPiscinaEntrenamiento::reiniciar()
{
    puntaje = 0;
    intentoTerminado = false;

    jugador->setEnAire(false);
    jugador->setVX(0);
    jugador->setVY(0);
    jugador->colocarEn(plataforma->getX() + 40, plataforma->getY() - 55);
}

int NivelPiscinaEntrenamiento::getPuntaje() const
{
    return puntaje;
}