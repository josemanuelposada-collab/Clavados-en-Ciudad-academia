#ifndef NIVELPISCINAENTRENAMIENTO_H
#define NIVELPISCINAENTRENAMIENTO_H

#include <QPainter>
#include <QKeyEvent>
#include <QPixmap>
#include <QRectF>
#include <memory>

#include "../entidades/Personaje.h"
#include "../entidades/Plataforma.h"
#include "../agente/DronVigilante.h"
#include "Dificultad.h"
#include "NivelJuego.h"

class NivelPiscinaEntrenamiento : public NivelJuego
{
private:
    std::unique_ptr<Personaje> jugador;
    std::unique_ptr<Plataforma> plataforma;
    std::unique_ptr<DronVigilante> dron;

    QRectF piscina;
    QRectF zonaViento;
    QRectF zonaMeta;
    QRectF suelo;

    QPixmap spritePiscina;
    QPixmap spritePiscinaBase;
    QPixmap spriteViento;
    QPixmap spriteAdvertencia;
    QPixmap spriteTemporizador;

    Dificultad dificultad;
    QVector<EventoSonidoJuego> eventosSonido;

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
    float tiempoNivel;

public:
    NivelPiscinaEntrenamiento();
    ~NivelPiscinaEntrenamiento();

    void actualizar(float dt) override;
    void dibujar(QPainter& painter) override;

    void teclaPresionada(int tecla) override;
    void teclaLiberada(int tecla) override;

    void verificarColisiones();
    void verificarPiscina();
    void verificarSuelo();
    void verificarZonaViento();

    void calcularPuntajePorEntrada();

    void reiniciarIntento();
    void reiniciarNivel() override;

    void cambiarDificultad(TipoDificultad tipo);
    void aplicarParametrosDificultad();

    int getPuntaje() const;
    bool estaSuperado() const override;
    bool estaPerdido() const override;
    QString nombreNivel() const override;
    QVector<EventoSonidoJuego> consumirEventosSonido() override;
};

#endif
