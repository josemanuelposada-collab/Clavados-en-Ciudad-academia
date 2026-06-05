#include "Entidad.h"

Entidad::Entidad(float xInicial, float yInicial, float anchoInicial, float altoInicial)
    : x(xInicial),
      y(yInicial),
      ancho(anchoInicial),
      alto(altoInicial)
{
}

QRectF Entidad::rect() const
{
    return QRectF(x, y, ancho, alto);
}

QPointF Entidad::centro() const
{
    return QPointF(x + ancho / 2.0f, y + alto / 2.0f);
}

float Entidad::getX() const
{
    return x;
}

float Entidad::getY() const
{
    return y;
}

float Entidad::getAncho() const
{
    return ancho;
}

float Entidad::getAlto() const
{
    return alto;
}
