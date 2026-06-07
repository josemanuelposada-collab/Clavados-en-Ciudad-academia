#include "Moneda.h"
#include "../fisicas/ModelosFisicos.h"
#include "../render/SpriteCache.h"
#include <algorithm>
#include <cmath>

Moneda::Moneda(float xInicial, float yInicial)
    : Entidad(xInicial, yInicial, 34.0f, 34.0f),
      recolectada(false),
      tiempo(0.0f),
      vx(0.0f),
      vy(0.0f),
      rutaSprite(":/recursos/sprites/anillo_brillante.png")
{
}

void Moneda::actualizar(float dt)
{
    if (recolectada) {
        return;
    }

    tiempo += dt;
    vx *= 0.985f;
    vy *= 0.985f;
    FisicaJuego::integrarPosicion(x, vx, dt);
    FisicaJuego::integrarPosicion(y, vy, dt);
    y += std::sin(tiempo * 4.5f) * 0.08f;
}

void Moneda::dibujar(QPainter& painter)
{
    if (recolectada) {
        return;
    }

    QRect area = rect().toRect();
    const QPixmap& sprite = SpriteCache::obtener(rutaSprite);
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
    float distancia = std::sqrt(FisicaJuego::distanciaCuadrada(dx, dy));
    if (distancia < 1.0f) {
        return;
    }

    float influencia = std::max(0.0f, 1.0f - distancia / radio);
    float aceleracion = intensidad * (0.35f + influencia * influencia * 2.6f);
    vx += dx / distancia * aceleracion * dt;
    vy += dy / distancia * aceleracion * dt;

    FisicaJuego::limitarVector(vx, vy, 760.0f);
}

void Moneda::atraerRectilineoHacia(const QPointF& objetivo, float rapidez, float radio, float dt)
{
    if (recolectada) {
        return;
    }

    const float dx = objetivo.x() - centro().x();
    const float dy = objetivo.y() - centro().y();
    const float distancia2 = FisicaJuego::distanciaCuadrada(dx, dy);
    if (distancia2 < 1.0f || distancia2 > radio * radio) {
        return;
    }

    const float distancia = std::sqrt(distancia2);
    const float direccionX = dx / distancia;
    const float direccionY = dy / distancia;
    const float paso = std::min(distancia, rapidez * dt);

    x += direccionX * paso;
    y += direccionY * paso;
    vx = direccionX * rapidez;
    vy = direccionY * rapidez;
}

void Moneda::desplazarVectorialHacia(const QPointF& objetivo, float intensidad, float radio, float dt)
{
    if (recolectada) {
        return;
    }

    float dx = objetivo.x() - centro().x();
    float dy = objetivo.y() - centro().y();
    float distancia = std::sqrt(FisicaJuego::distanciaCuadrada(dx, dy));
    if (distancia < 1.0f || distancia > radio) {
        return;
    }

    float direccionX = dx / distancia;
    float direccionY = dy / distancia;
    float peso = std::max(0.18f, 1.0f - distancia / radio);
    float velocidadObjetivo = intensidad * (0.65f + peso * 0.95f);

    vx = vx * 0.72f + direccionX * velocidadObjetivo * 0.28f;
    vy = vy * 0.72f + direccionY * velocidadObjetivo * 0.28f;

    FisicaJuego::limitarVector(vx, vy, 880.0f);

    FisicaJuego::integrarPosicion(x, direccionX * 34.0f * peso, dt);
    FisicaJuego::integrarPosicion(y, direccionY * 34.0f * peso, dt);
}

void Moneda::canalizarMeltdowner(const QPointF& origen, float intensidad, float alcanceVertical, float dt)
{
    if (recolectada) {
        return;
    }

    float dx = origen.x() - centro().x();
    float dy = origen.y() - centro().y();
    float distanciaVertical = std::abs(dy);
    if (distanciaVertical > alcanceVertical) {
        return;
    }

    float alineacion = 1.0f - distanciaVertical / alcanceVertical;
    float direccionX = dx >= 0.0f ? 1.0f : -1.0f;
    vx += direccionX * intensidad * (0.55f + alineacion) * dt;
    vy += dy * 7.5f * alineacion * dt;

    vx = std::clamp(vx, -820.0f, 820.0f);
    vy = std::clamp(vy, -520.0f, 520.0f);
}

void Moneda::orbitarHacia(const QPointF& objetivo, float intensidad, float radio, float dt)
{
    if (recolectada) {
        return;
    }

    float dx = objetivo.x() - centro().x();
    float dy = objetivo.y() - centro().y();
    float distancia = std::sqrt(FisicaJuego::distanciaCuadrada(dx, dy));
    if (distancia < 1.0f || distancia > radio) {
        return;
    }

    float nx = dx / distancia;
    float ny = dy / distancia;
    float influencia = 1.0f - distancia / radio;
    float radial = intensidad * (0.30f + influencia * 1.35f);
    float tangencial = intensidad * (0.34f + influencia * 0.70f);

    vx += (nx * radial - ny * tangencial) * dt;
    vy += (ny * radial + nx * tangencial) * dt;

    FisicaJuego::limitarVector(vx, vy, 700.0f);
}

void Moneda::recolectar()
{
    recolectada = true;
}

bool Moneda::estaRecolectada() const
{
    return recolectada;
}
