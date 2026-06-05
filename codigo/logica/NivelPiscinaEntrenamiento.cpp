#include "NivelPiscinaEntrenamiento.h"
#include "JuegoException.h"
#include "../fisicas/ModelosFisicos.h"
#include "../render/SpriteCache.h"
#include <algorithm>
#include <cmath>
#include <QLinearGradient>

namespace {
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

void dibujarBarra(QPainter& painter, const QRectF& rect, float porcentaje, const QColor& color)
{
    porcentaje = std::max(0.0f, std::min(1.0f, porcentaje));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 52));
    painter.drawRect(rect);
    painter.setBrush(color);
    painter.drawRect(QRectF(rect.x(), rect.y(), rect.width() * porcentaje, rect.height()));
}

QRectF contraer(const QRectF& rect, float x, float y)
{
    return rect.adjusted(x, y, -x, -y);
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
    painter.setOpacity(0.78);
    QBrush mosaico(textura);
    painter.fillRect(QRectF(exterior.left(), exterior.top(), exterior.width(), huecoPiscina.top() - exterior.top()), mosaico);
    painter.fillRect(QRectF(exterior.left(), huecoPiscina.bottom(), exterior.width(), exterior.bottom() - huecoPiscina.bottom()), mosaico);
    painter.fillRect(QRectF(exterior.left(), huecoPiscina.top(), huecoPiscina.left() - exterior.left(), huecoPiscina.height()), mosaico);
    painter.fillRect(QRectF(huecoPiscina.right(), huecoPiscina.top(), exterior.right() - huecoPiscina.right(), huecoPiscina.height()), mosaico);
    painter.setOpacity(1.0);
    painter.setPen(QPen(QColor(255, 255, 255, 95), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(exterior.adjusted(1.0, 1.0, -1.0, -1.0));
    painter.restore();
}
}

NivelPiscinaEntrenamiento::NivelPiscinaEntrenamiento()
    : jugador(std::make_unique<Personaje>()),
      plataforma(std::make_unique<Plataforma>()),
      dron(std::make_unique<DronVigilante>(560.0f, 145.0f))
{
    alturaMundo = 4200.0f;
    camaraY = 0.0f;
    piscina = QRectF(220, 3890, 360, 105);
    zonaMeta = QRectF(340, 3890, 120, 105);
    zonaViento = QRectF(110, 1050, 580, 1680);
    suelo = QRectF(0, 4108, 800, 92);

    spritePiscina.load(":/recursos/sprites/salpicadura_grande.png");
    spritePiscinaBase.load(":/recursos/sprites/piscina_ciudad_academia.png");
    spriteViento.load(":/recursos/sprites/viento_lateral.png");
    spriteAdvertencia.load(":/recursos/sprites/advertencia.png");
    spriteTemporizador.load(":/recursos/sprites/temporizador.png");
    spriteFondoCiudad.load(":/recursos/sprites/fondo_vertical_entrenamiento.png");
    spriteGradas.load(":/recursos/sprites/gradas_academia.png");
    spriteCarriles.load(":/recursos/sprites/carriles_piscina.png");
    spriteBanderines.load(":/recursos/sprites/banderines_academia.png");
    spriteEdificio.load(":/recursos/sprites/edificio_cristal_academia.png");
    spriteBrilloAgua.load(":/recursos/sprites/brillo_agua.png");
    spritePiscinaPremium.load(":/recursos/sprites/piscina_final_premium.png");
    spriteAguaProfunda.load(":/recursos/sprites/agua_profunda.png");
    spriteChapuzonLimpio.load(":/recursos/sprites/chapuzon_limpio.png");
    spriteChapuzonMedio.load(":/recursos/sprites/chapuzon_medio.png");
    spriteChapuzonFuerte.load(":/recursos/sprites/chapuzon_fuerte.png");
    spriteBurbujas.load(":/recursos/sprites/burbujas_entrada.png");
    spriteTexturaTierra.load(":/recursos/sprites/textura_tierra_borde.png");
    spriteDecoracionIzq.load(":/recursos/sprites/decoracion_piscina_izq.png");
    spriteDecoracionDer.load(":/recursos/sprites/decoracion_piscina_der.png");
    spriteTexturaPixel.load(":/recursos/sprites/textura_pixel_overlay.png");
    spriteAlrededorPiscina.load(":/recursos/sprites/alrededor_piscina.jpg");
    spriteCorazonLleno.load(":/recursos/sprites/hud_corazon_lleno.png");
    spriteCorazonVacio.load(":/recursos/sprites/hud_corazon_vacio.png");
    spriteAdvertenciaHud.load(":/recursos/sprites/hud_advertencia.png");

    exigirSprite(spriteFondoCiudad, "fondo_vertical_entrenamiento.png");
    exigirSprite(spritePiscinaPremium, "piscina_final_premium.png");
    exigirSprite(spriteCorazonLleno, "hud_corazon_lleno.png");
    exigirSprite(spriteCorazonVacio, "hud_corazon_vacio.png");

    puntaje = 0;
    mejorPuntaje = 0;
    intentosRestantes = 0;
    monedasRecolectadas = 0;

    intentoTerminado = false;
    intentoGanado = false;
    nivelSuperado = false;
    nivelPerdido = false;
    jugadorEnZonaViento = false;
    frenandoCaida = false;
    acelerandoCaida = false;

    vientoLateral = 0.0f;
    gravedad = 4.2f;
    errorEntrada = 0.0f;
    tiempoNivel = 0.0f;
    tiempoIman = 0.0f;
    cooldownIman = 0.0f;
    radioIman = 172.0f;
    piscinaVelocidad = 0.0f;
    piscinaAceleracion = 0.0f;
    tiempoReinicioIntento = 0.0f;
    tiempoCorreccionLateral = 0.0f;

    dificultad.configurar(NORMAL);
    aplicarParametrosDificultad();
    plataforma->colocarEn(-8.0f, 165.0f);
    proyectilesDron.reserve(10);

    intentosRestantes = dificultad.getIntentosMaximos();
    crearMonedas();

    jugador->colocarEn(
        plataforma->getX() + plataforma->getAncho() / 2.0f - jugador->getAncho() / 2.0f,
        plataforma->getY() - jugador->getAlto() + 8.0f
        );
}

NivelPiscinaEntrenamiento::~NivelPiscinaEntrenamiento() = default;

void NivelPiscinaEntrenamiento::aplicarParametrosDificultad()
{
    vientoLateral = dificultad.getIntensidadViento();

    plataforma->configurarOscilacion(0.0f, 0.0f);

    jugador->setEnergiaMaxima(dificultad.getEnergiaInicial());

    if (dificultad.getTipo() == FACIL) {
        gravedad = 30.0f * dificultad.getFactorGravedad();
    }
    else if (dificultad.getTipo() == NORMAL) {
        gravedad = 38.0f * dificultad.getFactorGravedad();
    }
    else {
        gravedad = 48.0f * dificultad.getFactorGravedad();
    }
}

void NivelPiscinaEntrenamiento::cambiarDificultad(TipoDificultad tipo)
{
    dificultad.configurar(tipo);
    aplicarParametrosDificultad();
    reiniciarNivel();
}

void NivelPiscinaEntrenamiento::actualizar(float dt)
{
    bool intentoActivo = !intentoTerminado && !nivelSuperado && !nivelPerdido;

    if (intentoTerminado && !intentoGanado && !nivelSuperado && !nivelPerdido) {
        tiempoReinicioIntento += dt;
        if (tiempoReinicioIntento >= 1.25f) {
            reiniciarIntento();
            return;
        }
    }

    plataforma->actualizar(dt);
    if (intentoActivo && jugador->estaEnAire()) {
        float yDron = std::clamp(jugador->getY() - 170.0f, 112.0f, alturaMundo - 300.0f);
        dron->colocarY(yDron);
    }
    if (intentoActivo) {
        dron->actualizar(dt, *jugador);
    }
    if (intentoActivo && dron->solicitarDisparo(dt, *jugador)) {
        crearProyectilDron();
    }
    tiempoNivel += dt;
    cooldownIman = std::max(0.0f, cooldownIman - dt);
    tiempoIman = std::max(0.0f, tiempoIman - dt);
    if (tiempoIman <= 0.0f) {
        jugador->activarImpulso(false);
    }
    actualizarPiscina(dt);
    actualizarMonedas(dt);
    if (intentoActivo) {
        actualizarProyectilesDron(dt);
    }
    else {
        proyectilesDron.clear();
    }

    if (!jugador->estaEnAire()) {
        jugador->colocarEn(
            plataforma->getX() + plataforma->getAncho() / 2.0f - jugador->getAncho() / 2.0f,
            plataforma->getY() - jugador->getAlto() + 8.0f
            );
    }

    if (!intentoTerminado && !nivelSuperado && !nivelPerdido) {
        verificarZonaViento();

        jugador->aplicarGravedad(gravedad);
        if (jugador->estaEnAire() && frenandoCaida) {
            jugador->setVY(std::max(80.0f, jugador->getVY() - 150.0f * dt));
        }
        if (jugador->estaEnAire() && acelerandoCaida) {
            jugador->setVY(std::min(760.0f, jugador->getVY() + 145.0f * dt));
        }
        if (jugador->estaEnAire() && jugador->estaCorrigiendoLateral()) {
            float cercaniaEntrada = std::clamp(static_cast<float>((jugador->getY() - (piscina.y() - 760.0f)) / 760.0f), 0.15f, 1.0f);
            tiempoCorreccionLateral += dt * cercaniaEntrada;
        }

        if (jugadorEnZonaViento) {
            float turbulencia = std::sin(tiempoNivel * 4.0f + jugador->getY() * 0.01f) * 26.0f;
            jugador->aplicarViento(vientoLateral + turbulencia);
        }

        if (contraer(dron->rect(), 10.0f, 10.0f).intersects(jugador->hitboxAjustada()) && jugador->estaEnAire()) {
            jugador->aplicarViento(jugador->centro().x() < dron->centro().x() ? -120.0f : 120.0f);
        }

        jugador->actualizar(dt);
        if (jugador->getY() < 32.0f) {
            jugador->colocarEn(jugador->getX(), 32.0f);
            jugador->setVY(std::max(0.0f, jugador->getVY()));
        }
        verificarColisiones();
    }

    actualizarCamara();
}

void NivelPiscinaEntrenamiento::crearMonedas()
{
    monedas.clear();
    monedas.reserve(16);
    monedas.emplace_back(std::make_unique<Moneda>(392.0f, 450.0f));
    monedas.emplace_back(std::make_unique<Moneda>(210.0f, 690.0f));
    monedas.emplace_back(std::make_unique<Moneda>(555.0f, 920.0f));
    monedas.emplace_back(std::make_unique<Moneda>(310.0f, 1190.0f));
    monedas.emplace_back(std::make_unique<Moneda>(610.0f, 1470.0f));
    monedas.emplace_back(std::make_unique<Moneda>(180.0f, 1760.0f));
    monedas.emplace_back(std::make_unique<Moneda>(420.0f, 2070.0f));
    monedas.emplace_back(std::make_unique<Moneda>(580.0f, 2350.0f));
    monedas.emplace_back(std::make_unique<Moneda>(250.0f, 2640.0f));
    monedas.emplace_back(std::make_unique<Moneda>(475.0f, 2920.0f));
    monedas.emplace_back(std::make_unique<Moneda>(160.0f, 3210.0f));
    monedas.emplace_back(std::make_unique<Moneda>(620.0f, 3490.0f));
    monedas.emplace_back(std::make_unique<Moneda>(350.0f, 3730.0f));
}

void NivelPiscinaEntrenamiento::actualizarMonedas(float dt)
{
    QPointF objetivo = jugador->centro();
    for (const auto& moneda : monedas) {
        if (moneda->estaRecolectada()) {
            continue;
        }

        moneda->actualizar(dt);
        float dx = moneda->centro().x() - objetivo.x();
        float dy = moneda->centro().y() - objetivo.y();
        float distancia2 = FisicaJuego::distanciaCuadrada(dx, dy);
        auto dentroDelRadio = [distancia2](float radio) {
            return distancia2 <= radio * radio;
        };

        if (tiempoIman > 0.0f) {
            if (jugador->getTipo() == PERSONAJE_ACCELERATOR && dentroDelRadio(radioIman * 1.18f)) {
                moneda->desplazarVectorialHacia(objetivo, jugador->getAtraccionMonedas() * 1.35f, radioIman * 1.18f, dt);
            }
            else if (jugador->getTipo() == PERSONAJE_MUGINO) {
                QPointF foco(objetivo.x(), objetivo.y());
                moneda->canalizarMeltdowner(foco, jugador->getAtraccionMonedas() * 1.18f, radioIman * 0.62f, dt);
            }
            else if (jugador->getTipo() == PERSONAJE_DARK_MATTER && dentroDelRadio(radioIman * 1.12f)) {
                moneda->orbitarHacia(objetivo, jugador->getAtraccionMonedas() * 1.05f, radioIman * 1.12f, dt);
            }
            else if (dentroDelRadio(radioIman)) {
                moneda->atraerHacia(objetivo, jugador->getAtraccionMonedas(), radioIman, dt);
            }
        }

        if (contraer(moneda->rect(), 7.0f, 7.0f).intersects(jugador->hitboxAjustada())) {
            moneda->recolectar();
            monedasRecolectadas++;
            puntaje = std::min(100, puntaje + 3);
            eventosSonido.push_back(SONIDO_ANILLO);
        }
    }
}

void NivelPiscinaEntrenamiento::actualizarPiscina(float dt)
{
    if (intentoTerminado || nivelSuperado || nivelPerdido) {
        return;
    }

    float factor = dificultad.getFactorPiscinaMovil();
    piscinaAceleracion = std::sin(tiempoNivel * 0.82f) * 34.0f * factor;
    piscinaVelocidad += piscinaAceleracion * dt;
    piscinaVelocidad *= 0.992f;

    piscina.translate(piscinaVelocidad * dt, 0.0f);

    const float limiteIzq = 150.0f;
    const float limiteDer = 290.0f;
    if (piscina.x() < limiteIzq) {
        piscina.moveLeft(limiteIzq);
        piscinaVelocidad = std::abs(piscinaVelocidad) * 0.82f;
    }
    else if (piscina.x() > limiteDer) {
        piscina.moveLeft(limiteDer);
        piscinaVelocidad = -std::abs(piscinaVelocidad) * 0.82f;
    }

    zonaMeta.moveCenter(QPointF(piscina.center().x(), zonaMeta.center().y()));
}

void NivelPiscinaEntrenamiento::crearProyectilDron()
{
    bool elastico = dron->getEstado() != INTERCEPTA;
    float rapidezJugador = std::sqrt(FisicaJuego::rapidezCuadrada(jugador->getVX(), jugador->getVY()));
    float rapidezBase = (elastico ? 320.0f : 285.0f) * dificultad.getFactorAgente();
    float rapidez = std::clamp(rapidezBase + rapidezJugador * 0.72f, 360.0f, 920.0f);
    QPointF velocidad = dron->calcularVectorDisparo(*jugador, rapidez);
    proyectilesDron.emplace_back(std::make_unique<ProyectilDron>(
        dron->centro().x() - 12.0f,
        dron->centro().y() - 12.0f,
        static_cast<float>(velocidad.x()),
        static_cast<float>(velocidad.y()),
        elastico
        ));
}

void NivelPiscinaEntrenamiento::actualizarProyectilesDron(float dt)
{
    if (intentoTerminado || nivelSuperado || nivelPerdido) {
        proyectilesDron.clear();
        return;
    }

    for (const auto& proyectil : proyectilesDron) {
        bool estabaActivo = proyectil->estaActivo();
        proyectil->actualizar(dt);

        if (proyectil->estaActivo() && proyectil->rect().intersects(jugador->hitboxAjustada()) && jugador->estaEnAire()) {
            resolverColisionProyectil(*proyectil);
            proyectil->desactivar();
            puntaje = std::max(0, puntaje - (proyectil->usaColisionElastica() ? 2 : 5));
            dron->aprender(175.0f);
            dron->registrarImpactoJugador();
            eventosSonido.push_back(SONIDO_COLISION);
        }
        else if (estabaActivo && !proyectil->estaActivo()) {
            dron->registrarEvasionJugador();
        }
    }

    proyectilesDron.erase(
        std::remove_if(proyectilesDron.begin(), proyectilesDron.end(),
                       [](const std::unique_ptr<ProyectilDron>& proyectil) {
                           return !proyectil->estaActivo();
                       }),
        proyectilesDron.end());
}

void NivelPiscinaEntrenamiento::resolverColisionProyectil(ProyectilDron& proyectil)
{
    auto choqueX = proyectil.usaColisionElastica()
                       ? FisicaJuego::resolverChoqueElastico(jugador->getMasa(), jugador->getVX(),
                                                             proyectil.getMasa(), proyectil.getVX())
                       : FisicaJuego::resolverChoqueInelastico(jugador->getMasa(), jugador->getVX(),
                                                               proyectil.getMasa(), proyectil.getVX());
    auto choqueY = proyectil.usaColisionElastica()
                       ? FisicaJuego::resolverChoqueElastico(jugador->getMasa(), jugador->getVY(),
                                                             proyectil.getMasa(), proyectil.getVY())
                       : FisicaJuego::resolverChoqueInelastico(jugador->getMasa(), jugador->getVY(),
                                                               proyectil.getMasa(), proyectil.getVY());

    jugador->setVX(std::clamp(choqueX.velocidadA, -360.0f, 360.0f));
    jugador->setVY(std::clamp(choqueY.velocidadA, -120.0f, 780.0f));
    proyectil.setVX(choqueX.velocidadB);
    proyectil.setVY(choqueY.velocidadB);
}

void NivelPiscinaEntrenamiento::actualizarCamara()
{
    camaraY = std::clamp(jugador->getY() - 250.0f, 0.0f, alturaMundo - 600.0f);
}

void NivelPiscinaEntrenamiento::verificarColisiones()
{
    verificarPiscina();
    verificarSuelo();
}

void NivelPiscinaEntrenamiento::verificarZonaViento()
{
    jugadorEnZonaViento = false;

    if (jugador->estaEnAire() && zonaViento.intersects(jugador->rect())) {
        jugadorEnZonaViento = true;
    }
}

void NivelPiscinaEntrenamiento::verificarPiscina()
{
    if (intentoTerminado || nivelSuperado || nivelPerdido) {
        return;
    }

    if (piscina.intersects(jugador->hitboxAjustada()) && jugador->getVY() > 0.0f) {
        intentoTerminado = true;
        calcularPuntajePorEntrada();

        if (puntaje > mejorPuntaje) {
            mejorPuntaje = puntaje;
        }

        if (puntaje >= dificultad.getPuntajeMinimo()) {
            intentoGanado = true;
            nivelSuperado = true;
            dron->registrarAciertoJugador();
            eventosSonido.push_back(SONIDO_NIVEL);
        }
        else {
            intentoGanado = false;
            tiempoReinicioIntento = 0.0f;
            intentosRestantes--;

            if (intentosRestantes <= 0) {
                nivelPerdido = true;
            }
        }

        jugador->detenerMovimiento();
        proyectilesDron.clear();
        dron->aprender(errorEntrada);
        eventosSonido.push_back(SONIDO_AGUA);
    }
}

void NivelPiscinaEntrenamiento::verificarSuelo()
{
    if (intentoTerminado || nivelSuperado || nivelPerdido) {
        return;
    }

    QRectF hitbox = jugador->hitboxAjustada();
    bool pasoElSuelo = hitbox.bottom() >= suelo.top();
    bool pasoLaPiscina = hitbox.top() > piscina.bottom() + 18.0f;
    bool fueraDelAgua = !piscina.intersects(hitbox);

    if ((suelo.intersects(hitbox) || pasoElSuelo || pasoLaPiscina) && fueraDelAgua) {
        intentoTerminado = true;
        intentoGanado = false;
        tiempoReinicioIntento = 0.0f;
        puntaje = 0;
        errorEntrada = std::abs(jugador->centro().x() - piscina.center().x());
        intentosRestantes--;

        if (intentosRestantes <= 0) {
            nivelPerdido = true;
        }

        jugador->detenerMovimiento();
        proyectilesDron.clear();
        eventosSonido.push_back(SONIDO_COLISION);
    }
}

void NivelPiscinaEntrenamiento::calcularPuntajePorEntrada()
{
    float centroPiscina = piscina.x() + piscina.width() / 2.0f;
    float centroJugador = jugador->getX() + jugador->getAncho() / 2.0f;

    errorEntrada = std::abs(centroPiscina - centroJugador);

    float velocidadVertical = std::abs(jugador->getVY());

    int puntajePrecision = 0;

    if (errorEntrada < 20.0f) {
        puntajePrecision = 90;
    }
    else if (errorEntrada < 50.0f) {
        puntajePrecision = 75;
    }
    else if (errorEntrada < 90.0f) {
        puntajePrecision = 55;
    }
    else if (errorEntrada < 135.0f) {
        puntajePrecision = 30;
    }
    else {
        puntajePrecision = 10;
    }

    int bonusVelocidad = 0;

    if (velocidadVertical >= 450.0f && velocidadVertical <= 850.0f) {
        bonusVelocidad = 10;
    }
    else if (velocidadVertical > 850.0f) {
        bonusVelocidad = -10;
    }

    int bonusMonedas = std::min(18, monedasRecolectadas * 2);
    int penalizacionCorreccion = static_cast<int>(std::clamp(tiempoCorreccionLateral * 12.0f, 0.0f, 24.0f));
    if (jugador->estaCorrigiendoLateral()) {
        penalizacionCorreccion += 8;
    }

    puntaje = puntajePrecision + bonusVelocidad + bonusMonedas - penalizacionCorreccion;

    if (puntaje > 100) {
        puntaje = 100;
    }

    if (puntaje < 0) {
        puntaje = 0;
    }
}

void NivelPiscinaEntrenamiento::dibujarEscenario(QPainter& painter)
{
    if (!spriteFondoCiudad.isNull()) {
        painter.drawPixmap(QRect(0, 0, 800, static_cast<int>(alturaMundo)), spriteFondoCiudad);
        painter.fillRect(QRectF(0, 0, 800, alturaMundo), QColor(7, 22, 38, 66));
        if (!spriteTexturaPixel.isNull()) {
            painter.setOpacity(0.24);
            painter.fillRect(QRectF(0, 0, 800, alturaMundo), QBrush(spriteTexturaPixel));
            painter.setOpacity(1.0);
        }
    }
    else {
        QLinearGradient cielo(0, 0, 0, alturaMundo);
        cielo.setColorAt(0.0, QColor(34, 150, 230));
        cielo.setColorAt(0.58, QColor(105, 205, 248));
        cielo.setColorAt(1.0, QColor(44, 126, 78));
        painter.fillRect(QRectF(0, 0, 800, alturaMundo), cielo);
    }

    dibujarRafagasViento(painter);

    const QRectF pisoVisual(0, 3820, 800, alturaMundo - 3820);
    QLinearGradient pisoFinal(0, pisoVisual.y(), 0, pisoVisual.y() + pisoVisual.height());
    pisoFinal.setColorAt(0.0, QColor(218, 232, 232));
    pisoFinal.setColorAt(0.36, QColor(126, 158, 142));
    pisoFinal.setColorAt(1.0, QColor(21, 112, 66));
    painter.setPen(Qt::NoPen);
    if (!spriteTexturaTierra.isNull()) {
        painter.fillRect(pisoVisual, QBrush(spriteTexturaTierra));
        painter.fillRect(pisoVisual, QColor(16, 88, 55, 68));
    }
    else {
        painter.setBrush(QBrush(pisoFinal));
        painter.drawRect(pisoVisual);
    }

    painter.setPen(QPen(QColor(255, 255, 255, 58), 2));
    painter.drawLine(QPointF(0, pisoVisual.y() + 6.0f), QPointF(800, pisoVisual.y() + 6.0f));

    QRectF exteriorPiscina(piscina.x() - 145.0f, piscina.y() - 95.0f, piscina.width() + 290.0f, 285.0f);
    QRectF huecoPiscina(piscina.x() - 22.0f, piscina.y() - 22.0f, piscina.width() + 44.0f, 150.0f);
    dibujarPisoAlrededorPiscina(painter, spriteAlrededorPiscina, exteriorPiscina, huecoPiscina);
    painter.setBrush(QColor(4, 26, 36, 110));
    painter.setPen(Qt::NoPen);
    painter.drawRect(QRectF(piscina.x() - 55.0f, piscina.y() - 24.0f, piscina.width() + 110.0f, 158));
    if (!spriteDecoracionIzq.isNull()) {
        painter.drawPixmap(QRect(static_cast<int>(piscina.x() - 118.0f),
                                 static_cast<int>(piscina.y() - 56.0f), 128, 180), spriteDecoracionIzq);
    }
    if (!spriteDecoracionDer.isNull()) {
        painter.drawPixmap(QRect(static_cast<int>(piscina.right() - 10.0f),
                                 static_cast<int>(piscina.y() - 56.0f), 128, 180), spriteDecoracionDer);
    }

    if (!spritePiscinaPremium.isNull()) {
        painter.drawPixmap(QRect(static_cast<int>(piscina.x() - 44.0f), static_cast<int>(piscina.y() - 54.0f), 448, 190), spritePiscinaPremium);
    }
    else if (!spritePiscinaBase.isNull()) {
        painter.drawPixmap(QRect(static_cast<int>(piscina.x() - 22.0f), static_cast<int>(piscina.y() - 34.0f), 402, 140), spritePiscinaBase);
    }

    for (const auto& moneda : monedas) {
        moneda->dibujar(painter);
    }

    plataforma->dibujar(painter);
    dron->dibujar(painter);
    dibujarProyectilesDron(painter);

    if (tiempoIman > 0.0f) {
        dibujarIman(painter);
    }

    if (intentoTerminado && piscina.intersects(jugador->rect())) {
        if (!spriteBurbujas.isNull()) {
            painter.drawPixmap(QRect(static_cast<int>(jugador->centro().x() - 86.0f),
                                     static_cast<int>(piscina.y() + 12.0f), 172, 170), spriteBurbujas);
        }

        const QPixmap* splash = &spriteChapuzonFuerte;
        if (puntaje >= 85) {
            splash = &spriteChapuzonLimpio;
        }
        else if (puntaje >= dificultad.getPuntajeMinimo()) {
            splash = &spriteChapuzonMedio;
        }
        if (!splash->isNull()) {
            SpriteCache::dibujarAjustado(painter, *splash,
                                         QRect(static_cast<int>(jugador->centro().x() - 135.0f),
                                               static_cast<int>(piscina.y() - 88.0f), 270, 158),
                                         "salpicadura_entrenamiento");
        }

        painter.save();
        painter.setClipRect(QRectF(piscina.x(), piscina.y(), piscina.width(), 150.0f));
        painter.setOpacity(0.45);
        const float xOriginal = jugador->getX();
        const float yOriginal = jugador->getY();
        jugador->colocarEn(xOriginal, piscina.y() + 40.0f);
        jugador->dibujar(painter);
        jugador->colocarEn(xOriginal, yOriginal);
        painter.restore();
    }
    else {
        jugador->dibujar(painter);
    }
}

void NivelPiscinaEntrenamiento::dibujarRafagasViento(QPainter& painter)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(jugadorEnZonaViento ? QColor(80, 220, 255, 46) : QColor(80, 220, 255, 25));
    painter.drawRect(zonaViento);

    painter.setPen(QPen(QColor(150, 235, 255, jugadorEnZonaViento ? 185 : 112), 3));
    for (int i = 0; i < 22; ++i) {
        float y = zonaViento.y() + 45.0f + i * 72.0f;
        float offset = std::sin(tiempoNivel * 3.0f + i) * 34.0f;
        painter.drawArc(QRectF(120 + offset, y, 560, 38), 0, 180 * 16);
        painter.drawLine(QPointF(170 + offset, y + 18), QPointF(620 + offset, y + 18));
        painter.drawLine(QPointF(620 + offset, y + 18), QPointF(596 + offset, y + 5));
        painter.drawLine(QPointF(620 + offset, y + 18), QPointF(596 + offset, y + 31));
    }

    if (!spriteViento.isNull()) {
        painter.setOpacity(jugadorEnZonaViento ? 0.78 : 0.48);
        for (int y = static_cast<int>(zonaViento.y()) + 120; y < zonaViento.bottom() - 80; y += 260) {
            painter.drawPixmap(QRect(280 + static_cast<int>(std::sin(tiempoNivel * 2.0f + y) * 30.0f), y, 220, 88), spriteViento);
        }
        painter.setOpacity(1.0);
    }
}

