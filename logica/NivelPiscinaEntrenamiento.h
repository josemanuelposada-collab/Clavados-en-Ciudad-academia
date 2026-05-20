#ifndef NIVELPISCINAENTRENAMIENTO_H
#define NIVELPISCINAENTRENAMIENTO_H

#include <QPainter>
#include <QKeyEvent>
#include "../entidades/Personaje.h"
#include "../entidades/Plataforma.h"

class NivelPiscinaEntrenamiento
{
private:
    Personaje* jugador;
    Plataforma* plataforma;

    QRectF piscina;

    int puntaje;
    bool intentoTerminado;

public:
    NivelPiscinaEntrenamiento();
    ~NivelPiscinaEntrenamiento();

    void actualizar(float dt);
    void dibujar(QPainter& painter);

    void teclaPresionada(int tecla);
    void teclaLiberada(int tecla);

    void verificarPiscina();
    void reiniciar();

    int getPuntaje() const;
};

#endif