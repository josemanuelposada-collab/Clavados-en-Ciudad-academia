#include "Moneda.h"
#include "../render/SpriteCache.h"
#include <cmath>

Moneda::Moneda(float xInicial, float yInicial)
    : Entidad(xInicial, yInicial, 34.0f, 34.0f),
      recolectada(false),
      tiempo(0.0f),
      vx(0.0f),
      vy(0.0f)
{
    sprite.load(":/recursos/sprites/anillo_brillante.png");
}

void Moneda::actualizar(float dt)
{
    tiempo += dt;
    vx *= 0.985f;
    vy *= 0.985f;
    x += vx * dt;
    y += vy * dt;
    y += std::sin(tiempo * 4.5f) * 0.08f;
}

void Moneda::dibujar(QPainter& painter)
{
    if (recolectada) {
        return;
    }

    QRect area = rect().toRect();
    if (!sprite.isNull()) {
        SpriteCache::dibujarAjustado(painter, sprite, area, "moneda");
    }
    else {
        painter.setPen(QColor(230, 240, 245));
        painter.setBrush(QColor(255, 220, 80));
        painter.drawEllipse(rect());
    }
}

void Moneda::atraerHacia(const QPointF& objetivo, float intensidad, float radio, float dt)
{
    if (recolectada) {
        return;
    }

    float dx = objetivo.x() - centro().x();
    float dy = objetivo.y() - centro().y();
    float distancia = std::sqrt(dx * dx + dy * dy);
    if (distancia < 1.0f) {
        return;
    }

    float influencia = std::max(0.0f, 1.0f - distancia / radio);
    float aceleracion = intensidad * (0.35f + influencia * influencia * 2.6f);
    vx += dx / distancia * aceleracion * dt;
    vy += dy / distancia * aceleracion * dt;

    float rapidez = std::sqrt(vx * vx + vy * vy);
    if (rapidez > 760.0f) {
        vx = vx / rapidez * 760.0f;
        vy = vy / rapidez * 760.0f;
    }
}

void Moneda::recolectar()
{
    recolectada = true;
}

bool Moneda::estaRecolectada() const
{
    return recolectada;
}
