#include "Plataforma.h"
#include "../render/SpriteCache.h"
#include <cmath>

Plataforma::Plataforma()
    : Entidad(90.0f, 365.0f, 170.0f, 55.0f),
      oscilacion(45.0f, 2.0f)
{
    xBase = 90.0f;

    amplitud = 45.0f;
    frecuencia = 2.0f;
    tiempo = 0.0f;
    rutaSprite = ":/recursos/sprites/plataforma_media.png";
}

void Plataforma::actualizar(float dt)
{
    tiempo += dt;
    x = oscilacion.calcular(tiempo, xBase);
}

void Plataforma::dibujar(QPainter& painter)
{
    QRectF area(x, y, ancho, alto);
    const QPixmap& spritePlataforma = SpriteCache::obtener(rutaSprite);

    if (!spritePlataforma.isNull()) {
        SpriteCache::dibujarAjustado(painter, spritePlataforma, area.toRect(), "plataforma");
    }
    else {
        painter.setPen(Qt::black);
        painter.setBrush(QBrush(Qt::darkGray));
        painter.drawRect(area);
    }

}

void Plataforma::configurarOscilacion(float nuevaAmplitud, float nuevaFrecuencia)
{
    amplitud = nuevaAmplitud;
    frecuencia = nuevaFrecuencia;
    oscilacion.configurar(amplitud, frecuencia);
}

void Plataforma::colocarEn(float nuevoX, float nuevoY)
{
    xBase = nuevoX;
    x = nuevoX;
    y = nuevoY;
}
