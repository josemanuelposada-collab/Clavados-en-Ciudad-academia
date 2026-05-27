#ifndef NIVELPISCINAENTRENAMIENTO_H
#define NIVELPISCINAENTRENAMIENTO_H

#include <QPainter>
#include <QKeyEvent>
#include <QPixmap>
#include <QRectF>

#include "../entidades/Personaje.h"
#include "../entidades/Plataforma.h"
#include "Dificultad.h"

class NivelPiscinaEntrenamiento
{
private:
    Personaje* jugador;
    Plataforma* plataforma;

    QRectF piscina;
    QRectF zonaViento;
    QRectF zonaMeta;
    QRectF suelo;

    QPixmap spritePiscina;
    QPixmap spriteViento;
    QPixmap spriteAdvertencia;
    QPixmap spriteTemporizador;

    Dificultad dificultad;

    int puntaje;
    int mejorPuntaje;
    int intentosRestantes;

    bool intentoTerminado;
    bool intentoGanado;
    bool nivelSuperado;
    bool nivelPerdido;
    bool jugadorEnZonaViento;

    float vientoLateral;
    float gravedad;
    float errorEntrada;

public:
    NivelPiscinaEntrenamiento();
    ~NivelPiscinaEntrenamiento();

    void actualizar(float dt);
    void dibujar(QPainter& painter);

    void teclaPresionada(int tecla);
    void teclaLiberada(int tecla);

    void verificarColisiones();
    void verificarPiscina();
    void verificarSuelo();
    void verificarZonaViento();

    void calcularPuntajePorEntrada();

    void reiniciarIntento();
    void reiniciarNivel();

    void cambiarDificultad(TipoDificultad tipo);
    void aplicarParametrosDificultad();

    int getPuntaje() const;
};

#endif