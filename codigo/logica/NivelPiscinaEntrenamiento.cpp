#include "NivelPiscinaEntrenamiento.h"
#include "JuegoException.h"
#include "../render/SpriteCache.h"
#include <QLinearGradient>
#include <algorithm>
#include <cmath>

namespace {
constexpr float ANCHO_NIVEL = 800.0f;
constexpr float ALTO_NIVEL = 600.0f;
constexpr float X_PERSONAJE_PREPARACION = 0.5f;
constexpr float PI = 3.1415926535f;

void dibujarPanel(QPainter& painter, const QRectF& rect, const QColor& color)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRect(rect);
    painter.setBrush(QColor(255, 225, 95, 180));
    painter.drawRect(QRectF(rect.x(), rect.y(), 5, rect.height()));
    painter.setPen(QPen(QColor(120, 230, 255, 80), 1));
    painter.drawLine(rect.topLeft(), rect.topRight());
    painter.drawLine(rect.bottomLeft(), rect.bottomRight());
}

QColor colorPotencia(float porcentaje)
{
    porcentaje = std::clamp(porcentaje, 0.0f, 1.0f);
    if (porcentaje < 0.34f) {
        return QColor(235, 80, 76);
    }
    if (porcentaje < 0.68f) {
        return QColor(255, 220, 80);
    }
    return QColor(80, 224, 150);
}

void exigirSprite(const QPixmap& sprite, const QString& nombre)
{
    if (sprite.isNull()) {
        throw JuegoException("No se pudo cargar el recurso grafico obligatorio: " + nombre);
    }
}

