#include "Personaje.h"

Personaje::Personaje()
{
    x = 120;
    y = 300;
    vx = 0;
    vy = 0;

    ancho = 35;
    alto = 55;

    enAire = false;
    izquierda = false;
    derecha = false;
}

void Personaje::actualizar(float dt)
{
    if (izquierda) {
        vx = -180;
    } else if (derecha) {
        vx = 180;
    } else {
        vx = 0;
    }

    if (enAire) {
        float gravedad = 700.0f;

        vy = vy + gravedad * dt;
        x = x + vx * dt;
        y = y + vy * dt;
    }
}

void Personaje::dibujar(QPainter& painter)
{
    painter.setPen(Qt::black);

    painter.setBrush(QBrush(Qt::red));
    painter.drawRect(QRectF(x, y, ancho, alto));

    painter.setBrush(QBrush(Qt::blue));
    painter.drawRect(QRectF(x + 8, y + 15, ancho - 16, alto - 15));
}

void Personaje::saltar()
{
    if (!enAire) {
        enAire = true;
        vy = -430;
    }
}

void Personaje::moverIzquierda(bool estado)
{
    izquierda = estado;
}

void Personaje::moverDerecha(bool estado)
{
    derecha = estado;
}

void Personaje::colocarEn(float nuevoX, float nuevoY)
{
    x = nuevoX;
    y = nuevoY;
}

QRectF Personaje::rect() const
{
    return QRectF(x, y, ancho, alto);
}

float Personaje::getX() const
{
    return x;
}

float Personaje::getY() const
{
    return y;
}

void Personaje::setVX(float nuevoVX)
{
    vx = nuevoVX;
}

void Personaje::setVY(float nuevoVY)
{
    vy = nuevoVY;
}

void Personaje::setEnAire(bool estado)
{
    enAire = estado;
}

bool Personaje::estaEnAire() const
{
    return enAire;
}