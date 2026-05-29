#ifndef DRONVIGILANTE_H
#define DRONVIGILANTE_H

#include "../entidades/Entidad.h"
#include "../entidades/Personaje.h"
#include <QPixmap>
#include <QVector>

enum EstadoDron
{
    PATRULLA,
    ESCANEO,
    INTERCEPTA
};

struct PercepcionDron
{
    float distanciaJugador;
    float velocidadJugador;
    bool jugadorCerca;
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
    QVector<float> memoriaErrores;

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
    void reiniciarMemoriaParcial();

    float calcularPresionDificultad() const;
    EstadoDron getEstado() const;
};

#endif