void NivelPiscinaEntrenamiento::dibujarIman(QPainter& painter)
{
    QColor color = jugador->getColorPoder();
    QColor relleno = color;
    relleno.setAlpha(24);
    color.setAlpha(160);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (jugador->getTipo() == PERSONAJE_ACCELERATOR) {
        painter.setPen(QPen(QColor(220, 245, 255, 180), 3));
        painter.setBrush(QColor(220, 245, 255, 22));
        painter.drawRect(QRectF(jugador->centro().x() - radioIman,
                                jugador->centro().y() - radioIman * 0.45f,
                                radioIman * 2.0f,
                                radioIman * 0.9f));
        for (int i = 0; i < 6; ++i) {
            float avance = -radioIman + i * (radioIman * 0.38f) + std::fmod(tiempoNivel * 90.0f, 36.0f);
            QPointF a(jugador->centro().x() + avance, jugador->centro().y() - 28.0f);
            QPointF b(jugador->centro().x() + avance + 42.0f, jugador->centro().y());
            painter.drawLine(a, b);
            painter.drawLine(b, QPointF(a.x(), jugador->centro().y() + 28.0f));
        }
    }
    else if (jugador->getTipo() == PERSONAJE_MUGINO) {
        painter.setPen(QPen(QColor(100, 255, 120, 210), 5));
        painter.setBrush(QColor(100, 255, 120, 22));
        painter.drawRoundedRect(QRectF(0.0f, jugador->centro().y() - radioIman * 0.32f,
                                       800.0f, radioIman * 0.64f), 14.0, 14.0);
        painter.setPen(QPen(QColor(210, 255, 160, 180), 2));
        painter.drawLine(QPointF(0.0f, jugador->centro().y()), QPointF(800.0f, jugador->centro().y()));
    }
    else if (jugador->getTipo() == PERSONAJE_DARK_MATTER) {
        painter.setPen(QPen(QColor(190, 120, 255, 185), 3));
        painter.setBrush(QColor(120, 70, 210, 24));
        painter.drawEllipse(jugador->centro(), radioIman, radioIman);
        for (int i = 0; i < 18; ++i) {
            float angulo = tiempoNivel * 4.0f + i * 0.46f;
            float radio = 22.0f + i * (radioIman / 20.0f);
            QPointF p(jugador->centro().x() + std::cos(angulo) * radio,
                      jugador->centro().y() + std::sin(angulo) * radio);
            painter.drawPoint(p);
        }
    }
    else {
        painter.setPen(QPen(color, 3));
        painter.setBrush(relleno);
        painter.drawEllipse(jugador->centro(), radioIman, radioIman);
        painter.setPen(QPen(QColor(80, 230, 255, 160), 2));
        for (int i = 0; i < 8; ++i) {
            float angulo = tiempoNivel * 3.2f + i * 0.78f;
            QPointF a(jugador->centro().x() + std::cos(angulo) * 42.0f,
                      jugador->centro().y() + std::sin(angulo) * 42.0f);
            QPointF b(jugador->centro().x() + std::cos(angulo) * radioIman,
                      jugador->centro().y() + std::sin(angulo) * radioIman);
            painter.drawLine(a, b);
        }
    }

    painter.restore();
}