void dibujarPisoAlrededorPiscina(QPainter& painter,
                                 const QPixmap& textura,
                                 const QRectF& exterior,
                                 const QRectF& huecoPiscina)
{
    if (textura.isNull()) {
        return;
    }

    painter.save();
    painter.setOpacity(0.82);
    QBrush mosaico(textura);
    painter.fillRect(QRectF(exterior.left(), exterior.top(), exterior.width(), huecoPiscina.top() - exterior.top()), mosaico);
    painter.fillRect(QRectF(exterior.left(), huecoPiscina.bottom(), exterior.width(), exterior.bottom() - huecoPiscina.bottom()), mosaico);
    painter.fillRect(QRectF(exterior.left(), huecoPiscina.top(), huecoPiscina.left() - exterior.left(), huecoPiscina.height()), mosaico);
    painter.fillRect(QRectF(huecoPiscina.right(), huecoPiscina.top(), exterior.right() - huecoPiscina.right(), huecoPiscina.height()), mosaico);
    painter.setOpacity(1.0);
    painter.setPen(QPen(QColor(255, 255, 255, 120), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(exterior.adjusted(1.0, 1.0, -1.0, -1.0));
    painter.restore();
}

float acercar(float valor, float objetivo, float paso)
{
    if (valor < objetivo) {
        return std::min(valor + paso, objetivo);
    }
    return std::max(valor - paso, objetivo);
}
}

NivelPiscinaEntrenamiento::NivelPiscinaEntrenamiento()
    : jugador(std::make_unique<Personaje>()),
      plataforma(std::make_unique<Plataforma>())
{
    piscina = QRectF(555.0f, 430.0f, 185.0f, 88.0f);
    zonaPerfecta = QRectF(0.0f, 0.0f, 92.0f, 96.0f);
    zonaViento = QRectF(300.0f, 190.0f, 190.0f, 238.0f);
    suelo = QRectF(0.0f, 520.0f, ANCHO_NIVEL, 80.0f);

    spritePiscinaBase.load(":/recursos/sprites/piscina_ciudad_academia.png");
    spritePiscinaPremium.load(":/recursos/sprites/piscina_final_premium.png");
    spriteViento.load(":/recursos/sprites/viento_lateral.png");
    spriteVientoDerecha.load(":/recursos/sprites/velocidad_derecha.png");
    spriteVientoIzquierda.load(":/recursos/sprites/velocidad_izquierda.png");
    spriteFondoCiudad.load(":/recursos/sprites/fondo_vertical_entrenamiento.png");
    spriteChapuzonLimpio.load(":/recursos/sprites/chapuzon_limpio.png");
    spriteChapuzonMedio.load(":/recursos/sprites/chapuzon_medio.png");
    spriteChapuzonFuerte.load(":/recursos/sprites/chapuzon_fuerte.png");
    spriteBurbujas.load(":/recursos/sprites/burbujas_entrada.png");
    spriteTexturaPixel.load(":/recursos/sprites/textura_pixel_overlay.png");
    spriteAlrededorPiscina.load(":/recursos/sprites/alrededor_piscina.jpg");
    spriteCorazonLleno.load(":/recursos/sprites/hud_corazon_lleno.png");
    spriteCorazonVacio.load(":/recursos/sprites/hud_corazon_vacio.png");
    spriteAdvertenciaHud.load(":/recursos/sprites/hud_advertencia.png");

    exigirSprite(spritePiscinaPremium, "piscina_final_premium.png");
    exigirSprite(spriteCorazonLleno, "hud_corazon_lleno.png");
    exigirSprite(spriteCorazonVacio, "hud_corazon_vacio.png");

    intentoActual = 1;
    intentosMaximos = 5;
    puntajeTotal = 0;
    puntajeObjetivo = 300;
    puntajeUltimoIntento = 0;
    mejorPuntaje = 0;
    puntaje = 0;

    intentoEnCurso = false;
    esperandoSiguienteIntento = false;
    intentoEvaluado = false;
    nivelSuperado = false;
    nivelPerdido = false;
    jugadorEnZonaViento = false;
    frenandoCaida = false;
    acelerandoCaida = false;
    corrigiendoIzquierda = false;
    corrigiendoDerecha = false;
    usoImpulsoIntento = false;
    motivoDerrotaActual = "";

    tiempoNivel = 0.0f;
    tiempoIntento = 0.0f;
    tiempoEsperaReinicio = 1.5f;
    tiempoEsperaActual = 0.0f;
    tiempoIman = 0.0f;
    cooldownIman = 0.0f;
    radioIman = jugador->getRadioPoder();

    gravedad = 305.0f;
    vientoLateral = 0.0f;
    vientoRafaga = 0.0f;
    errorEntrada = 0.0f;
    velocidadEntrada = 0.0f;
    anguloPostura = 90.0f;
    tiempoCorreccionLateral = 0.0f;

    potenciaMinima = 165.0f;
    potenciaMaxima = 310.0f;
    potenciaActual = potenciaMinima;
    velocidadCambioPotencia = 118.0f;
    direccionPotencia = 1;

    xBasePlataforma = 72.0f;
    amplitudPlataforma = 36.0f;
    frecuenciaPlataforma = 1.4f;
    velocidadPlataforma = 0.0f;
    anchoZonaPerfecta = 92.0f;

    dificultad.configurar(NORMAL);
    aplicarParametrosDificultad();
    iniciarIntento();
}

NivelPiscinaEntrenamiento::~NivelPiscinaEntrenamiento() = default;

void NivelPiscinaEntrenamiento::aplicarParametrosDificultad()
{
    intentosMaximos = 5;
    if (dificultad.getTipo() == FACIL) {
        puntajeObjetivo = 260;
        potenciaMinima = 160.0f;
        potenciaMaxima = 300.0f;
        velocidadCambioPotencia = 96.0f;
        gravedad = 292.0f;
    }
    else if (dificultad.getTipo() == NORMAL) {
        puntajeObjetivo = 300;
        potenciaMinima = 165.0f;
        potenciaMaxima = 310.0f;
        velocidadCambioPotencia = 118.0f;
        gravedad = 305.0f;
    }
    else {
        puntajeObjetivo = 330;
        potenciaMinima = 170.0f;
        potenciaMaxima = 322.0f;
        velocidadCambioPotencia = 145.0f;
        gravedad = 318.0f;
    }
    potenciaActual = std::clamp(potenciaActual, potenciaMinima, potenciaMaxima);
    jugador->setEnergiaMaxima(dificultad.getEnergiaInicial());
    radioIman = jugador->getRadioPoder();
    configurarIntento();
}

void NivelPiscinaEntrenamiento::configurarIntento()
{
    const float dificultadFisica = dificultad.getTipo() == FACIL ? 0.78f : dificultad.getTipo() == NORMAL ? 1.0f : 1.22f;
    const float avance = static_cast<float>(std::max(0, intentoActual - 1));

    amplitudPlataforma = (22.0f + avance * 7.0f) * dificultadFisica;
    frecuenciaPlataforma = (1.05f + avance * 0.18f) * dificultadFisica;
    if (intentoActual >= 5) {
        frecuenciaPlataforma += 0.20f * dificultadFisica;
        amplitudPlataforma += 8.0f * dificultadFisica;
    }

    float anchoBase = dificultad.getTipo() == FACIL ? 102.0f : dificultad.getTipo() == NORMAL ? 84.0f : 72.0f;
    anchoZonaPerfecta = std::max(48.0f, anchoBase - avance * (intentoActual >= 4 ? 11.0f : 7.0f));

    zonaPerfecta.setSize(QSizeF(anchoZonaPerfecta, piscina.height()));
    actualizarZonaPerfecta();

    plataforma->configurarOscilacion(amplitudPlataforma, frecuenciaPlataforma);
    plataforma->colocarEn(xBasePlataforma, 218.0f);
    velocidadPlataforma = 0.0f;
}

void NivelPiscinaEntrenamiento::actualizarZonaPerfecta()
{
    const float centroX = piscina.center().x();
    zonaPerfecta.moveTo(centroX - anchoZonaPerfecta * 0.5f, piscina.y());
}

void NivelPiscinaEntrenamiento::iniciarIntento()
{
    configurarIntento();
    intentoEnCurso = true;
    esperandoSiguienteIntento = false;
    intentoEvaluado = false;
    jugadorEnZonaViento = false;
    frenandoCaida = false;
    acelerandoCaida = false;
    corrigiendoIzquierda = false;
    corrigiendoDerecha = false;
    usoImpulsoIntento = false;

    tiempoIntento = 0.0f;
    tiempoEsperaActual = 0.0f;
    tiempoIman = 0.0f;
    cooldownIman = 0.0f;
    errorEntrada = 0.0f;
    velocidadEntrada = 0.0f;
    anguloPostura = 90.0f;
    tiempoCorreccionLateral = 0.0f;
    puntaje = 0;

    jugador->setEnAire(false);
    jugador->setVX(0.0f);
    jugador->setVY(0.0f);
    jugador->activarImpulso(false);
    jugador->colocarEn(
        plataforma->getX() + plataforma->getAncho() * X_PERSONAJE_PREPARACION - jugador->getAncho() * 0.5f,
        plataforma->getY() - jugador->getAlto() + 8.0f);
}

void NivelPiscinaEntrenamiento::actualizar(float dt)
{
    tiempoNivel += dt;
    cooldownIman = std::max(0.0f, cooldownIman - dt);
    tiempoIman = std::max(0.0f, tiempoIman - dt);
    if (tiempoIman <= 0.0f) {
        jugador->activarImpulso(false);
    }

    if (nivelSuperado || nivelPerdido) {
        actualizarCamara();
        return;
    }

    if (esperandoSiguienteIntento) {
        tiempoEsperaActual += dt;
        if (tiempoEsperaActual >= tiempoEsperaReinicio) {
            intentoActual++;
            iniciarIntento();
        }
        actualizarCamara();
        return;
    }

    const float xAnteriorPlataforma = plataforma->getX();
    plataforma->actualizar(dt);
    velocidadPlataforma = dt > 0.0001f ? (plataforma->getX() - xAnteriorPlataforma) / dt : 0.0f;
    actualizarPotencia(dt);
    verificarZonaViento();

    if (!jugador->estaEnAire()) {
        jugador->colocarEn(
            plataforma->getX() + plataforma->getAncho() * X_PERSONAJE_PREPARACION - jugador->getAncho() * 0.5f,
            plataforma->getY() - jugador->getAlto() + 8.0f);
    }
    else {
        tiempoIntento += dt;
        jugador->aplicarGravedad(gravedad);

        if (frenandoCaida) {
            jugador->setVY(std::max(80.0f, jugador->getVY() - 150.0f * dt));
        }
        if (acelerandoCaida) {
            jugador->setVY(std::min(620.0f, jugador->getVY() + 145.0f * dt));
        }

        if (jugadorEnZonaViento) {
            jugador->aplicarViento(vientoLateral + vientoRafaga);
        }

        if (tiempoIman > 0.0f) {
            const float dx = static_cast<float>(zonaPerfecta.center().x() - jugador->centro().x());
            jugador->aplicarViento(std::clamp(dx * 1.5f, -95.0f, 95.0f));
        }

        if (jugador->estaCorrigiendoLateral()) {
            float cercaniaEntrada = std::clamp((jugador->getY() - 220.0f) / 245.0f, 0.15f, 1.0f);
            tiempoCorreccionLateral += dt * cercaniaEntrada;
        }

        actualizarPostura(dt);
        jugador->actualizar(dt);
        verificarPiscina();
        verificarSuelo();
    }

    actualizarCamara();
}

void NivelPiscinaEntrenamiento::actualizarPotencia(float dt)
{
    if (jugador->estaEnAire() || esperandoSiguienteIntento || nivelSuperado || nivelPerdido) {
        return;
    }

    potenciaActual += direccionPotencia * velocidadCambioPotencia * dt;
    if (potenciaActual >= potenciaMaxima) {
        potenciaActual = potenciaMaxima;
        direccionPotencia = -1;
    }
    else if (potenciaActual <= potenciaMinima) {
        potenciaActual = potenciaMinima;
        direccionPotencia = 1;
    }
}

void NivelPiscinaEntrenamiento::actualizarPostura(float dt)
{
    if (corrigiendoIzquierda) {
        anguloPostura -= 55.0f * dt;
    }
    if (corrigiendoDerecha) {
        anguloPostura += 55.0f * dt;
    }
    if (!corrigiendoIzquierda && !corrigiendoDerecha) {
        anguloPostura = acercar(anguloPostura, 90.0f, 18.0f * dt);
    }
    anguloPostura = std::clamp(anguloPostura, 55.0f, 125.0f);
}

void NivelPiscinaEntrenamiento::verificarZonaViento()
{
    jugadorEnZonaViento = false;
    vientoRafaga = 0.0f;

    if (!jugador->estaEnAire()) {
        return;
    }

    const bool ventanaViento = intentoActual >= 3 && tiempoIntento >= 1.05f && tiempoIntento <= 2.45f;
    if (ventanaViento && zonaViento.intersects(jugador->rect())) {
        jugadorEnZonaViento = true;
        float direccion = intentoActual % 2 == 0 ? -1.0f : 1.0f;
        vientoRafaga = direccion * (72.0f + intentoActual * 18.0f) *
                       (0.80f + 0.20f * std::sin(tiempoNivel * 4.2f));
        vientoLateral = dificultad.getIntensidadViento() * 0.70f * direccion;
    }
}

void NivelPiscinaEntrenamiento::verificarPiscina()
{
    if (!intentoEnCurso || intentoEvaluado) {
        return;
    }

    if (piscina.intersects(jugador->hitboxAjustada()) && jugador->getVY() > 0.0f) {
        calcularPuntajePorEntrada();
        finalizarIntento(puntajeUltimoIntento, true);
    }
}

void NivelPiscinaEntrenamiento::verificarSuelo()
{
    if (!intentoEnCurso || intentoEvaluado) {
        return;
    }

    QRectF hitbox = jugador->hitboxAjustada();
    bool pasoPiscinaSinEntrar = hitbox.top() > piscina.bottom() + 10.0f;
    bool tocaSuelo = hitbox.bottom() >= suelo.top();
    if ((pasoPiscinaSinEntrar || tocaSuelo) && !piscina.intersects(hitbox)) {
        errorEntrada = std::abs(static_cast<float>(jugador->centro().x() - zonaPerfecta.center().x()));
        velocidadEntrada = std::abs(jugador->getVY());
        motivoDerrotaActual = pasoPiscinaSinEntrar
                                   ? "Pasaste de largo sin entrar a la piscina."
                                   : "Tocaste el piso antes de completar el clavado.";
        finalizarIntento(0, false);
    }
}

void NivelPiscinaEntrenamiento::calcularPuntajePorEntrada()
{
    errorEntrada = std::abs(static_cast<float>(jugador->centro().x() - zonaPerfecta.center().x()));
    velocidadEntrada = std::abs(jugador->getVY());

    float puntosPosicion = std::clamp(100.0f - errorEntrada * (100.0f / (anchoZonaPerfecta * 1.45f)), 0.0f, 100.0f);
    float errorPostura = std::abs(anguloPostura - 90.0f);
    float puntosPostura = std::clamp(100.0f - errorPostura * 3.7f, 0.0f, 100.0f);
    float puntosVelocidad = std::clamp(100.0f - std::abs(velocidadEntrada - 455.0f) * 0.28f, 0.0f, 100.0f);

    float bonusEntradaLimpia = (errorEntrada < 14.0f && errorPostura < 7.0f && velocidadEntrada >= 380.0f && velocidadEntrada <= 540.0f) ? 8.0f : 0.0f;
    float penalizacionImpulso = usoImpulsoIntento ? 8.0f : 0.0f;
    float penalizacionCorreccion = std::clamp(tiempoCorreccionLateral * 9.0f, 0.0f, 16.0f);

    float total = puntosPosicion * 0.45f +
                  puntosPostura * 0.32f +
                  puntosVelocidad * 0.18f +
                  bonusEntradaLimpia -
                  penalizacionImpulso -
                  penalizacionCorreccion;

    puntajeUltimoIntento = static_cast<int>(std::clamp(std::round(total), 0.0f, 100.0f));
    puntaje = puntajeUltimoIntento;
}

void NivelPiscinaEntrenamiento::finalizarIntento(int puntajeIntento, bool entroAlAgua)
{
    intentoEvaluado = true;
    intentoEnCurso = false;
    puntajeUltimoIntento = std::clamp(puntajeIntento, 0, 100);
    puntaje = puntajeUltimoIntento;
    puntajeTotal = std::min(500, puntajeTotal + puntajeUltimoIntento);
    mejorPuntaje = std::max(mejorPuntaje, puntajeUltimoIntento);
    jugador->detenerMovimiento();

    if (entroAlAgua) {
        eventosSonido.push_back(SONIDO_AGUA);
    }
    else {
        eventosSonido.push_back(SONIDO_COLISION);
    }

    if (puntajeTotal >= puntajeObjetivo) {
        nivelSuperado = true;
        eventosSonido.push_back(SONIDO_NIVEL);
    }
    else if (intentoActual >= intentosMaximos) {
        if (motivoDerrotaActual.isEmpty()) {
            motivoDerrotaActual = "No alcanzaste el puntaje objetivo con los intentos disponibles.";
        }
        nivelPerdido = true;
    }
    else {
        esperandoSiguienteIntento = true;
        tiempoEsperaActual = 0.0f;
    }
}

void NivelPiscinaEntrenamiento::actualizarCamara()
{
    // Nivel 1 es una vista lateral fija; no hay seguimiento vertical de camara.
}

void NivelPiscinaEntrenamiento::dibujarEscenario(QPainter& painter)
{
    if (!spriteFondoCiudad.isNull()) {
        QRect fuente(0, std::max(0, spriteFondoCiudad.height() - 620), spriteFondoCiudad.width(), 620);
        painter.drawPixmap(QRect(0, 0, 800, 600), spriteFondoCiudad, fuente);
        painter.fillRect(QRectF(0, 0, 800, 600), QColor(7, 22, 38, 78));
    }
    else {
        QLinearGradient cielo(0, 0, 0, 600);
        cielo.setColorAt(0.0, QColor(50, 165, 230));
        cielo.setColorAt(1.0, QColor(42, 112, 86));
        painter.fillRect(QRectF(0, 0, 800, 600), cielo);
    }

    if (!spriteTexturaPixel.isNull()) {
        painter.setOpacity(0.16);
        painter.fillRect(QRectF(0, 0, 800, 600), QBrush(spriteTexturaPixel));
        painter.setOpacity(1.0);
    }

    dibujarRafagasViento(painter);
    dibujarTrayectoriaGuia(painter);

    QRectF exteriorPiscina(piscina.x() - 42.0f, piscina.y() - 24.0f, piscina.width() + 84.0f, piscina.height() + 74.0f);
    QRectF huecoPiscina(piscina.x() - 4.0f, piscina.y() + 8.0f, piscina.width() + 8.0f, piscina.height() - 2.0f);
    dibujarPisoAlrededorPiscina(painter, spriteAlrededorPiscina, exteriorPiscina, huecoPiscina);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(5, 22, 32, 122));
    painter.drawRect(QRectF(piscina.x() - 15.0f, piscina.y() - 5.0f, piscina.width() + 30.0f, piscina.height() + 30.0f));

    if (!spritePiscinaPremium.isNull()) {
        painter.drawPixmap(QRect(static_cast<int>(piscina.x() - 34.0f), static_cast<int>(piscina.y() - 40.0f), 260, 138), spritePiscinaPremium);
    }
    else if (!spritePiscinaBase.isNull()) {
        painter.drawPixmap(QRect(static_cast<int>(piscina.x() - 14.0f), static_cast<int>(piscina.y() - 22.0f), 218, 104), spritePiscinaBase);
    }

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(255, 238, 95, 220), 3));
    painter.setBrush(QColor(255, 238, 95, 42));
    painter.drawRoundedRect(zonaPerfecta.adjusted(0.0, -8.0, 0.0, 8.0), 4.0, 4.0);
    painter.setPen(QPen(QColor(255, 255, 255, 130), 1, Qt::DashLine));
    painter.drawLine(QPointF(zonaPerfecta.center().x(), zonaPerfecta.top() - 14.0),
                     QPointF(zonaPerfecta.center().x(), zonaPerfecta.bottom() + 14.0));
    painter.restore();

    plataforma->dibujar(painter);

    if (tiempoIman > 0.0f) {
        dibujarIman(painter);
    }

    if (intentoEvaluado && piscina.intersects(jugador->rect())) {
        if (!spriteBurbujas.isNull()) {
            painter.drawPixmap(QRect(static_cast<int>(jugador->centro().x() - 78.0f),
                                     static_cast<int>(piscina.y() + 6.0f), 156, 136), spriteBurbujas);
        }

        const QPixmap* splash = &spriteChapuzonFuerte;
        if (puntajeUltimoIntento >= 85) {
            splash = &spriteChapuzonLimpio;
        }
        else if (puntajeUltimoIntento >= 62) {
            splash = &spriteChapuzonMedio;
        }
        if (!splash->isNull()) {
            SpriteCache::dibujarAjustado(painter, *splash,
                                         QRect(static_cast<int>(jugador->centro().x() - 118.0f),
                                               static_cast<int>(piscina.y() - 88.0f), 236, 138),
                                         "salpicadura_precision");
        }
    }

    jugador->dibujar(painter);
}

