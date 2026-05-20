#include "Plataforma.h"
#include <cmath>

Plataforma::Plataforma()
{
    xBase = 110;
    x = xBase;
    y = 360;

    ancho = 120;
    alto = 18;

    amplitud = 45;
    frecuencia = 2.0f;
    tiempo = 0;
}

void Plataforma::actualizar(float dt)
{
    tiempo = tiempo + dt;
    x = xBase + amplitud * std::sin(frecuencia * tiempo);
}

void Plataforma::dibujar(QPainter& painter)
{
    painter.setPen(Qt::black);
    painter.setBrush(QBrush(Qt::darkGray));
    painter.drawRect(QRectF(x, y, ancho, alto));
}

QRectF Plataforma::rect() const
{
    return QRectF(x, y, ancho, alto);
}

float Plataforma::getX() const
{
    return x;
}

float Plataforma::getY() const
{
    return y;
}