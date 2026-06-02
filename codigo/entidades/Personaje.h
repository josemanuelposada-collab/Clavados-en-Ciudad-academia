#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <QPainter>
#include <QRectF>
#include <QPixmap>
#include "Entidad.h"

class Personaje : public Entidad
{
private:
    float vx;
    float vy;
    float ax;
    float ay;

    float masa;

    bool enAire;
    bool izquierda;
    bool derecha;
    bool impulsoActivo;

    float energia;
    float energiaMaxima;

    QPixmap spriteIdle;
    QPixmap spriteSalto;
    QPixmap spriteCaida;
    QPixmap spriteIzquierda;
    QPixmap spriteDerecha;
    QPixmap spriteImpulso;
    QPixmap spriteEntradaAgua;
    QPixmap spriteSplash;

public:
    Personaje();

    void actualizar(float dt) override;
    void dibujar(QPainter& painter) override;

    void saltar();
    void moverIzquierda(bool estado);
    void moverDerecha(bool estado);
    void activarImpulso(bool estado);

    void aplicarGravedad(float gravedad);
    void aplicarViento(float aceleracionViento);
    void aplicarFriccionAire(float factor);
    void aplicarImpulsoElectromagnetico(float dt);
    void aplicarLimites(float anchoPantalla);

    void detenerMovimiento();
    void colocarEn(float nuevoX, float nuevoY);

    float getX() const;
    float getY() const;
    float getVX() const;
    float getVY() const;
    float getAncho() const;
    float getAlto() const;
    float getEnergia() const;
    float getEnergiaMaxima() const;

    void setVX(float nuevoVX);
    void setVY(float nuevoVY);
    void setEnAire(bool estado);
    void setEnergiaMaxima(float nuevaEnergia);

    bool estaEnAire() const;
    bool estaUsandoImpulso() const;
};

#endif