void NivelPiscinaEntrenamiento::dibujarTrayectoriaGuia(QPainter& painter)
{
    if (jugador->estaEnAire() || esperandoSiguienteIntento || nivelSuperado || nivelPerdido) {
        return;
    }

    const float vxInicial = potenciaActual * 0.91f + velocidadPlataforma * 0.36f;
    const float vyInicial = -potenciaActual * 0.62f;
    const QPointF inicio = jugador->centro();

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen guia(QColor(255, 255, 255, 135), 2, Qt::DashLine);
    guia.setCapStyle(Qt::RoundCap);
    painter.setPen(guia);
    painter.setBrush(Qt::NoBrush);

    QPointF anterior = inicio;
    for (int i = 1; i <= 32; ++i) {
        const float t = i * 0.075f;
        QPointF punto(inicio.x() + vxInicial * t,
                      inicio.y() + vyInicial * t + 0.5f * gravedad * t * t);
        if (punto.y() > piscina.bottom() + 18.0f || punto.x() > ANCHO_NIVEL + 30.0f) {
            break;
        }
        painter.drawLine(anterior, punto);
        anterior = punto;
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 238, 95, 190));
    painter.drawEllipse(anterior, 4.0, 4.0);
    painter.restore();
}

void NivelPiscinaEntrenamiento::dibujarRafagasViento(QPainter& painter)
{
    if (intentoActual < 3) {
        return;
    }

    painter.save();
    painter.setOpacity(jugadorEnZonaViento ? 0.92 : 0.52);
    float direccion = intentoActual % 2 == 0 ? -1.0f : 1.0f;
    const QPixmap& sprite = direccion > 0.0f ? spriteVientoDerecha : spriteVientoIzquierda;
    if (!sprite.isNull()) {
        for (int i = 0; i < 3; ++i) {
            int x = static_cast<int>(zonaViento.x() + 10.0f + std::fmod(tiempoNivel * 90.0f + i * 62.0f, zonaViento.width() - 78.0f));
            int y = static_cast<int>(zonaViento.y() + 36.0f + i * 58.0f + std::sin(tiempoNivel * 5.0f + i) * 8.0f);
            painter.drawPixmap(QRect(x, y, 92, 38), sprite);
        }
    }
    else if (!spriteViento.isNull()) {
        painter.drawPixmap(QRect(static_cast<int>(zonaViento.x() + 20.0f), static_cast<int>(zonaViento.y() + 62.0f), 150, 64), spriteViento);
    }
    painter.restore();
}

