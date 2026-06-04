#ifndef PROYECTILDRON_H
#define PROYECTILDRON_H

#include "Entidad.h"
#include <QColor>

class ProyectilDron : public Entidad
{
private:
    float vx;
    float vy;
    float masa;
    float tiempoVida;
    bool activo;
    bool elastico;
    QColor color;

public:
    ProyectilDron(float xInicial, float yInicial, float vxInicial, float vyInicial, bool colisionElastica);

    void actualizar(float dt) override;
    void dibujar(QPainter& painter) override;

    bool estaActivo() const;
    bool usaColisionElastica() const;
    void desactivar();

    float getVX() const;
    float getVY() const;
    float getMasa() const;
    void setVX(float nuevoVX);
    void setVY(float nuevoVY);
};

#endif
