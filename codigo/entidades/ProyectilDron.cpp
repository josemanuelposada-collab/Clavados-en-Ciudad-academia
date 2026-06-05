#include "ProyectilDron.h"
#include "../fisicas/ModelosFisicos.h"
#include <QPainter>

ProyectilDron::ProyectilDron(float xInicial, float yInicial, float vxInicial, float vyInicial, bool colisionElastica)
    : Entidad(xInicial, yInicial, 24.0f, 24.0f),
      vx(vxInicial),
      vy(vyInicial),
      masa(colisionElastica ? 0.62f : 1.35f),
      tiempoVida(0.0f),
      activo(true),
      elastico(colisionElastica),
      color(colisionElastica ? QColor(110, 220, 255) : QColor(255, 185, 70))
{
}

void ProyectilDron::actualizar(float dt)
{
    if (!activo) {
        return;
    }

    tiempoVida += dt;
    FisicaJuego::integrarVelocidad(vy, 18.0f, dt);
    FisicaJuego::integrarPosicion(x, vx, dt);
    FisicaJuego::integrarPosicion(y, vy, dt);

    if (tiempoVida > 5.0f || x < -80.0f || x > 880.0f || y < -120.0f || y > 6400.0f) {
        activo = false;
    }
}

void ProyectilDron::dibujar(QPainter& painter)
{
    if (!activo) {
        return;
    }

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    QColor halo = color;
    halo.setAlpha(48);
    painter.setPen(Qt::NoPen);
    painter.setBrush(halo);
    painter.drawEllipse(rect().adjusted(-7.0f, -7.0f, 7.0f, 7.0f));

    painter.setBrush(color);
    painter.drawEllipse(rect());
    painter.setPen(QPen(QColor(255, 255, 255, 190), 2));
    painter.drawLine(QPointF(x + 7.0f, y + 7.0f), QPointF(x + ancho - 7.0f, y + alto - 7.0f));
    painter.restore();
}

bool ProyectilDron::estaActivo() const
{
    return activo;
}

bool ProyectilDron::usaColisionElastica() const
{
    return elastico;
}

void ProyectilDron::reiniciar(float nuevoX, float nuevoY, float nuevoVX, float nuevoVY, bool colisionElastica)
{
    x = nuevoX;
    y = nuevoY;
    vx = nuevoVX;
    vy = nuevoVY;
    elastico = colisionElastica;
    masa = elastico ? 0.62f : 1.35f;
    tiempoVida = 0.0f;
    activo = true;
    color = elastico ? QColor(110, 220, 255) : QColor(255, 185, 70);
}

void ProyectilDron::desactivar()
{
    activo = false;
}

float ProyectilDron::getVX() const
{
    return vx;
}

float ProyectilDron::getVY() const
{
    return vy;
}

float ProyectilDron::getMasa() const
{
    return masa;
}

void ProyectilDron::setVX(float nuevoVX)
{
    vx = nuevoVX;
}

void ProyectilDron::setVY(float nuevoVY)
{
    vy = nuevoVY;
}
