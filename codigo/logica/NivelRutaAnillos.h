#ifndef NIVELRUTAANILLOS_H
#define NIVELRUTAANILLOS_H

#include "NivelJuego.h"
#include "Dificultad.h"
#include "../entidades/Anillo.h"
#include "../entidades/Moneda.h"
#include "../entidades/Obstaculo.h"
#include "../entidades/ProyectilDron.h"
#include "../agente/DronVigilante.h"
#include <QPixmap>
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
    std::vector<std::unique_ptr<ProyectilDron>> proyectilesDron;

    bool teclaIzquierda;
    bool teclaDerecha;
    bool teclaArriba;
    bool teclaAbajo;
    QPixmap spriteFondo;
    QPixmap spriteAlarma;
    QPixmap spriteViento;
    QPixmap spriteVientoDerecha;
    QPixmap spriteVientoIzquierda;
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
    QPixmap spriteTexturaTierra;
    QPixmap spriteDecoracionIzq;
    QPixmap spriteDecoracionDer;
    QPixmap spriteTexturaPixel;
    QPixmap spriteAdvertenciaHud;
    QPixmap spriteCorazonLleno;
    QPixmap spriteCorazonVacio;
    QPixmap spritePlataformaInicio;
    QPixmap spriteAlrededorPiscina;
    QPixmap spritePoderMikoto;
    QPixmap spritePoderAccelerator;
    QPixmap spritePoderMugino;
    QPixmap spritePoderDarkMatter;

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
    float piscinaVelocidad;
    float piscinaAceleracion;
    float tiempoProximaRafaga;
    float tiempoRafaga;
    float intensidadRafaga;
    float direccionRafaga;
    float yRafaga;
    float tiempoEntradaDron;
    float tiempoCorreccionLateral;
    float tiempoInvulnerable;
    bool poderUsadoNivel;
    int calidadEntrada;
    bool nivelSuperado;
    bool nivelPerdido;
    bool entradaAguaActiva;
    bool resultadoEntradaSuperado;
    bool saltoInicialPendiente;
    bool dronActivo;
    QString motivoDerrotaActual;

public:
    NivelRutaAnillos();
    ~NivelRutaAnillos();

    void actualizar(float dt) override;
    void dibujar(QPainter& painter) override;
    void teclaPresionada(int tecla) override;
    void teclaLiberada(int tecla) override;
    void mousePresionado(const QPointF& posicion) override;
    void reiniciarNivel() override;
    void configurarPersonaje(TipoPersonaje tipo) override;
    bool estaSuperado() const override;
    bool estaPerdido() const override;
    QString motivoDerrota() const override;
    QString nombreNivel() const override;
    QVector<EventoSonidoJuego> consumirEventosSonido() override;

    void cambiarDificultad(TipoDificultad tipo) override;

private:
    void crearEntidades();
    void crearMonedas();
    void liberarEntidades();
    void aplicarMovimientoJugador(float dt);
    void actualizarMonedas(float dt);
    void aplicarMeltdownerInstantaneo();
    void aplicarPresionAgente(float dt);
    void actualizarProyectilesDron(float dt);
    void crearProyectilDron();
    void resolverColisionProyectil(ProyectilDron& proyectil);
    void actualizarPiscina(float dt);
    void verificarInteracciones();
    void registrarEntradaAgua();
    void calcularPuntaje();
    void actualizarCamara();
    void dibujarEscenario(QPainter& painter);
    void dibujarHud(QPainter& painter);
    void dibujarPiscinaFinal(QPainter& painter);
    void dibujarEntradaAgua(QPainter& painter);
    void dibujarIman(QPainter& painter);
    void dibujarProyectilesDron(QPainter& painter);
    bool estaVisible(const QRectF& rect) const;
};

#endif
