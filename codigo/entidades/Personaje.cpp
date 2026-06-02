#include "Personaje.h"
#include "../render/SpriteCache.h"

Personaje::Personaje()
    : Entidad(120.0f, 300.0f, 34.0f, 68.0f)
{
    vx = 0.0f;
    vy = 0.0f;
    ax = 0.0f;
    ay = 0.0f;

    // AQUÍ van estos valores
    masa = 1.0f;

    enAire = false;
    izquierda = false;
    derecha = false;
    impulsoActivo = false;

    energiaMaxima = 100.0f;
    energia = energiaMaxima;

    spriteIdle.load(":/recursos/sprites/personaje_idle.png");
    spriteSalto.load(":/recursos/sprites/personaje_salto.png");
    spriteCaida.load(":/recursos/sprites/personaje_caida.png");
    spriteIzquierda.load(":/recursos/sprites/personaje_lateral_izq.png");
    spriteDerecha.load(":/recursos/sprites/personaje_lateral_der.png");
    spriteImpulso.load(":/recursos/sprites/personaje_impulso.png");
    spriteEntradaAgua.load(":/recursos/sprites/personaje_entrada_agua.png");
    spriteSplash.load(":/recursos/sprites/personaje_splash.png");
}

void Personaje::actualizar(float dt)
{
    if (izquierda) {
        ax -= 850.0f;
    }

    if (derecha) {
        ax += 850.0f;
    }

    if (enAire) {
        aplicarImpulsoElectromagnetico(dt);

        vx += ax * dt;
        vy += ay * dt;

        x += vx * dt;
        y += vy * dt;

        aplicarFriccionAire(0.995f);
        aplicarLimites(800.0f);
    }
    else {
        vx = 0.0f;
        vy = 0.0f;

        if (energia < energiaMaxima) {
            energia += 30.0f * dt;

            if (energia > energiaMaxima) {
                energia = energiaMaxima;
            }
        }
    }

    // Se reinician al final, no al principio.
    ax = 0.0f;
    ay = 0.0f;
}

void Personaje::dibujar(QPainter& painter)
{
    QPixmap spriteActual;

    if (impulsoActivo && energia > 0.0f && enAire && !spriteImpulso.isNull()) {
        spriteActual = spriteImpulso;
    }
    else if (enAire && izquierda && !spriteIzquierda.isNull()) {
        spriteActual = spriteIzquierda;
    }
    else if (enAire && derecha && !spriteDerecha.isNull()) {
        spriteActual = spriteDerecha;
    }
    else if (enAire && vy < 0.0f && !spriteSalto.isNull()) {
        spriteActual = spriteSalto;
    }
    else if (enAire && vy >= 0.0f && !spriteCaida.isNull()) {
        spriteActual = spriteCaida;
    }
    else if (!spriteIdle.isNull()) {
        spriteActual = spriteIdle;
    }

    QRectF hitbox = rect();

    if (!spriteActual.isNull()) {
        int spriteW = 58;
        int spriteH = 76;

        QRect destino(
            static_cast<int>(x + ancho / 2.0f - spriteW / 2.0f),
            static_cast<int>(y + alto - spriteH),
            spriteW,
            spriteH
            );

        SpriteCache::dibujarAjustado(painter, spriteActual, destino, "personaje");
    }
    else {
        painter.setPen(Qt::black);
        painter.setBrush(QBrush(Qt::red));
        painter.drawRect(hitbox);

        painter.setBrush(QBrush(Qt::blue));
        painter.drawRect(QRectF(x + 8, y + 15, ancho - 16, alto - 15));
    }

    if (impulsoActivo && energia > 0.0f && enAire) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 230, 80, 80));
        painter.drawEllipse(QRectF(x - 8, y - 5, ancho + 16, alto + 10));
        painter.setPen(Qt::black);
    }

}
void Personaje::saltar()
{
    if (!enAire) {
        enAire = true;

        vx = 120.0f;
        vy = -380.0f;
    }
}

void Personaje::moverIzquierda(bool estado)
{
    izquierda = estado;
}

void Personaje::moverDerecha(bool estado)
{
    derecha = estado;
}

void Personaje::activarImpulso(bool estado)
{
    impulsoActivo = estado;
}

void Personaje::aplicarGravedad(float gravedad)
{
    if (enAire) {
        ay += gravedad;
    }
}

void Personaje::aplicarViento(float aceleracionViento)
{
    if (enAire) {
        ax += aceleracionViento / masa;
    }
}

void Personaje::aplicarFriccionAire(float factor)
{
    vx *= factor;
}

void Personaje::aplicarImpulsoElectromagnetico(float dt)
{
    if (impulsoActivo && energia > 0.0f && enAire) {
        vy -= 360.0f * dt;
        vx *= 0.985f;

        energia -= 55.0f * dt;

        if (energia < 0.0f) {
            energia = 0.0f;
        }
    }
}

void Personaje::aplicarLimites(float anchoPantalla)
{
    if (x < 0.0f) {
        x = 0.0f;
        vx = 0.0f;
    }

    if (x + ancho > anchoPantalla) {
        x = anchoPantalla - ancho;
        vx = 0.0f;
    }
}

void Personaje::detenerMovimiento()
{
    vx = 0.0f;
    vy = 0.0f;
    ax = 0.0f;
    ay = 0.0f;
    impulsoActivo = false;
}

void Personaje::colocarEn(float nuevoX, float nuevoY)
{
    x = nuevoX;
    y = nuevoY;
}

float Personaje::getX() const
{
    return x;
}

float Personaje::getY() const
{
    return y;
}

float Personaje::getVX() const
{
    return vx;
}

float Personaje::getVY() const
{
    return vy;
}

float Personaje::getAncho() const
{
    return ancho;
}

float Personaje::getAlto() const
{
    return alto;
}

float Personaje::getEnergia() const
{
    return energia;
}

float Personaje::getEnergiaMaxima() const
{
    return energiaMaxima;
}

void Personaje::setVX(float nuevoVX)
{
    vx = nuevoVX;
}

void Personaje::setVY(float nuevoVY)
{
    vy = nuevoVY;
}

void Personaje::setEnAire(bool estado)
{
    enAire = estado;
}

void Personaje::setEnergiaMaxima(float nuevaEnergia)
{
    energiaMaxima = nuevaEnergia;
    energia = energiaMaxima;
}

bool Personaje::estaEnAire() const
{
    return enAire;
}

bool Personaje::estaUsandoImpulso() const
{
    return impulsoActivo;
}
