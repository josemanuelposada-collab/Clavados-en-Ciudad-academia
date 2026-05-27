#ifndef PLATAFORMA_H
#define PLATAFORMA_H

#include <QPainter>
#include <QRectF>
#include <QPixmap>

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

    QPixmap spritePlataforma;

public:
    Plataforma();

    void actualizar(float dt);
    void dibujar(QPainter& painter);

    void configurarOscilacion(float nuevaAmplitud, float nuevaFrecuencia);

    QRectF rect() const;

    float getX() const;
    float getY() const;
    float getAncho() const;
    float getAlto() const;
};

#endif