void NivelPiscinaEntrenamiento::dibujarPotencia(QPainter& painter, const QRectF& rect)
{
    float porcentaje = (potenciaActual - potenciaMinima) / std::max(1.0f, potenciaMaxima - potenciaMinima);
    QColor relleno = colorPotencia(porcentaje);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(255, 255, 255, 150), 1));
    painter.setBrush(QColor(0, 18, 32, 190));
    painter.drawRoundedRect(rect.adjusted(-2, -2, 2, 2), 4.0, 4.0);

    QLinearGradient gradiente(rect.topLeft(), rect.topRight());
    gradiente.setColorAt(0.0, QColor(170, 55, 62));
    gradiente.setColorAt(0.50, QColor(255, 220, 84));
    gradiente.setColorAt(1.0, QColor(74, 225, 152));
    painter.setPen(Qt::NoPen);
    painter.setBrush(gradiente);
    painter.drawRoundedRect(QRectF(rect.x(), rect.y(), rect.width() * porcentaje, rect.height()), 3.0, 3.0);

    QRectF zonaIdeal(rect.x() + rect.width() * 0.70f, rect.y() - 4.0f, rect.width() * 0.16f, rect.height() + 8.0f);
    painter.setBrush(QColor(255, 255, 255, 34));
    painter.setPen(QPen(QColor(255, 255, 255, 115), 1));
    painter.drawRoundedRect(zonaIdeal, 3.0, 3.0);

    painter.setPen(QPen(relleno.lighter(125), 2));
    float marcadorX = rect.x() + rect.width() * porcentaje;
    painter.drawLine(QPointF(marcadorX, rect.y() - 5.0f), QPointF(marcadorX, rect.bottom() + 5.0f));
    painter.restore();
}

