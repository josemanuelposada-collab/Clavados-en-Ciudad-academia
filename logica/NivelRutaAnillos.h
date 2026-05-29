#ifndef NIVELRUTAANILLOS_H
#define NIVELRUTAANILLOS_H

#include "NivelJuego.h"
#include "Dificultad.h"
#include "../entidades/Anillo.h"
#include "../entidades/Obstaculo.h"
#include "../agente/DronVigilante.h"
#include <QPixmap>
#include <QSet>
#include <vector>

class NivelRutaAnillos : public NivelJuego
{
private:
    Personaje* jugador;
    DronVigilante* dron;
    std::vector<Anillo*> anillos;
    std::vector<Obstaculo*> obstaculos;

    QSet<int> teclas;
    QPixmap spriteFondo;
    QPixmap spriteAlarma;

    Dificultad dificultad;

    int anillosRecolectados;
    int golpes;
    int puntaje;
    float tiempoRestante;
    float tiempoTotal;
    float tiempoNivel;
    bool nivelSuperado;
    bool nivelPerdido;

public:
    NivelRutaAnillos();
    ~NivelRutaAnillos();

    void actualizar(float dt) override;
    void dibujar(QPainter& painter) override;
    void teclaPresionada(int tecla) override;
    void teclaLiberada(int tecla) override;
    void reiniciarNivel() override;
    bool estaSuperado() const override;
    bool estaPerdido() const override;
    QString nombreNivel() const override;

    void cambiarDificultad(TipoDificultad tipo);

private:
    void crearEntidades();
    void liberarEntidades();
    void aplicarMovimientoJugador(float dt);
    void verificarInteracciones();
    void calcularPuntaje();
};

#endif
