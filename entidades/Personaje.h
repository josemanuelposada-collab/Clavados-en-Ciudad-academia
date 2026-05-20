#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <QPainter>
#include <QRectF>

class Personaje
{
private:
    float x;
    float y;
    float vx;
    float vy;
    float ancho;
    float alto;

    bool enAire;
    bool izquierda;
    bool derecha;

public:
    Personaje();

    void actualizar(float dt);
    void dibujar(QPainter& painter);

    void saltar();
    void moverIzquierda(bool estado);
    void moverDerecha(bool estado);

    void colocarEn(float nuevoX, float nuevoY);

    QRectF rect() const;

    float getX() const;
    float getY() const;

    void setVX(float nuevoVX);
    void setVY(float nuevoVY);
    void setEnAire(bool estado);

    bool estaEnAire() const;
};

#endif