void NivelPiscinaEntrenamiento::dibujarGaugePostura(QPainter& painter, const QRectF& rect)
{
    const QPointF centro(rect.x() + 68.0f, rect.y() + 76.0f);
    const float radio = 48.0f;
    const float errorPostura = std::abs(anguloPostura - 90.0f);
    const QString estado = errorPostura < 7.0f ? "Excelente" : errorPostura < 17.0f ? "Aceptable" : "Mala postura";
    const QColor colorEstado = errorPostura < 7.0f ? QColor(86, 232, 155) :
                               errorPostura < 17.0f ? QColor(255, 221, 90) :
                               QColor(245, 96, 92);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(255, 255, 255, 78), 8));
    painter.drawArc(QRectF(centro.x() - radio, centro.y() - radio, radio * 2.0f, radio * 2.0f), 200 * 16, 140 * 16);
    painter.setPen(QPen(QColor(86, 232, 155, 185), 8));
    painter.drawArc(QRectF(centro.x() - radio, centro.y() - radio, radio * 2.0f, radio * 2.0f), 258 * 16, 24 * 16);

    const float normalizado = (std::clamp(anguloPostura, 55.0f, 125.0f) - 55.0f) / 70.0f;
    const float gradosAguja = 200.0f + normalizado * 140.0f;
    const float rad = gradosAguja * PI / 180.0f;
    QPointF punta(centro.x() + std::cos(rad) * (radio - 5.0f),
                  centro.y() + std::sin(rad) * (radio - 5.0f));

    painter.setPen(QPen(colorEstado, 3));
    painter.drawLine(centro, punta);
    painter.setPen(Qt::NoPen);
    painter.setBrush(colorEstado);
    painter.drawEllipse(centro, 5.0, 5.0);

    QFont fuente = painter.font();
    fuente.setPointSize(8);
    fuente.setBold(true);
    painter.setFont(fuente);
    painter.setPen(colorEstado);
    painter.drawText(QRectF(rect.x() + 126.0f, rect.y() + 42.0f, rect.width() - 138.0f, 22.0f), Qt::AlignLeft, estado);
    fuente.setPointSize(7);
    fuente.setBold(false);
    painter.setFont(fuente);
    painter.setPen(QColor(210, 238, 248));
    painter.drawText(QRectF(rect.x() + 126.0f, rect.y() + 68.0f, rect.width() - 138.0f, 18.0f),
                     Qt::AlignLeft, QString::number(anguloPostura, 'f', 0) + " / ideal 90");
    painter.restore();
}

