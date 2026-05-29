#include "Obstaculo.h"
#include <cmath>

Obstaculo::Obstaculo(float xInicial, float yInicial, float anchoInicial, float altoInicial,
                     float vxInicial, float vyInicial, TipoObstaculo tipoInicial)
    : Entidad(xInicial, yInicial, anchoInicial, altoInicial),
      vx(vxInicial),
      vy(vyInicial),
      tipo(tipoInicial)
{
    if (tipo == BOYA) {
        sprite.load(":/recursos/sprites/obstaculo_boya.png");
    }
    else if (tipo == BARRIL) {
        sprite.load(":/recursos/sprites/obstaculo_barril.png");
    }
    else if (tipo == MINA) {
        sprite.load(":/recursos/sprites/obstaculo_mina.png");
    }
    else {
        sprite.load(":/recursos/sprites/bloqueo.png");
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

    if (!sprite.isNull()) {
        painter.drawPixmap(area, sprite.scaled(area.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
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