void NivelPiscinaEntrenamiento::dibujarProyectilesDron(QPainter& painter)
{
    for (const auto& proyectil : proyectilesDron) {
        proyectil->dibujar(painter);
    }
}

void NivelPiscinaEntrenamiento::dibujarHud(QPainter& painter)
{
    float porcentajeEnergia = jugador->getEnergiaMaxima() > 0.0f ? jugador->getEnergia() / jugador->getEnergiaMaxima() : 0.0f;

    dibujarPanel(painter, QRectF(18, 18, 322, 190), QColor(3, 12, 25, 204));

    QFont fuente = painter.font();
    fuente.setPointSize(12);
    fuente.setBold(true);
    painter.setFont(fuente);
    painter.setPen(QColor(238, 252, 255));
    painter.drawText(38, 46, "Torre de entrenamiento");

    fuente.setPointSize(8);
    fuente.setBold(false);
    painter.setFont(fuente);
    painter.setPen(QColor(180, 226, 242));
    painter.drawText(38, 70, "Espacio: iniciar caida | Click: iman metalico");

    painter.setPen(QColor(255, 225, 95));
    painter.drawText(38, 100, "Puntaje");
    dibujarBarra(painter, QRectF(112, 90, 178, 10), puntaje / 100.0f, QColor(255, 225, 95));
    painter.drawText(298, 101, QString::number(puntaje));

    painter.setPen(QColor(120, 235, 255));
    painter.drawText(38, 126, "Energia");
    dibujarBarra(painter, QRectF(112, 116, 178, 10), porcentajeEnergia, QColor(120, 235, 255));

    painter.setPen(QColor(230, 245, 255));
    painter.drawText(38, 151, "Monedas: " + QString::number(monedasRecolectadas) + "/" + QString::number(monedas.size()));
    painter.drawText(38, 176, jugador->getNombre() + ": " + QString(tiempoIman > 0.0f ? "poder activo" : cooldownIman > 0.0f ? "recargando" : "listo"));

    for (int i = 0; i < dificultad.getIntentosMaximos(); ++i) {
        const QPixmap& corazon = i < intentosRestantes ? spriteCorazonLleno : spriteCorazonVacio;
        if (!corazon.isNull()) {
            painter.drawPixmap(QRect(206 + i * 27, 156, 24, 24), corazon);
        }
    }

    dibujarPanel(painter, QRectF(548, 24, 232, 74), QColor(3, 12, 25, 174));
    painter.setPen(QColor(190, 236, 248));
    if (jugadorEnZonaViento && !spriteAdvertenciaHud.isNull()) {
        painter.drawPixmap(QRect(556, 41, 28, 28), spriteAdvertenciaHud);
    }
    painter.drawText(jugadorEnZonaViento ? 588 : 568, 52, jugadorEnZonaViento ? "Rafagas visibles activas" : "Aire estable");
    painter.drawText(568, 78, "Descenso: " + QString::number(jugador->getY() / (alturaMundo - 80.0f) * 100.0f, 'f', 0) + "%");

    if (intentoTerminado || nivelSuperado || nivelPerdido) {
        dibujarPanel(painter, QRectF(210, 218, 420, 188), QColor(3, 12, 25, 220));
        fuente.setPointSize(14);
        fuente.setBold(true);
        painter.setFont(fuente);
        painter.setPen(QColor(255, 225, 95));
        painter.drawText(QRectF(234, 246, 372, 30), Qt::AlignCenter,
                         nivelSuperado ? "ENTRADA APROBADA" : nivelPerdido ? "ENTRADA FALLIDA" : "Entrada en evaluacion");

        fuente.setPointSize(9);
        fuente.setBold(false);
        painter.setFont(fuente);
        painter.setPen(QColor(220, 242, 250));
        painter.drawText(260, 305, "Puntaje: " + QString::number(puntaje));
        painter.drawText(260, 332, "Error: " + QString::number(errorEntrada, 'f', 1));
        painter.drawText(260, 359, "Monedas metalicas: " + QString::number(monedasRecolectadas));
        painter.drawText(260, 386, "R reinicia  |  Enter avanza si superaste");
    }
}