void NivelPiscinaEntrenamiento::dibujarIman(QPainter& painter)
{
    QColor color = jugador->getColorPoder();
    QColor relleno = color;
    color.setAlpha(160);
    relleno.setAlpha(24);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(color, 3));
    painter.setBrush(relleno);
    painter.drawEllipse(jugador->centro(), radioIman, radioIman * 0.72f);
    painter.restore();
}

void NivelPiscinaEntrenamiento::dibujarResultado(QPainter& painter)
{
    if (!intentoEvaluado && !nivelSuperado && !nivelPerdido) {
        return;
    }

    dibujarPanel(painter, QRectF(218, 206, 366, 176), QColor(3, 12, 25, 226));

    QFont fuente = painter.font();
    fuente.setPointSize(14);
    fuente.setBold(true);
    painter.setFont(fuente);
    painter.setPen(QColor(255, 225, 95));

    QString titulo = nivelSuperado ? "PISCINA SUPERADA" : nivelPerdido ? "PISCINA FALLIDA" : "INTENTO EVALUADO";
    painter.drawText(QRectF(242, 232, 318, 30), Qt::AlignCenter, titulo);

    fuente.setPointSize(9);
    fuente.setBold(false);
    painter.setFont(fuente);
    painter.setPen(QColor(220, 242, 250));
    painter.drawText(260, 292, "Ultimo intento: " + QString::number(puntajeUltimoIntento) + "/100");
    painter.drawText(260, 318, "Total: " + QString::number(puntajeTotal) + "/" + QString::number(puntajeObjetivo));
    painter.drawText(260, 344, "Error horizontal: " + QString::number(errorEntrada, 'f', 1));
    painter.drawText(260, 370, nivelSuperado || nivelPerdido ? "R reinicia el nivel" : "Preparando siguiente intento...");
}

