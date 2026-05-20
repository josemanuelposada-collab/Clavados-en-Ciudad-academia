#ifndef PLATAFORMA_H
#define PLATAFORMA_H

#include <QPainter>
#include <QRectF>

class Plataforma
{
private:
    float x;
    float y;
    float xBase;
    float ancho;
    float alto;
    float amplitud;
    float frecuencia;
    float tiempo;

public:
    Plataforma();

    void actualizar(float dt);
    void dibujar(QPainter& painter);

    QRectF rect() const;

    float getX() const;
    float getY() const;
};

#endif