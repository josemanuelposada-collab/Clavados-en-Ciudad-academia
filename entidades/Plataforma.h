#ifndef PLATAFORMA_H
#define PLATAFORMA_H

#include <QPainter>
#include <QRectF>
#include <QPixmap>
#include "Entidad.h"
#include "../fisicas/ModelosFisicos.h"

class Plataforma : public Entidad
{
private:
    float xBase;
    float amplitud;
    float frecuencia;
    float tiempo;
    ModeloOscilatorio oscilacion;

    QPixmap spritePlataforma;

public:
    Plataforma();

    void actualizar(float dt) override;
    void dibujar(QPainter& painter) override;

    void configurarOscilacion(float nuevaAmplitud, float nuevaFrecuencia);

    void colocarEn(float nuevoX, float nuevoY);
};

#endif
