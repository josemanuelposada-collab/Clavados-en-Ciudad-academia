#include "Anillo.h"
#include "../render/SpriteCache.h"
#include <cmath>

Anillo::Anillo(float xInicial, float yInicial)
    : Entidad(xInicial, yInicial, 42.0f, 42.0f),
      recolectado(false),
      tiempo(0.0f)
{
    sprite.load(":/recursos/sprites/anillo_dorado.png");
}

void Anillo::actualizar(float dt)
{
    tiempo += dt;
    y += std::sin(tiempo * 5.0f) * 0.15f;
}

void Anillo::dibujar(QPainter& painter)
{
    if (recolectado) {
        return;
    }

    QRect area = rect().toRect();
    if (!sprite.isNull()) {
        SpriteCache::dibujarAjustado(painter, sprite, area, "anillo");
    }
    else {
        painter.setPen(QColor(220, 170, 0));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(rect());
    }
}

bool Anillo::estaRecolectado() const
{
    return recolectado;
}

void Anillo::recolectar()
{
    recolectado = true;
}

void Anillo::reiniciar(float nuevoX, float nuevoY)
{
    x = nuevoX;
    y = nuevoY;
    recolectado = false;
    tiempo = 0.0f;
}