void NivelPiscinaEntrenamiento::dibujarHud(QPainter& painter)
{
    dibujarPanel(painter, QRectF(18, 18, 350, 158), QColor(3, 12, 25, 214));

    QFont fuente = painter.font();
    fuente.setPointSize(12);
    fuente.setBold(true);
    painter.setFont(fuente);
    painter.setPen(QColor(238, 252, 255));
    painter.drawText(38, 46, "Piscina de precision");

    fuente.setPointSize(8);
    fuente.setBold(false);
    painter.setFont(fuente);
    painter.setPen(QColor(190, 232, 246));

    QString estado = nivelSuperado ? "Nivel superado" :
                     nivelPerdido ? "Nivel fallido" :
                     intentoEvaluado ? "Evaluacion" :
                     esperandoSiguienteIntento ? "Siguiente intento" :
                     !jugador->estaEnAire() ? "Preparacion" :
                     "Salto";

    painter.drawText(38, 72, "Intento: " + QString::number(std::min(intentoActual, intentosMaximos)) + "/" + QString::number(intentosMaximos));
    painter.drawText(38, 96, "Total: " + QString::number(puntajeTotal) + " / " + QString::number(puntajeObjetivo));
    painter.drawText(190, 96, "Ultimo: " + QString::number(puntajeUltimoIntento));
    painter.drawText(38, 120, "Estado: " + estado);

    painter.setPen(QColor(255, 225, 95));
    painter.drawText(38, 148, "Potencia");
    dibujarPotencia(painter, QRectF(112, 137, 214, 14));

    for (int i = 0; i < intentosMaximos; ++i) {
        const QPixmap& corazon = i < (intentosMaximos - intentoActual + 1) && !nivelPerdido ? spriteCorazonLleno : spriteCorazonVacio;
        if (!corazon.isNull()) {
            painter.drawPixmap(QRect(302 - i * 25, 48, 22, 22), corazon);
        }
    }

    QRectF panelPostura(542, 24, 236, 136);
    dibujarPanel(painter, panelPostura, QColor(3, 12, 25, 184));
    painter.setPen(QColor(190, 236, 248));
    if (jugadorEnZonaViento && !spriteAdvertenciaHud.isNull()) {
        painter.drawPixmap(QRect(556, 112, 22, 22), spriteAdvertenciaHud);
    }
    dibujarGaugePostura(painter, panelPostura);

    fuente.setPointSize(8);
    fuente.setBold(false);
    painter.setFont(fuente);
    painter.setPen(QColor(190, 236, 248));
    painter.drawText(jugadorEnZonaViento ? 582 : 558, 132, jugadorEnZonaViento ? "Rafaga leve" : "Aire estable");

    dibujarResultado(painter);
}

