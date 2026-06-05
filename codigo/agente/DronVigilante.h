#ifndef DRONVIGILANTE_H
#define DRONVIGILANTE_H

#include "../entidades/Entidad.h"
#include "../entidades/Personaje.h"
#include <QPixmap>
#include <array>

enum EstadoDron
{
    PATRULLA,
    ESCANEO,
    ANTICIPA,
    INTERCEPTA
};

struct PercepcionDron
{
    float distanciaJugador;
    float velocidadJugador;
    float distanciaCuadrada;
    float rapidezCuadrada;
    float dxJugador;
    float dyJugador;
    float prediccionX;
    float presionAprendida;
    bool jugadorCerca;
    bool jugadorRapido;
    bool jugadorImpulsando;
};

class DronVigilante : public Entidad
{
private:
    float xBase;
    float velocidad;
    float tiempo;
    float tiempoDecision;
    int aciertosJugador;
    EstadoDron estado;
    static constexpr int TAMANO_MEMORIA = 10;
    std::array<float, TAMANO_MEMORIA> memoriaErrores;
    int memoriaCantidad;
    int memoriaIndice;
    float sumaErrores;
    float ultimoError;
    float tendenciaError;
    float objetivoSuavizado;
    float presionActual;
    float tiempoDisparo;
    float tiempoDesdeImpacto;
    int impactosJugador;
    int evasionesJugador;

    QPixmap spriteNormal;
    QPixmap spriteEscaneo;
    QPixmap spriteAlerta;

public:
    DronVigilante(float xInicial, float yInicial);

    void actualizar(float dt) override;
    void actualizar(float dt, const Personaje& jugador);
    void dibujar(QPainter& painter) override;

    PercepcionDron percibir(const Personaje& jugador) const;
    EstadoDron razonar(const PercepcionDron& percepcion);
    void actuar(float dt, const Personaje& jugador);
    void aprender(float errorEntrada);
    void registrarAciertoJugador();
    void registrarImpactoJugador();
    void registrarEvasionJugador();
    void reiniciarMemoriaParcial();
    void colocarEn(float nuevoX, float nuevoY);
    void colocarY(float nuevoY);
    bool solicitarDisparo(float dt, const Personaje& jugador);
    QPointF calcularVectorDisparo(const Personaje& jugador, float rapidez) const;

    float calcularPresionDificultad() const;
    EstadoDron getEstado() const;
};

#endif
