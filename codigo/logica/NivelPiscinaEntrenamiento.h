#ifndef NIVELPISCINAENTRENAMIENTO_H
#define NIVELPISCINAENTRENAMIENTO_H

#include <QPainter>
#include <QKeyEvent>
#include <QPixmap>
#include <QRectF>
#include <memory>

#include "../entidades/Personaje.h"
#include "../entidades/Plataforma.h"
#include "Dificultad.h"
#include "NivelJuego.h"

class NivelPiscinaEntrenamiento : public NivelJuego
{
private:
    std::unique_ptr<Personaje> jugador;
    std::unique_ptr<Plataforma> plataforma;

    QRectF piscina;
    QRectF zonaPerfecta;
    QRectF zonaViento;
    QRectF suelo;

    QPixmap spritePiscinaBase;
    QPixmap spritePiscinaPremium;
    QPixmap spriteViento;
    QPixmap spriteVientoDerecha;
    QPixmap spriteVientoIzquierda;
    QPixmap spriteFondoCiudad;
    QPixmap spriteChapuzonLimpio;
    QPixmap spriteChapuzonMedio;
    QPixmap spriteChapuzonFuerte;
    QPixmap spriteBurbujas;
    QPixmap spriteTexturaPixel;
    QPixmap spriteAlrededorPiscina;
    QPixmap spriteCorazonLleno;
    QPixmap spriteCorazonVacio;
    QPixmap spriteAdvertenciaHud;

    Dificultad dificultad;
    QVector<EventoSonidoJuego> eventosSonido;

    int intentoActual;
    int intentosMaximos;
    int puntajeTotal;
    int puntajeObjetivo;
    int puntajeUltimoIntento;
    int mejorPuntaje;
    int puntaje;

    bool intentoEnCurso;
    bool esperandoSiguienteIntento;
    bool intentoEvaluado;
    bool nivelSuperado;
    bool nivelPerdido;
    bool jugadorEnZonaViento;
    bool frenandoCaida;
    bool acelerandoCaida;
    bool corrigiendoIzquierda;
    bool corrigiendoDerecha;
    bool usoImpulsoIntento;

    float tiempoNivel;
    float tiempoIntento;
    float tiempoEsperaReinicio;
    float tiempoEsperaActual;
    float tiempoIman;
    float cooldownIman;
    float radioIman;

    float gravedad;
    float vientoLateral;
    float vientoRafaga;
    float errorEntrada;
    float velocidadEntrada;
    float anguloPostura;
    float tiempoCorreccionLateral;

    float potenciaActual;
    float potenciaMinima;
    float potenciaMaxima;
    float velocidadCambioPotencia;
    int direccionPotencia;

    float xBasePlataforma;
    float amplitudPlataforma;
    float frecuenciaPlataforma;
    float velocidadPlataforma;
    float anchoZonaPerfecta;

    void configurarIntento();
    void iniciarIntento();
    void finalizarIntento(int puntajeIntento, bool entroAlAgua);
    void actualizarPotencia(float dt);
    void actualizarPostura(float dt);
    void actualizarZonaPerfecta();
    void actualizarCamara();
    void activarIman();

    void verificarPiscina();
    void verificarSuelo();
    void verificarZonaViento();
    void calcularPuntajePorEntrada();

    void dibujarEscenario(QPainter& painter);
    void dibujarHud(QPainter& painter);
    void dibujarRafagasViento(QPainter& painter);
    void dibujarTrayectoriaGuia(QPainter& painter);
    void dibujarPotencia(QPainter& painter, const QRectF& rect);
    void dibujarGaugePostura(QPainter& painter, const QRectF& rect);
    void dibujarIman(QPainter& painter);
    void dibujarResultado(QPainter& painter);

public:
    NivelPiscinaEntrenamiento();
    ~NivelPiscinaEntrenamiento();

    void actualizar(float dt) override;
    void dibujar(QPainter& painter) override;

    void teclaPresionada(int tecla) override;
    void teclaLiberada(int tecla) override;
    void mousePresionado(const QPointF& posicion) override;

    void reiniciarNivel() override;
    void configurarPersonaje(TipoPersonaje tipo) override;
    void cambiarDificultad(TipoDificultad tipo);
    void aplicarParametrosDificultad();

    int getPuntaje() const;
    bool estaSuperado() const override;
    bool estaPerdido() const override;
    QString nombreNivel() const override;
    QVector<EventoSonidoJuego> consumirEventosSonido() override;
};

#endif
