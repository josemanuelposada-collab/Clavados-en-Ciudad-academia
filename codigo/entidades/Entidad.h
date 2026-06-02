#ifndef ENTIDAD_H
#define ENTIDAD_H

#include <QPainter>
#include <QRectF>

class Entidad
{
protected:
    float x;
    float y;
    float ancho;
    float alto;

public:
    Entidad(float xInicial, float yInicial, float anchoInicial, float altoInicial);
    virtual ~Entidad();

    virtual void actualizar(float dt) = 0;
    virtual void dibujar(QPainter& painter) = 0;

    virtual QRectF rect() const;
    QPointF centro() const;

    float getX() const;
    float getY() const;
    float getAncho() const;
    float getAlto() const;
};

#endif
