#include "DronVigilante.h"
#include <algorithm>
#include <cmath>

DronVigilante::DronVigilante(float xInicial, float yInicial)
    : Entidad(xInicial, yInicial, 70.0f, 48.0f),
      xBase(xInicial),
      velocidad(95.0f),
      tiempo(0.0f),
      tiempoDecision(0.0f),
      aciertosJugador(0),
      estado(PATRULLA)
{
    spriteNormal.load(":/recursos/sprites/dron_normal.png");
    spriteEscaneo.load(":/recursos/sprites/dron_escaneo.png");
    spriteAlerta.load(":/recursos/sprites/dron_alerta.png");
}

void DronVigilante::actualizar(float dt)
{
    tiempo += dt;
    x = xBase + 120.0f * std::sin(tiempo * 1.4f);
}

void DronVigilante::actualizar(float dt, const Personaje& jugador)
{
    tiempo += dt;
    tiempoDecision += dt;

    if (tiempoDecision >= 0.18f) {
        estado = razonar(percibir(jugador));
        tiempoDecision = 0.0f;
    }

    actuar(dt, jugador);
}

void DronVigilante::dibujar(QPainter& painter)
{
    QPixmap sprite = spriteNormal;
    if (estado == ESCANEO && !spriteEscaneo.isNull()) {
        sprite = spriteEscaneo;
    }
    else if (estado == INTERCEPTA && !spriteAlerta.isNull()) {
        sprite = spriteAlerta;
    }

    QRect area = rect().toRect();
    if (!sprite.isNull()) {
        painter.drawPixmap(area, sprite.scaled(area.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else {
        painter.setPen(Qt::black);
        painter.setBrush(QColor(80, 80, 180));
        painter.drawRoundedRect(rect(), 6, 6);
    }

    if (estado != PATRULLA) {
        painter.setPen(QPen(QColor(255, 80, 80, 130), 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(QRectF(x - 45, y - 38, ancho + 90, alto + 76));
    }
}

PercepcionDron DronVigilante::percibir(const Personaje& jugador) const
{
    float dx = jugador.centro().x() - centro().x();
    float dy = jugador.centro().y() - centro().y();
    float distancia = std::sqrt(dx * dx + dy * dy);
    float velocidadJugador = std::sqrt(jugador.getVX() * jugador.getVX() + jugador.getVY() * jugador.getVY());

    PercepcionDron percepcion;
    percepcion.distanciaJugador = distancia;
    percepcion.velocidadJugador = velocidadJugador;
    percepcion.jugadorCerca = distancia < 230.0f;
    percepcion.jugadorImpulsando = jugador.estaUsandoImpulso();
    return percepcion;
}

EstadoDron DronVigilante::razonar(const PercepcionDron& percepcion)
{
    float presion = calcularPresionDificultad();

    if (percepcion.jugadorCerca && (percepcion.jugadorImpulsando || percepcion.velocidadJugador > 520.0f || presion > 0.55f)) {
        return INTERCEPTA;
    }

    if (percepcion.jugadorCerca) {
        return ESCANEO;
    }

    return PATRULLA;
}

void DronVigilante::actuar(float dt, const Personaje& jugador)
{
    if (estado == PATRULLA) {
        x = xBase + 130.0f * std::sin(tiempo * 1.2f);
        return;
    }

    float objetivo = jugador.centro().x() - ancho / 2.0f;
    float direccion = objetivo > x ? 1.0f : -1.0f;
    float multiplicador = estado == INTERCEPTA ? 1.85f : 1.0f;

    if (std::abs(objetivo - x) > 8.0f) {
        x += direccion * velocidad * multiplicador * dt;
    }
}

void DronVigilante::aprender(float errorEntrada)
{
    memoriaErrores.push_back(errorEntrada);
    if (memoriaErrores.size() > 8) {
        memoriaErrores.pop_front();
    }
}

void DronVigilante::registrarAciertoJugador()
{
    aciertosJugador++;
}

void DronVigilante::reiniciarMemoriaParcial()
{
    estado = PATRULLA;
    tiempoDecision = 0.0f;
}

void DronVigilante::colocarEn(float nuevoX, float nuevoY)
{
    xBase = nuevoX;
    x = nuevoX;
    y = nuevoY;
}

float DronVigilante::calcularPresionDificultad() const
{
    if (memoriaErrores.isEmpty()) {
        return aciertosJugador > 0 ? 0.4f : 0.0f;
    }

    float suma = 0.0f;
    for (float error : memoriaErrores) {
        suma += error;
    }

    float promedio = suma / memoriaErrores.size();
    float precisionJugador = 1.0f - std::min(promedio / 160.0f, 1.0f);
    float racha = std::min(aciertosJugador * 0.12f, 0.36f);
    return std::min(precisionJugador + racha, 1.0f);
}

EstadoDron DronVigilante::getEstado() const
{
    return estado;
}
