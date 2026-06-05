#ifndef OBSTACULO_H
#define OBSTACULO_H

#include "Entidad.h"
#include <QString>

enum TipoObstaculo
{
    BOYA,
    BARRIL,
    MINA,
    BLOQUEO
};

class Obstaculo : public Entidad
{
private:
    float vx;
    float vy;
    TipoObstaculo tipo;
    QString rutaSprite;

public:
    Obstaculo(float xInicial, float yInicial, float anchoInicial, float altoInicial,
              float vxInicial, float vyInicial, TipoObstaculo tipoInicial);

    void actualizar(float dt) override;
    void dibujar(QPainter& painter) override;
    void rebotarHorizontal(float limiteIzquierdo, float limiteDerecho);
    TipoObstaculo getTipo() const;
};

#endif
