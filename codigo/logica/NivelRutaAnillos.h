#ifndef NIVELRUTAANILLOS_H
#define NIVELRUTAANILLOS_H

#include "NivelJuego.h"
#include "Dificultad.h"
#include "../entidades/Anillo.h"
#include "../entidades/Moneda.h"
#include "../entidades/Obstaculo.h"
#include "../agente/DronVigilante.h"
#include <QPixmap>
#include <QSet>
#include <memory>
#include <vector>

class NivelRutaAnillos : public NivelJuego
{
private:
    std::unique_ptr<Personaje> jugador;
    std::unique_ptr<DronVigilante> dron;
    std::vector<std::unique_ptr<Anillo>> anillos;
    std::vector<std::unique_ptr<Moneda>> monedas;
    std::vector<std::unique_ptr<Obstaculo>> obstaculos;

    QSet<int> teclas;
    QPixmap spriteFondo;
    QPixmap spriteAlarma;
    QPixmap spriteViento;
    QPixmap spritePiscinaFinal;
    QPixmap spriteColumnaTorre;
    QPixmap spriteBanderines;
    QPixmap spriteEdificio;
    QPixmap spriteTorreEpica;
    QPixmap spritePiscinaPremium;
    QPixmap spriteAguaProfunda;
    QPixmap spriteChapuzonLimpio;
    QPixmap spriteChapuzonMedio;
    QPixmap spriteChapuzonFuerte;
    QPixmap spriteBurbujas;

    Dificultad dificultad;
    QVector<EventoSonidoJuego> eventosSonido;
    QRectF piscinaFinal;
    QRectF zonaViento;
    QRectF zonaVelocidad;

    int anillosRecolectados;
    int monedasRecolectadas;
    int golpes;
    int puntaje;
    float tiempoRestante;
    float tiempoTotal;
    float tiempoNivel;
    float alturaMundo;
    float camaraY;
    float velocidadVertical;
    float velocidadHorizontal;
    float tiempoEntradaAgua;
    float xEntradaAgua;
    float velocidadEntradaAgua;
    float tiempoIman;
    float cooldownIman;
    float radioIman;
    int calidadEntrada;
    bool nivelSuperado;
    bool nivelPerdido;
    bool entradaAguaActiva;
    bool resultadoEntradaSuperado;

public:
    NivelRutaAnillos();
    ~NivelRutaAnillos();

    void actualizar(float dt) override;
    void dibujar(QPainter& painter) override;
    void teclaPresionada(int tecla) override;
    void teclaLiberada(int tecla) override;
    void mousePresionado(const QPointF& posicion) override;
    void reiniciarNivel() override;
    bool estaSuperado() const override;
    bool estaPerdido() const override;
    QString nombreNivel() const override;
    QVector<EventoSonidoJuego> consumirEventosSonido() override;

    void cambiarDificultad(TipoDificultad tipo);

private:
    void crearEntidades();
    void crearMonedas();
    void liberarEntidades();
    void aplicarMovimientoJugador(float dt);
    void actualizarMonedas(float dt);
    void verificarInteracciones();
    void registrarEntradaAgua();
    void calcularPuntaje();
    void actualizarCamara();
    void dibujarEscenario(QPainter& painter);
    void dibujarHud(QPainter& painter);
    void dibujarPiscinaFinal(QPainter& painter);
    void dibujarEntradaAgua(QPainter& painter);
    void dibujarIman(QPainter& painter);
};

#endif
