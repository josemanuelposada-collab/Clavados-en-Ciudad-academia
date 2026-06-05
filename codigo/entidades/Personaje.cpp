#include "Personaje.h"
#include "../fisicas/ModelosFisicos.h"
#include "../logica/JuegoException.h"
#include "../render/SpriteCache.h"
#include <algorithm>

Personaje::Personaje()
    : Entidad(120.0f, 300.0f, 34.0f, 68.0f)
{
    vx = 0.0f;
    vy = 0.0f;
    ax = 0.0f;
    ay = 0.0f;

    enAire = false;
    izquierda = false;
    derecha = false;
    impulsoActivo = false;
    tiempoAnimacion = 0.0f;

    configurarTipo(PERSONAJE_MIKOTO);
}

void Personaje::cargarSprites()
{
    spritesIdle.clear();
    spritesSalto.clear();
    spritesCaida.clear();
    spritesIzquierda.clear();
    spritesDerecha.clear();
    spritesImpulso.clear();
    spritesSplash.clear();

    QString carpeta = ":/recursos/sprites/personajes/mikoto_misaka";
    QString prefijo = "mikoto_misaka";
    QString spriteCaidaQuieta = "06_mikoto_misaka_correccion_lateral_aire_01_izquierda.png";

    if (tipo == PERSONAJE_ACCELERATOR) {
        carpeta = ":/recursos/sprites/personajes/accelerator";
        prefijo = "accelerator";
        spriteCaidaQuieta = "14_accelerator_clavado_inverso_01.png";
    }
    else if (tipo == PERSONAJE_MUGINO) {
        carpeta = ":/recursos/sprites/personajes/mugino";
        prefijo = "mugino";
        spriteCaidaQuieta = "14_mugino_clavado_inverso_01.png";
    }
    else if (tipo == PERSONAJE_DARK_MATTER) {
        carpeta = ":/recursos/sprites/personajes/dark_matter";
        prefijo = "dark_matter";
        spriteCaidaQuieta = "16_dark_matter_clavado_especial_01.png";
    }

    cargarSecuencia(spritesIdle, carpeta, {
        "01_" + prefijo + "_salto_desde_altura_01_parado.png"
    });
    cargarSecuencia(spritesSalto, carpeta, {
        "02_" + prefijo + "_salto_desde_altura_02_salto.png"
    });
    cargarSecuencia(spritesCaida, carpeta, {
        spriteCaidaQuieta
    });
    cargarSecuencia(spritesIzquierda, carpeta, {
        "19_" + prefijo + "_clavado_espiral_02.png"
    });
    cargarSecuencia(spritesDerecha, carpeta, {
        "20_" + prefijo + "_clavado_espiral_03.png"
    });
    cargarSecuencia(spritesImpulso, carpeta, {
        "11_" + prefijo + "_voltereta_tuck_01.png",
        "12_" + prefijo + "_voltereta_tuck_02.png",
        "13_" + prefijo + "_voltereta_tuck_03.png",
        "14_" + prefijo + "_clavado_inverso_01.png",
        "15_" + prefijo + "_clavado_inverso_02.png",
        "16_" + prefijo + "_clavado_especial_01.png",
        "17_" + prefijo + "_clavado_especial_02.png",
        "18_" + prefijo + "_clavado_espiral_01.png",
        "19_" + prefijo + "_clavado_espiral_02.png",
        "20_" + prefijo + "_clavado_espiral_03.png",
        "21_" + prefijo + "_clavado_espiral_04.png"
    });
    cargarSecuencia(spritesSplash, carpeta, {
        "22_" + prefijo + "_entrada_profunda_splash_01.png",
        "23_" + prefijo + "_entrada_profunda_splash_02.png",
        "24_" + prefijo + "_entrada_profunda_splash_03.png",
        "25_" + prefijo + "_entrada_profunda_splash_04.png",
        "26_" + prefijo + "_emerger_del_agua_01.png",
        "27_" + prefijo + "_emerger_del_agua_02.png",
        "28_" + prefijo + "_emerger_del_agua_03.png",
        "29_" + prefijo + "_emerger_del_agua_04.png",
        "30_" + prefijo + "_emerger_del_agua_05.png"
    });

    if (spritesIdle.isEmpty() || spritesSalto.isEmpty() || spritesCaida.isEmpty()) {
        throw JuegoException("No se pudieron cargar los sprites basicos de " + getNombre() + ".");
    }

    if (spritesIzquierda.isEmpty()) {
        spritesIzquierda = spritesCaida;
    }
    if (spritesDerecha.isEmpty()) {
        spritesDerecha = spritesCaida;
    }
    if (spritesSplash.isEmpty()) {
        spritesSplash = spritesCaida;
    }
}