void NivelPiscinaEntrenamiento::dibujar(QPainter& painter)
{
    painter.save();
    painter.translate(0, -camaraY);
    dibujarEscenario(painter);
    painter.restore();

    dibujarHud(painter);
}

void NivelPiscinaEntrenamiento::teclaPresionada(int tecla)
{
    if (tecla == Qt::Key_Space) {
        if (!jugador->estaEnAire()) {
            eventosSonido.push_back(SONIDO_SALTO);
            jugador->setEnAire(true);
            jugador->setVX(130.0f);
            jugador->setVY(-86.0f);
        }
    }

    if (tecla == Qt::Key_A) {
        jugador->moverIzquierda(true);
    }

    if (tecla == Qt::Key_D) {
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
        if (nivelSuperado || nivelPerdido) {
            reiniciarNivel();
        }
        else {
            reiniciarIntento();
        }
    }

    if (tecla == Qt::Key_1) {
        cambiarDificultad(FACIL);
    }

    if (tecla == Qt::Key_2) {
        cambiarDificultad(NORMAL);
    }

    if (tecla == Qt::Key_3) {
        cambiarDificultad(DIFICIL);
    }
}

void NivelPiscinaEntrenamiento::teclaLiberada(int tecla)
{
    if (tecla == Qt::Key_A) {
        jugador->moverIzquierda(false);
    }

    if (tecla == Qt::Key_D) {
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
    if (intentoTerminado || nivelSuperado || nivelPerdido) {
        return;
    }

    if (cooldownIman <= 0.0f && jugador->getEnergia() > 8.0f) {
        tiempoIman = 1.15f;
        cooldownIman = 2.4f;
        if (jugador->getTipo() == PERSONAJE_ACCELERATOR) {
            tiempoIman = 0.86f;
            cooldownIman = 1.95f;
        }
        else if (jugador->getTipo() == PERSONAJE_MUGINO) {
            tiempoIman = 1.25f;
            cooldownIman = 2.55f;
        }
        else if (jugador->getTipo() == PERSONAJE_DARK_MATTER) {
            tiempoIman = 1.55f;
            cooldownIman = 2.8f;
        }
        radioIman = jugador->getRadioPoder();
        jugador->activarImpulso(true);
        eventosSonido.push_back(SONIDO_ANILLO);
    }
}

void NivelPiscinaEntrenamiento::reiniciarIntento()
{
    if (nivelSuperado || nivelPerdido) {
        return;
    }

    puntaje = 0;
    errorEntrada = 0.0f;
    monedasRecolectadas = 0;
    proyectilesDron.clear();
    intentoTerminado = false;
    intentoGanado = false;
    jugadorEnZonaViento = false;
    frenandoCaida = false;
    acelerandoCaida = false;
    tiempoReinicioIntento = 0.0f;
    tiempoIman = 0.0f;
    cooldownIman = 0.0f;
    tiempoCorreccionLateral = 0.0f;
    piscina.moveLeft(220.0f);
    zonaMeta.moveCenter(QPointF(piscina.center().x(), zonaMeta.center().y()));
    piscinaVelocidad = 0.0f;
    piscinaAceleracion = 0.0f;
    dron->reiniciarMemoriaParcial();
    crearMonedas();

    jugador->setEnAire(false);
    jugador->setVX(0.0f);
    jugador->setVY(0.0f);
    jugador->activarImpulso(false);

    jugador->colocarEn(
        plataforma->getX() + plataforma->getAncho() / 2.0f - jugador->getAncho() / 2.0f,
        plataforma->getY() - jugador->getAlto() + 8.0f
        );
}

void NivelPiscinaEntrenamiento::reiniciarNivel()
{
    puntaje = 0;
    mejorPuntaje = 0;
    errorEntrada = 0.0f;
    monedasRecolectadas = 0;
    proyectilesDron.clear();

    intentoTerminado = false;
    intentoGanado = false;
    nivelSuperado = false;
    nivelPerdido = false;
    jugadorEnZonaViento = false;
    frenandoCaida = false;
    acelerandoCaida = false;
    tiempoReinicioIntento = 0.0f;
    tiempoNivel = 0.0f;
    camaraY = 0.0f;
    tiempoIman = 0.0f;
    cooldownIman = 0.0f;
    tiempoCorreccionLateral = 0.0f;
    piscina.moveLeft(220.0f);
    zonaMeta.moveCenter(QPointF(piscina.center().x(), zonaMeta.center().y()));
    piscinaVelocidad = 0.0f;
    piscinaAceleracion = 0.0f;
    dron->reiniciarMemoriaParcial();

    intentosRestantes = dificultad.getIntentosMaximos();

    jugador->setEnAire(false);
    jugador->setVX(0.0f);
    jugador->setVY(0.0f);
    jugador->activarImpulso(false);
    jugador->setEnergiaMaxima(dificultad.getEnergiaInicial());
    plataforma->colocarEn(-8.0f, 165.0f);
    crearMonedas();

    jugador->colocarEn(
        plataforma->getX() + plataforma->getAncho() / 2.0f - jugador->getAncho() / 2.0f,
        plataforma->getY() - jugador->getAlto() + 8.0f
        );
}

void NivelPiscinaEntrenamiento::configurarPersonaje(TipoPersonaje tipo)
{
    jugador->configurarTipo(tipo);
    radioIman = jugador->getRadioPoder();
    reiniciarNivel();
}

int NivelPiscinaEntrenamiento::getPuntaje() const
{
    return puntaje;
}

bool NivelPiscinaEntrenamiento::estaSuperado() const
{
    return nivelSuperado;
}

bool NivelPiscinaEntrenamiento::estaPerdido() const
{
    return nivelPerdido;
}

QString NivelPiscinaEntrenamiento::nombreNivel() const
{
    return "Piscina de entrenamiento";
}

QVector<EventoSonidoJuego> NivelPiscinaEntrenamiento::consumirEventosSonido()
{
    QVector<EventoSonidoJuego> eventos = eventosSonido;
    eventosSonido.clear();
    return eventos;
}
