#ifndef MONEDA_H
#define MONEDA_H

#include "Entidad.h"
#include <QPixmap>

class Moneda : public Entidad
{
private:
    bool recolectada;
    float tiempo;
    float vx;
    float vy;
    QPixmap sprite;

public:
    Moneda(float xInicial, float yInicial);

    void actualizar(float dt) override;
    void dibujar(QPainter& painter) override;
    void atraerHacia(const QPointF& objetivo, float intensidad, float radio, float dt);
    void desplazarVectorialHacia(const QPointF& objetivo, float intensidad, float radio, float dt);
    void canalizarMeltdowner(const QPointF& origen, float intensidad, float alcanceVertical, float dt);
    void orbitarHacia(const QPointF& objetivo, float intensidad, float radio, float dt);
    void recolectar();
    bool estaRecolectada() const;
};

#endif
