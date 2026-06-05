#ifndef NIVELJUEGO_H
#define NIVELJUEGO_H

#include <QPainter>
#include <QPointF>
#include <QString>
#include <QVector>
#include "../entidades/Personaje.h"

enum EventoSonidoJuego
{
    SONIDO_SALTO,
    SONIDO_ANILLO,
    SONIDO_COLISION,
    SONIDO_AGUA,
    SONIDO_NIVEL,
    SONIDO_GAME_OVER
};

class NivelJuego
{
public:
    virtual ~NivelJuego() = default;

    virtual void actualizar(float dt) = 0;
    virtual void dibujar(QPainter& painter) = 0;
    virtual void teclaPresionada(int tecla) = 0;
    virtual void teclaLiberada(int tecla) = 0;
    virtual void mousePresionado(const QPointF& posicion) { Q_UNUSED(posicion); }
    virtual void configurarPersonaje(TipoPersonaje tipo) { Q_UNUSED(tipo); }
    virtual void reiniciarNivel() = 0;
    virtual bool estaSuperado() const = 0;
    virtual bool estaPerdido() const = 0;
    virtual QString nombreNivel() const = 0;
    virtual QVector<EventoSonidoJuego> consumirEventosSonido() = 0;
};

#endif
