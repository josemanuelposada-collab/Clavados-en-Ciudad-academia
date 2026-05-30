#ifndef NIVELJUEGO_H
#define NIVELJUEGO_H

#include <QPainter>
#include <QString>

class NivelJuego
{
public:
    virtual ~NivelJuego() {}

    virtual void actualizar(float dt) = 0;
    virtual void dibujar(QPainter& painter) = 0;
    virtual void teclaPresionada(int tecla) = 0;
    virtual void teclaLiberada(int tecla) = 0;
    virtual void reiniciarNivel() = 0;
    virtual bool estaSuperado() const = 0;
    virtual bool estaPerdido() const = 0;
    virtual QString nombreNivel() const = 0;
};

#endif
