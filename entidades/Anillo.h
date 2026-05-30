#ifndef ANILLO_H
#define ANILLO_H

#include "Entidad.h"
#include <QPixmap>

class Anillo : public Entidad
{
private:
    bool recolectado;
    float tiempo;
    QPixmap sprite;

public:
    Anillo(float xInicial, float yInicial);

    void actualizar(float dt) override;
    void dibujar(QPainter& painter) override;
    bool estaRecolectado() const;
    void recolectar();
    void reiniciar(float nuevoX, float nuevoY);
};

#endif
