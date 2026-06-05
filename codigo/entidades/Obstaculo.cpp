#include "Obstaculo.h"
#include "../render/SpriteCache.h"
#include <cmath>

Obstaculo::Obstaculo(float xInicial, float yInicial, float anchoInicial, float altoInicial,
                     float vxInicial, float vyInicial, TipoObstaculo tipoInicial)
    : Entidad(xInicial, yInicial, anchoInicial, altoInicial),
      vx(vxInicial),
      vy(vyInicial),
      tipo(tipoInicial)
{
    if (tipo == BOYA) {
        rutaSprite = ":/recursos/sprites/obstaculo_boya.png";
    }
    else if (tipo == BARRIL) {
        rutaSprite = ":/recursos/sprites/obstaculo_barril.png";
    }
    else if (tipo == MINA) {
        rutaSprite = ":/recursos/sprites/obstaculo_mina.png";
    }
    else {
        rutaSprite = ":/recursos/sprites/bloqueo.png";
    }
}

void Obstaculo::actualizar(float dt)
{
    x += vx * dt;
    y += vy * dt;
}

void Obstaculo::dibujar(QPainter& painter)
{
    QRect area = rect().toRect();
    const QPixmap& sprite = SpriteCache::obtener(rutaSprite);

    if (!sprite.isNull()) {
        SpriteCache::dibujarAjustado(painter, sprite, area, "obstaculo");
    }
    else {
        painter.setPen(Qt::black);
        painter.setBrush(QColor(180, 60, 60));
        painter.drawEllipse(rect());
    }
}

void Obstaculo::rebotarHorizontal(float limiteIzquierdo, float limiteDerecho)
{
    if (x < limiteIzquierdo) {
        x = limiteIzquierdo;
        vx = std::abs(vx);
    }

    if (x + ancho > limiteDerecho) {
        x = limiteDerecho - ancho;
        vx = -std::abs(vx);
    }
}

TipoObstaculo Obstaculo::getTipo() const
{
    return tipo;
}
