#ifndef NIVELPISCINAENTRENAMIENTO_H
#define NIVELPISCINAENTRENAMIENTO_H

#include <QPainter>
#include <QKeyEvent>
#include <QPixmap>
#include <QRectF>
#include <memory>

#include "../entidades/Personaje.h"
#include "../entidades/Plataforma.h"
#include "../entidades/Moneda.h"
#include "../agente/DronVigilante.h"
#include "Dificultad.h"
#include "NivelJuego.h"
#include <vector>

class NivelPiscinaEntrenamiento : public NivelJuego
{
private:
    std::unique_ptr<Personaje> jugador;
    std::unique_ptr<Plataforma> plataforma;
    std::unique_ptr<DronVigilante> dron;
    std::vector<std::unique_ptr<Moneda>> monedas;

    QRectF piscina;
    QRectF zonaViento;
    QRectF zonaMeta;
    QRectF suelo;

    QPixmap spritePiscina;
    QPixmap spritePiscinaBase;
    QPixmap spriteViento;
    QPixmap spriteAdvertencia;
    QPixmap spriteTemporizador;
    QPixmap spriteFondoCiudad;
    QPixmap spriteGradas;
    QPixmap spriteCarriles;
    QPixmap spriteBanderines;
    QPixmap spriteEdificio;
    QPixmap spriteBrilloAgua;
    QPixmap spritePiscinaPremium;
    QPixmap spriteAguaProfunda;
    QPixmap spriteChapuzonLimpio;
    QPixmap spriteChapuzonMedio;
    QPixmap spriteChapuzonFuerte;
    QPixmap spriteBurbujas;

    Dificultad dificultad;
    QVector<EventoSonidoJuego> eventosSonido;

    int puntaje;
    int mejorPuntaje;
    int intentosRestantes;
    int monedasRecolectadas;

    bool intentoTerminado;
    bool intentoGanado;
    bool nivelSuperado;
    bool nivelPerdido;
    bool jugadorEnZonaViento;

    float vientoLateral;
    float gravedad;
    float errorEntrada;
    float tiempoNivel;
    float alturaMundo;
    float camaraY;
    float tiempoIman;
    float cooldownIman;
    float radioIman;

public:
    NivelPiscinaEntrenamiento();
    ~NivelPiscinaEntrenamiento();

    void actualizar(float dt) override;
    void dibujar(QPainter& painter) override;

    void teclaPresionada(int tecla) override;
    void teclaLiberada(int tecla) override;
    void mousePresionado(const QPointF& posicion) override;

    void verificarColisiones();
    void verificarPiscina();
    void verificarSuelo();
    void verificarZonaViento();

    void calcularPuntajePorEntrada();
    void crearMonedas();
    void actualizarMonedas(float dt);
    void activarIman();
    void actualizarCamara();
    void dibujarEscenario(QPainter& painter);
    void dibujarHud(QPainter& painter);
    void dibujarRafagasViento(QPainter& painter);
    void dibujarIman(QPainter& painter);

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