void NivelPiscinaEntrenamiento::dibujar(QPainter& painter)
{
    dibujarEscenario(painter);
    dibujarHud(painter);
}

void NivelPiscinaEntrenamiento::teclaPresionada(int tecla)
{
    if (tecla == Qt::Key_Space && !jugador->estaEnAire() && intentoEnCurso && !esperandoSiguienteIntento) {
        eventosSonido.push_back(SONIDO_SALTO);
        jugador->setEnAire(true);
        jugador->setVX(potenciaActual * 0.91f + velocidadPlataforma * 0.36f);
        jugador->setVY(-potenciaActual * 0.62f);
        tiempoIntento = 0.0f;
        return;
    }

    if (tecla == Qt::Key_A || tecla == Qt::Key_Left) {
        corrigiendoIzquierda = true;
        jugador->moverIzquierda(true);
    }
    if (tecla == Qt::Key_D || tecla == Qt::Key_Right) {
        corrigiendoDerecha = true;
        jugador->moverDerecha(true);
    }
    if (tecla == Qt::Key_W || tecla == Qt::Key_Up) {
        frenandoCaida = true;
    }
    if (tecla == Qt::Key_S || tecla == Qt::Key_Down) {
        acelerandoCaida = true;
    }
    if (tecla == Qt::Key_E) {
        activarIman();
    }
    if (tecla == Qt::Key_R) {
        reiniciarNivel();
    }
}

void NivelPiscinaEntrenamiento::teclaLiberada(int tecla)
{
    if (tecla == Qt::Key_A || tecla == Qt::Key_Left) {
        corrigiendoIzquierda = false;
        jugador->moverIzquierda(false);
    }
    if (tecla == Qt::Key_D || tecla == Qt::Key_Right) {
        corrigiendoDerecha = false;
        jugador->moverDerecha(false);
    }
    if (tecla == Qt::Key_W || tecla == Qt::Key_Up) {
        frenandoCaida = false;
    }
    if (tecla == Qt::Key_S || tecla == Qt::Key_Down) {
        acelerandoCaida = false;
    }
}

void NivelPiscinaEntrenamiento::mousePresionado(const QPointF& posicion)
{
    Q_UNUSED(posicion);
    activarIman();
}

void NivelPiscinaEntrenamiento::activarIman()
{
    if (!jugador->estaEnAire() || esperandoSiguienteIntento || nivelSuperado || nivelPerdido) {
        return;
    }

    if (cooldownIman <= 0.0f && jugador->getEnergia() > 8.0f) {
        tiempoIman = 1.05f;
        cooldownIman = 2.4f;
        usoImpulsoIntento = true;
        jugador->activarImpulso(true);
        eventosSonido.push_back(SONIDO_ANILLO);
    }
}

void NivelPiscinaEntrenamiento::reiniciarNivel()
{
    intentoActual = 1;
    puntajeTotal = 0;
    puntajeUltimoIntento = 0;
    mejorPuntaje = 0;
    puntaje = 0;
    nivelSuperado = false;
    nivelPerdido = false;
    motivoDerrotaActual = "";
    esperandoSiguienteIntento = false;
    intentoEvaluado = false;
    tiempoNivel = 0.0f;
    potenciaActual = potenciaMinima;
    direccionPotencia = 1;
    jugador->setEnergiaMaxima(dificultad.getEnergiaInicial());
    iniciarIntento();
}

void NivelPiscinaEntrenamiento::configurarPersonaje(TipoPersonaje tipo)
{
    jugador->configurarTipo(tipo);
    radioIman = jugador->getRadioPoder();
    reiniciarNivel();
}

void NivelPiscinaEntrenamiento::cambiarDificultad(TipoDificultad tipo)
{
    dificultad.configurar(tipo);
    aplicarParametrosDificultad();
    reiniciarNivel();
}

int NivelPiscinaEntrenamiento::getPuntaje() const
{
    return puntajeTotal;
}

bool NivelPiscinaEntrenamiento::estaSuperado() const
{
    return nivelSuperado;
}

bool NivelPiscinaEntrenamiento::estaPerdido() const
{
    return nivelPerdido;
}

QString NivelPiscinaEntrenamiento::motivoDerrota() const
{
    return motivoDerrotaActual.isEmpty()
               ? "No alcanzaste el puntaje objetivo del nivel."
               : motivoDerrotaActual;
}

QString NivelPiscinaEntrenamiento::nombreNivel() const
{
    return "Piscina de precision";
}

QVector<EventoSonidoJuego> NivelPiscinaEntrenamiento::consumirEventosSonido()
{
    QVector<EventoSonidoJuego> eventos = eventosSonido;
    eventosSonido.clear();
    return eventos;
}