void Personaje::cargarSecuencia(QVector<QPixmap>& destino, const QString& carpeta, const QStringList& archivos)
{
    destino.clear();
    destino.reserve(archivos.size());

    for (const QString& archivo : archivos) {
        const QString ruta = carpeta + "/" + archivo;
        const QPixmap& sprite = SpriteCache::obtener(ruta);
        if (!sprite.isNull()) {
            destino.push_back(sprite);
        }
    }
}

const QVector<QPixmap>& Personaje::secuenciaActual() const
{
    if (enAire && izquierda && !spritesIzquierda.isEmpty()) {
        return spritesIzquierda;
    }
    if (enAire && derecha && !spritesDerecha.isEmpty()) {
        return spritesDerecha;
    }
    if (enAire && impulsoActivo && energia > 0.0f && !spritesImpulso.isEmpty()) {
        return spritesImpulso;
    }
    if (enAire && vy < 0.0f && !spritesSalto.isEmpty()) {
        return spritesSalto;
    }
    if (enAire && vy >= 0.0f && !spritesCaida.isEmpty()) {
        return spritesCaida;
    }
    return spritesIdle;
}

void Personaje::actualizar(float dt)
{
    tiempoAnimacion += dt;

    if (izquierda) {
        ax -= 850.0f * controlLateral;
    }

    if (derecha) {
        ax += 850.0f * controlLateral;
    }

    if (enAire) {
        aplicarImpulsoElectromagnetico(dt);

        FisicaJuego::integrarVelocidad(vx, ax, dt);
        FisicaJuego::integrarVelocidad(vy, ay, dt);

        FisicaJuego::integrarPosicion(x, vx, dt);
        FisicaJuego::integrarPosicion(y, vy, dt);

        aplicarFriccionAire(factorArrastre);
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

    ax = 0.0f;
    ay = 0.0f;
}

void Personaje::dibujar(QPainter& painter)
{
    const QVector<QPixmap>& secuencia = secuenciaActual();
    const QPixmap* spriteActual = nullptr;

    if (!secuencia.isEmpty()) {
        const int indice = secuencia.size() > 1
                               ? static_cast<int>(tiempoAnimacion * 9.0f) % secuencia.size()
                               : 0;
        spriteActual = &secuencia[indice];
    }

    if (spriteActual != nullptr && !spriteActual->isNull()) {
        int spriteH = 84;
        int spriteW = spriteActual->height() > 0 ? spriteActual->width() * spriteH / spriteActual->height() : 58;
        spriteW = std::clamp(spriteW, 42, 118);

        QRect destino(
            static_cast<int>(x + ancho / 2.0f - spriteW / 2.0f),
            static_cast<int>(y + alto - spriteH),
            spriteW,
            spriteH
            );

        if (enAire && (izquierda || derecha)) {
            painter.save();
            painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
            QPoint centro = destino.center();
            painter.translate(centro);
            painter.rotate(izquierda ? -7.0 : 7.0);
            painter.drawPixmap(QRect(-destino.width() / 2, -destino.height() / 2, destino.width(), destino.height()), *spriteActual);
            painter.restore();
        }
        else {
            SpriteCache::dibujarAjustado(painter, *spriteActual, destino, "personaje_" + getNombre());
        }
    }
    else {
        painter.setPen(Qt::black);
        painter.setBrush(QBrush(Qt::red));
        painter.drawRect(rect());

        painter.setBrush(QBrush(Qt::blue));
        painter.drawRect(QRectF(x + 8, y + 15, ancho - 16, alto - 15));
    }

    if (impulsoActivo && energia > 0.0f && enAire) {
        QColor color = colorPoder;
        color.setAlpha(82);
        painter.setPen(Qt::NoPen);
        painter.setBrush(color);
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
        ay += gravedad * factorGravedad;
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
        energia -= (tipo == PERSONAJE_MIKOTO ? 30.0f : 42.0f) * dt;

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

void Personaje::configurarTipo(TipoPersonaje nuevoTipo)
{
    tipo = nuevoTipo;

    if (tipo == PERSONAJE_ACCELERATOR) {
        masa = 0.92f;
        energiaMaxima = 92.0f;
        controlLateral = 1.16f;
        factorGravedad = 0.96f;
        factorArrastre = 0.992f;
        radioPoder = 152.0f;
        atraccionMonedas = 520.0f;
        colorPoder = QColor(230, 245, 255);
    }
    else if (tipo == PERSONAJE_MUGINO) {
        masa = 1.08f;
        energiaMaxima = 112.0f;
        controlLateral = 0.94f;
        factorGravedad = 1.03f;
        factorArrastre = 0.996f;
        radioPoder = 146.0f;
        atraccionMonedas = 560.0f;
        colorPoder = QColor(80, 240, 110);
    }
    else if (tipo == PERSONAJE_DARK_MATTER) {
        masa = 0.86f;
        energiaMaxima = 104.0f;
        controlLateral = 1.02f;
        factorGravedad = 0.88f;
        factorArrastre = 0.986f;
        radioPoder = 166.0f;
        atraccionMonedas = 610.0f;
        colorPoder = QColor(184, 116, 255);
    }
    else {
        masa = 1.0f;
        energiaMaxima = 100.0f;
        controlLateral = 1.0f;
        factorGravedad = 1.0f;
        factorArrastre = 0.995f;
        radioPoder = 178.0f;
        atraccionMonedas = 760.0f;
        colorPoder = QColor(255, 225, 95);
    }

    energia = energiaMaxima;
    impulsoActivo = false;
    cargarSprites();
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

float Personaje::getMasa() const
{
    return masa;
}

float Personaje::getControlLateral() const
{
    return controlLateral;
}

float Personaje::getFactorGravedad() const
{
    return factorGravedad;
}

float Personaje::getFactorArrastre() const
{
    return factorArrastre;
}

float Personaje::getRadioPoder() const
{
    return radioPoder;
}

float Personaje::getAtraccionMonedas() const
{
    return atraccionMonedas;
}

QColor Personaje::getColorPoder() const
{
    return colorPoder;
}

QString Personaje::getNombre() const
{
    if (tipo == PERSONAJE_ACCELERATOR) {
        return "Accelerator";
    }
    if (tipo == PERSONAJE_MUGINO) {
        return "Mugino";
    }
    if (tipo == PERSONAJE_DARK_MATTER) {
        return "Dark Matter";
    }
    return "Mikoto";
}

QString Personaje::getPoder() const
{
    if (tipo == PERSONAJE_ACCELERATOR) {
        return "Control vectorial";
    }
    if (tipo == PERSONAJE_MUGINO) {
        return "Meltdowner lateral";
    }
    if (tipo == PERSONAJE_DARK_MATTER) {
        return "Materia oscura orbital";
    }
    return "Campo electromagnetico";
}

TipoPersonaje Personaje::getTipo() const
{
    return tipo;
}

QRectF Personaje::hitboxAjustada() const
{
    return rect().adjusted(7.0f, 8.0f, -7.0f, -4.0f);
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

bool Personaje::estaCorrigiendoLateral() const
{
    return enAire && (izquierda || derecha);
}
