#include "Plataforma.h"
#include <cmath>

Plataforma::Plataforma()
{
    xBase = 90.0f;
    x = xBase;
    y = 365.0f;

    ancho = 170.0f;
    alto = 55.0f;

    amplitud = 45.0f;
    frecuencia = 2.0f;
    tiempo = 0.0f;

    spritePlataforma.load(":/recursos/sprites/plataforma_media.png");
}

void Plataforma::actualizar(float dt)
{
    tiempo += dt;
    x = xBase + amplitud * std::sin(frecuencia * tiempo);
}

void Plataforma::dibujar(QPainter& painter)
{
    QRectF area(x, y, ancho, alto);

    if (!spritePlataforma.isNull()) {
        painter.drawPixmap(
            area.toRect(),
            spritePlataforma.scaled(
                area.width(),
                area.height(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
                )
            );
    }
    else {
        painter.setPen(Qt::black);
        painter.setBrush(QBrush(Qt::darkGray));
        painter.drawRect(area);
    }

    // Hitbox física de la plataforma. Luego puedes comentarla.
    painter.setPen(QPen(QColor(255, 0, 0, 100), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(area);
}

void Plataforma::configurarOscilacion(float nuevaAmplitud, float nuevaFrecuencia)
{
    amplitud = nuevaAmplitud;
    frecuencia = nuevaFrecuencia;
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

float Plataforma::getAncho() const
{
    return ancho;
}

float Plataforma::getAlto() const
{
    return alto;
}