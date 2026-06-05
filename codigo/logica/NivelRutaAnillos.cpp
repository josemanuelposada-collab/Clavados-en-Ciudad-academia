#include "NivelRutaAnillos.h"
#include "../fisicas/ModelosFisicos.h"
#include "JuegoException.h"
#include <QKeyEvent>
#include <QLinearGradient>
#include <QRandomGenerator>
#include <algorithm>
#include <cmath>

namespace {
constexpr std::size_t MAX_PROYECTILES_DRON = 12;

void panel(QPainter& painter, const QRectF& rect, const QColor& color)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRect(rect);
    painter.setBrush(QColor(255, 225, 95, 190));
    painter.drawRect(QRectF(rect.x(), rect.y(), 5, rect.height()));
    painter.setPen(QPen(QColor(110, 230, 255, 88), 1));
    painter.drawLine(rect.topLeft(), rect.topRight());
    painter.drawLine(rect.bottomLeft(), rect.bottomRight());
}

void barra(QPainter& painter, const QRectF& rect, float porcentaje, const QColor& color)
{
    porcentaje = std::clamp(porcentaje, 0.0f, 1.0f);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 44));
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

NivelRutaAnillos::NivelRutaAnillos()
    : jugador(std::make_unique<Personaje>()),
      dron(std::make_unique<DronVigilante>(385.0f, 160.0f)),
      teclaIzquierda(false),
      teclaDerecha(false),
      teclaArriba(false),
      teclaAbajo(false),
      piscinaFinal(210.0f, 5850.0f, 380.0f, 105.0f),
      zonaViento(76.0f, 1060.0f, 648.0f, 640.0f),
      zonaVelocidad(96.0f, 3260.0f, 608.0f, 520.0f),
      anillosRecolectados(0),
      monedasRecolectadas(0),
      golpes(0),
      puntaje(0),
      tiempoRestante(48.0f),
      tiempoTotal(48.0f),
      tiempoNivel(0.0f),
      alturaMundo(6120.0f),
      camaraY(0.0f),
      velocidadVertical(0.0f),
      velocidadHorizontal(0.0f),
      tiempoEntradaAgua(0.0f),
      xEntradaAgua(0.0f),
      velocidadEntradaAgua(0.0f),
      tiempoIman(0.0f),
      cooldownIman(0.0f),
      radioIman(180.0f),
      piscinaVelocidad(0.0f),
      piscinaAceleracion(0.0f),
      tiempoProximaRafaga(1.6f),
      tiempoRafaga(0.0f),
      intensidadRafaga(0.0f),
      direccionRafaga(1.0f),
      yRafaga(900.0f),
      tiempoEntradaDron(0.0f),
      tiempoCorreccionLateral(0.0f),
      tiempoInvulnerable(0.0f),
      calidadEntrada(0),
      nivelSuperado(false),
      nivelPerdido(false),
      entradaAguaActiva(false),
      resultadoEntradaSuperado(false),
      saltoInicialPendiente(true),
      dronActivo(false),
      motivoDerrotaActual("")
{
    dificultad.configurar(NORMAL);
    spriteAlarma.load(":/recursos/sprites/alarma.png");
    spriteViento.load(":/recursos/sprites/viento_turbulencia.png");
    spriteVientoDerecha.load(":/recursos/sprites/velocidad_derecha.png");
    spriteVientoIzquierda.load(":/recursos/sprites/velocidad_izquierda.png");
    spritePiscinaFinal.load(":/recursos/sprites/piscina_ciudad_academia.png");
    spriteColumnaTorre.load(":/recursos/sprites/torre_columna_luz.png");
    spriteFondo.load(":/recursos/sprites/fondo_vertical_torre.png");
    spriteBanderines.load(":/recursos/sprites/banderines_academia.png");
    spriteEdificio.load(":/recursos/sprites/edificio_cristal_academia.png");
    spriteTorreEpica.load(":/recursos/sprites/torre_salto_epica.png");
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
    spriteAdvertenciaHud.load(":/recursos/sprites/hud_advertencia.png");
    spriteCorazonLleno.load(":/recursos/sprites/hud_corazon_lleno.png");
    spriteCorazonVacio.load(":/recursos/sprites/hud_corazon_vacio.png");
    spritePlataformaInicio.load(":/recursos/sprites/plataforma_media.png");
    spriteAlrededorPiscina.load(":/recursos/sprites/alrededor_piscina.jpg");
    exigirSprite(spriteFondo, "fondo_vertical_torre.png");
    exigirSprite(spritePiscinaPremium, "piscina_final_premium.png");
    exigirSprite(spriteCorazonLleno, "hud_corazon_lleno.png");
    exigirSprite(spriteCorazonVacio, "hud_corazon_vacio.png");
    jugador->colocarEn(82.0f, 128.0f);
    jugador->setEnAire(false);
    jugador->setVY(0.0f);
    proyectilesDron.reserve(MAX_PROYECTILES_DRON);
    crearEntidades();
    crearMonedas();
}

NivelRutaAnillos::~NivelRutaAnillos() = default;

void NivelRutaAnillos::crearEntidades()
{
    anillos.clear();
    obstaculos.clear();

    float factor = dificultad.getVelocidadPlataforma();
    anillos.reserve(9);
    obstaculos.reserve(8);
    obstaculos.emplace_back(std::make_unique<Obstaculo>(130.0f, 720.0f, 54.0f, 54.0f, 52.0f * factor, 0.0f, BOYA));
    obstaculos.emplace_back(std::make_unique<Obstaculo>(575.0f, 1380.0f, 58.0f, 58.0f, -64.0f * factor, 0.0f, BARRIL));
    obstaculos.emplace_back(std::make_unique<Obstaculo>(210.0f, 2050.0f, 54.0f, 54.0f, 72.0f * factor, 0.0f, MINA));
    obstaculos.emplace_back(std::make_unique<Obstaculo>(560.0f, 2700.0f, 56.0f, 56.0f, -78.0f * factor, 0.0f, BLOQUEO));
    obstaculos.emplace_back(std::make_unique<Obstaculo>(120.0f, 3460.0f, 54.0f, 54.0f, 70.0f * factor, 0.0f, BOYA));
    obstaculos.emplace_back(std::make_unique<Obstaculo>(610.0f, 4120.0f, 58.0f, 58.0f, -74.0f * factor, 0.0f, BARRIL));
    obstaculos.emplace_back(std::make_unique<Obstaculo>(230.0f, 4860.0f, 54.0f, 54.0f, 82.0f * factor, 0.0f, MINA));
    obstaculos.emplace_back(std::make_unique<Obstaculo>(560.0f, 5360.0f, 56.0f, 56.0f, -86.0f * factor, 0.0f, BLOQUEO));

    if (obstaculos.empty()) {
        throw JuegoException("La torre experimental no pudo crear sus entidades.");
    }
}

void NivelRutaAnillos::crearMonedas()
{
    monedas.clear();
    monedas.reserve(14);
    monedas.emplace_back(std::make_unique<Moneda>(440.0f, 620.0f));
    monedas.emplace_back(std::make_unique<Moneda>(240.0f, 1040.0f));
    monedas.emplace_back(std::make_unique<Moneda>(600.0f, 1460.0f));
    monedas.emplace_back(std::make_unique<Moneda>(310.0f, 1900.0f));
    monedas.emplace_back(std::make_unique<Moneda>(520.0f, 2360.0f));
    monedas.emplace_back(std::make_unique<Moneda>(180.0f, 2820.0f));
    monedas.emplace_back(std::make_unique<Moneda>(620.0f, 3260.0f));
    monedas.emplace_back(std::make_unique<Moneda>(360.0f, 3710.0f));
    monedas.emplace_back(std::make_unique<Moneda>(205.0f, 4190.0f));
    monedas.emplace_back(std::make_unique<Moneda>(560.0f, 4660.0f));
    monedas.emplace_back(std::make_unique<Moneda>(300.0f, 5150.0f));
    monedas.emplace_back(std::make_unique<Moneda>(470.0f, 5580.0f));
}

void NivelRutaAnillos::liberarEntidades()
{
    anillos.clear();
    monedas.clear();
    obstaculos.clear();
}

void NivelRutaAnillos::actualizar(float dt)
{
    if (nivelSuperado || nivelPerdido) {
        return;
    }

    tiempoNivel += dt;
    tiempoInvulnerable = std::max(0.0f, tiempoInvulnerable - dt);

    if (entradaAguaActiva) {
        tiempoEntradaAgua += dt;
        if (tiempoEntradaAgua >= 2.45f) {
            nivelSuperado = resultadoEntradaSuperado;
            nivelPerdido = !resultadoEntradaSuperado;
            if (nivelPerdido) {
                motivoDerrotaActual = QString("Entrada insuficiente: %1 pts. Minimo requerido: %2.")
                                          .arg(puntaje)
                                          .arg(dificultad.getPuntajeMinimo());
            }
        }
        return;
    }

    cooldownIman = std::max(0.0f, cooldownIman - dt);
    tiempoIman = std::max(0.0f, tiempoIman - dt);
    if (tiempoIman <= 0.0f) {
        jugador->activarImpulso(false);
    }

    if (saltoInicialPendiente && !jugador->estaEnAire()) {
        actualizarCamara();
        return;
    }

    tiempoRestante -= dt;

    actualizarPiscina(dt);
    aplicarMovimientoJugador(dt);
    if (jugador->estaEnAire() && jugador->estaCorrigiendoLateral()) {
        float cercaniaEntrada = std::clamp(static_cast<float>((jugador->getY() - (piscinaFinal.y() - 980.0f)) / 980.0f), 0.12f, 1.0f);
        tiempoCorreccionLateral += dt * cercaniaEntrada;
    }
    actualizarMonedas(dt);

    for (const auto& anillo : anillos) {
        anillo->actualizar(dt);
    }

    for (const auto& obstaculo : obstaculos) {
        obstaculo->actualizar(dt);
        obstaculo->rebotarHorizontal(76.0f, 724.0f);
    }

    tiempoEntradaDron += dt;
    constexpr float RETARDO_ENTRADA_DRON = 2.25f;
    constexpr float DURACION_ENTRADA_DRON = 0.95f;
    if (tiempoEntradaDron >= RETARDO_ENTRADA_DRON) {
        float progresoEntrada = std::clamp((tiempoEntradaDron - RETARDO_ENTRADA_DRON) / DURACION_ENTRADA_DRON, 0.0f, 1.0f);
        float yDron = std::clamp(jugador->getY() - 145.0f, 95.0f, alturaMundo - 250.0f);
        float xObjetivo = std::clamp(jugador->getX() + 240.0f, 520.0f, 705.0f);
        float xEntrada = 880.0f + (xObjetivo - 880.0f) * progresoEntrada;

        if (progresoEntrada < 1.0f) {
            dron->colocarEn(xEntrada, yDron);
        }
        else {
            dron->colocarY(yDron);
        }
        dronActivo = progresoEntrada >= 1.0f;

        if (dronActivo) {
            dron->actualizar(dt, *jugador);
            if (dron->solicitarDisparo(dt, *jugador)) {
                crearProyectilDron();
            }
        }
    }
    actualizarProyectilesDron(dt);

    verificarInteracciones();
    calcularPuntaje();
    actualizarCamara();

    if (!entradaAguaActiva && (tiempoRestante <= 0.0f || golpes >= 4)) {
        motivoDerrotaActual = tiempoRestante <= 0.0f
                                   ? "Se acabo el tiempo antes de completar la ruta."
                                   : "Recibiste demasiados impactos del dron u obstaculos.";
        nivelPerdido = true;
    }
}

void NivelRutaAnillos::actualizarMonedas(float dt)
{
    const QPointF objetivo = jugador->centro();
    const QRectF hitboxJugador = jugador->hitboxAjustada();
    const TipoPersonaje tipoJugador = jugador->getTipo();
    const float atraccionMonedas = jugador->getAtraccionMonedas();
    const bool imanActivo = tiempoIman > 0.0f;

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

        if (imanActivo) {
            if (tipoJugador == PERSONAJE_ACCELERATOR && dentroDelRadio(radioIman * 1.18f)) {
                moneda->desplazarVectorialHacia(objetivo, atraccionMonedas * 1.35f, radioIman * 1.18f, dt);
            }
            else if (tipoJugador == PERSONAJE_MUGINO) {
                QPointF foco(objetivo.x(), objetivo.y());
                moneda->canalizarMeltdowner(foco, atraccionMonedas * 1.18f, radioIman * 0.62f, dt);
            }
            else if (tipoJugador == PERSONAJE_DARK_MATTER && dentroDelRadio(radioIman * 1.12f)) {
                moneda->orbitarHacia(objetivo, atraccionMonedas * 1.05f, radioIman * 1.12f, dt);
            }
            else if (dentroDelRadio(radioIman)) {
                moneda->atraerHacia(objetivo, atraccionMonedas, radioIman, dt);
            }
        }

        if (contraer(moneda->rect(), 7.0f, 7.0f).intersects(hitboxJugador)) {
            moneda->recolectar();
            monedasRecolectadas++;
            eventosSonido.push_back(SONIDO_ANILLO);
        }
    }
}

void NivelRutaAnillos::aplicarMovimientoJugador(float dt)
{
    if (!jugador->estaEnAire()) {
        jugador->colocarEn(82.0f, 128.0f);
        jugador->setVX(0.0f);
        jugador->setVY(0.0f);
        return;
    }

    float gravedad = (dificultad.getTipo() == FACIL ? 30.0f : dificultad.getTipo() == NORMAL ? 38.0f : 48.0f) *
                    dificultad.getFactorGravedad() * jugador->getFactorGravedad();
    float controlHorizontal = 0.0f;

    if (teclaIzquierda) {
        controlHorizontal -= 560.0f;
    }
    if (teclaDerecha) {
        controlHorizontal += 560.0f;
    }
    if (teclaArriba) {
        velocidadVertical = std::max(36.0f, velocidadVertical - 150.0f * dt);
    }
    if (teclaAbajo) {
        velocidadVertical += 145.0f * dt;
    }

    velocidadHorizontal += controlHorizontal * jugador->getControlLateral() * dt;
    velocidadHorizontal *= 0.970f * jugador->getFactorArrastre() / 0.995f;
    FisicaJuego::integrarVelocidad(velocidadVertical, gravedad, dt);

    if (zonaViento.intersects(jugador->rect())) {
        float direccionVientoZona = std::sin(tiempoNivel * 0.45f) >= 0.0f ? 1.0f : -1.0f;
        float turbulencia = dificultad.getIntensidadViento() * (1.15f + 0.42f * std::sin(tiempoNivel * 3.4f));
        velocidadHorizontal += direccionVientoZona * turbulencia * dt;
    }

    tiempoProximaRafaga -= dt;
    if (tiempoRafaga <= 0.0f && tiempoProximaRafaga <= 0.0f) {
        auto aleatorio = [](float minimo, float maximo) {
            return minimo + static_cast<float>(QRandomGenerator::global()->generateDouble()) * (maximo - minimo);
        };
        direccionRafaga = QRandomGenerator::global()->bounded(2) == 0 ? -1.0f : 1.0f;
        intensidadRafaga = aleatorio(260.0f, 520.0f) * dificultad.getFactorRafagas();
        tiempoRafaga = aleatorio(0.85f, 1.55f);
        tiempoProximaRafaga = aleatorio(2.25f, 4.15f);
        yRafaga = std::clamp(jugador->getY() + aleatorio(-180.0f, 260.0f), 360.0f, alturaMundo - 520.0f);
    }

    if (tiempoRafaga > 0.0f) {
        tiempoRafaga -= dt;
        float distanciaY = std::abs(jugador->centro().y() - yRafaga);
        float influencia = std::clamp(1.0f - distanciaY / 320.0f, 0.25f, 1.0f);
        FisicaJuego::integrarVelocidad(velocidadHorizontal, direccionRafaga * intensidadRafaga * influencia, dt);
        velocidadVertical *= 0.998f;
    }

    if (zonaVelocidad.intersects(jugador->rect())) {
        velocidadVertical *= 0.996f;
    }

    if (tiempoIman > 0.0f) {
        if (jugador->getTipo() == PERSONAJE_ACCELERATOR) {
            velocidadHorizontal *= 0.92f;
        }
        else if (jugador->getTipo() == PERSONAJE_MUGINO) {
            velocidadHorizontal += (teclaIzquierda ? -92.0f : 92.0f) * dt;
        }
        else if (jugador->getTipo() == PERSONAJE_DARK_MATTER) {
            velocidadVertical *= 0.988f;
            velocidadHorizontal *= 0.985f;
        }
    }

    if (tiempoEntradaDron > 0.85f && velocidadVertical < 24.0f) {
        velocidadVertical = 24.0f;
    }
    velocidadVertical = std::clamp(velocidadVertical, -90.0f, 860.0f);
    velocidadHorizontal = std::clamp(velocidadHorizontal, -430.0f, 430.0f);

    float nuevoX = jugador->getX();
    float nuevoY = jugador->getY();
    FisicaJuego::integrarPosicion(nuevoX, velocidadHorizontal, dt);
    FisicaJuego::integrarPosicion(nuevoY, velocidadVertical, dt);
    nuevoX = std::clamp(nuevoX, 40.0f, 725.0f);
    nuevoY = std::clamp(nuevoY, 82.0f, alturaMundo - 62.0f);
    if (nuevoY <= 82.0f && velocidadVertical < 0.0f) {
        velocidadVertical = 0.0f;
    }
    jugador->colocarEn(nuevoX, nuevoY);
    jugador->setVX(velocidadHorizontal);
    jugador->setVY(velocidadVertical);
}

void NivelRutaAnillos::verificarInteracciones()
{
    for (const auto& anillo : anillos) {
        if (!anillo->estaRecolectado() && contraer(anillo->rect(), 8.0f, 8.0f).intersects(jugador->hitboxAjustada())) {
            anillo->recolectar();
            anillosRecolectados++;
            dron->aprender(35.0f);
            eventosSonido.push_back(SONIDO_ANILLO);
        }
    }

    const QRectF hitboxJugador = jugador->hitboxAjustada();
    const bool puedeRecibirImpacto = tiempoInvulnerable <= 0.0f;

    for (const auto& obstaculo : obstaculos) {
        if (puedeRecibirImpacto && contraer(obstaculo->rect(), 11.0f, 11.0f).intersects(hitboxJugador)) {
            if (tiempoIman > 0.0f && jugador->getTipo() == PERSONAJE_ACCELERATOR) {
                velocidadHorizontal += jugador->centro().x() < obstaculo->centro().x() ? -175.0f : 175.0f;
                eventosSonido.push_back(SONIDO_ANILLO);
                return;
            }
            golpes++;
            tiempoRestante = std::max(0.0f, tiempoRestante - 2.5f);
            velocidadHorizontal *= -0.45f;
            velocidadVertical = std::max(120.0f, velocidadVertical * 0.72f);
            dron->aprender(165.0f);
            tiempoInvulnerable = 1.0f;
            jugador->colocarEn(std::clamp(jugador->getX() + 55.0f, 45.0f, 720.0f), jugador->getY() - 35.0f);
            eventosSonido.push_back(SONIDO_COLISION);
            return;
        }
    }

    if (puedeRecibirImpacto && dronActivo && contraer(dron->rect(), 10.0f, 10.0f).intersects(hitboxJugador)) {
        golpes++;
        tiempoRestante = std::max(0.0f, tiempoRestante - 3.0f);
        velocidadHorizontal += jugador->centro().x() < dron->centro().x() ? -160.0f : 160.0f;
        tiempoInvulnerable = 1.0f;
        eventosSonido.push_back(SONIDO_COLISION);
    }

    if (piscinaFinal.intersects(hitboxJugador)) {
        registrarEntradaAgua();
        return;
    }

    QRectF hitbox = hitboxJugador;
    bool pasoLaPiscina = hitbox.top() > piscinaFinal.bottom() + 24.0f;
    bool llegoAlFondo = hitbox.bottom() >= alturaMundo - 18.0f;
    if ((pasoLaPiscina || llegoAlFondo) && !entradaAguaActiva) {
        calcularPuntaje();
        puntaje = std::max(0, puntaje - 30);
        golpes = 4;
        motivoDerrotaActual = pasoLaPiscina
                                  ? "Pasaste por fuera de la piscina final."
                                  : "Llegaste al limite inferior sin entrada valida al agua.";
        nivelPerdido = true;
        jugador->detenerMovimiento();
        proyectilesDron.clear();
        dron->aprender(180.0f);
        eventosSonido.push_back(SONIDO_COLISION);
    }
}

void NivelRutaAnillos::crearProyectilDron()
{
    bool elastico = dron->getEstado() != INTERCEPTA;
    float rapidezJugador = std::sqrt(FisicaJuego::rapidezCuadrada(jugador->getVX(), jugador->getVY()));
    float factorCaida = std::clamp(jugador->getY() / (alturaMundo - 650.0f), 0.0f, 1.0f);
    float rapidezBase = (elastico ? 350.0f : 330.0f) * dificultad.getFactorAgente();
    float rapidez = std::clamp(rapidezBase + rapidezJugador * (0.38f + factorCaida * 0.30f), 430.0f, 940.0f);
    QPointF velocidad = dron->calcularVectorDisparo(*jugador, rapidez);

    for (const auto& proyectil : proyectilesDron) {
        if (!proyectil->estaActivo()) {
            proyectil->reiniciar(dron->centro().x() - 12.0f,
                                 dron->centro().y() - 12.0f,
                                 static_cast<float>(velocidad.x()),
                                 static_cast<float>(velocidad.y()),
                                 elastico);
            return;
        }
    }

    if (proyectilesDron.size() >= MAX_PROYECTILES_DRON) {
        return;
    }

    proyectilesDron.emplace_back(std::make_unique<ProyectilDron>(
        dron->centro().x() - 12.0f,
        dron->centro().y() - 12.0f,
        static_cast<float>(velocidad.x()),
        static_cast<float>(velocidad.y()),
        elastico
        ));
}

void NivelRutaAnillos::actualizarProyectilesDron(float dt)
{
    const QRectF hitboxJugador = jugador->hitboxAjustada();

    for (const auto& proyectil : proyectilesDron) {
        bool estabaActivo = proyectil->estaActivo();
        proyectil->actualizar(dt);

        if (tiempoInvulnerable <= 0.0f && proyectil->estaActivo() && contraer(proyectil->rect(), 3.0f, 3.0f).intersects(hitboxJugador)) {
            resolverColisionProyectil(*proyectil);
            proyectil->desactivar();
            golpes += proyectil->usaColisionElastica() ? 0 : 1;
            tiempoRestante = std::max(0.0f, tiempoRestante - (proyectil->usaColisionElastica() ? 0.8f : 1.8f));
            dron->aprender(170.0f);
            dron->registrarImpactoJugador();
            tiempoInvulnerable = 1.0f;
            eventosSonido.push_back(SONIDO_COLISION);
        }
        else if (estabaActivo && !proyectil->estaActivo()) {
            dron->registrarEvasionJugador();
        }
    }
}

void NivelRutaAnillos::resolverColisionProyectil(ProyectilDron& proyectil)
{
    auto choqueX = proyectil.usaColisionElastica()
                       ? FisicaJuego::resolverChoqueElastico(jugador->getMasa(), velocidadHorizontal,
                                                             proyectil.getMasa(), proyectil.getVX())
                       : FisicaJuego::resolverChoqueInelastico(jugador->getMasa(), velocidadHorizontal,
                                                               proyectil.getMasa(), proyectil.getVX());
    auto choqueY = proyectil.usaColisionElastica()
                       ? FisicaJuego::resolverChoqueElastico(jugador->getMasa(), velocidadVertical,
                                                             proyectil.getMasa(), proyectil.getVY())
                       : FisicaJuego::resolverChoqueInelastico(jugador->getMasa(), velocidadVertical,
                                                               proyectil.getMasa(), proyectil.getVY());

    velocidadHorizontal = std::clamp(choqueX.velocidadA, -270.0f, 270.0f);
    velocidadVertical = std::clamp(choqueY.velocidadA, 88.0f, 245.0f);
    jugador->setVX(velocidadHorizontal);
    jugador->setVY(velocidadVertical);
    proyectil.setVX(choqueX.velocidadB);
    proyectil.setVY(choqueY.velocidadB);
}

void NivelRutaAnillos::actualizarPiscina(float dt)
{
    float factor = dificultad.getFactorPiscinaMovil();
    piscinaAceleracion = std::sin(tiempoNivel * 0.72f + 1.4f) * 28.0f * factor;
    piscinaVelocidad += piscinaAceleracion * dt;
    piscinaVelocidad *= 0.993f;
    piscinaFinal.translate(piscinaVelocidad * dt, 0.0f);

    const float limiteIzq = 158.0f;
    const float limiteDer = 262.0f;
    if (piscinaFinal.x() < limiteIzq) {
        piscinaFinal.moveLeft(limiteIzq);
        piscinaVelocidad = std::abs(piscinaVelocidad) * 0.84f;
    }
    else if (piscinaFinal.x() > limiteDer) {
        piscinaFinal.moveLeft(limiteDer);
        piscinaVelocidad = -std::abs(piscinaVelocidad) * 0.84f;
    }
}

void NivelRutaAnillos::registrarEntradaAgua()
{
    calcularPuntaje();
    entradaAguaActiva = true;
    resultadoEntradaSuperado = puntaje >= dificultad.getPuntajeMinimo();
    tiempoEntradaAgua = 0.0f;
    xEntradaAgua = jugador->centro().x();
    velocidadEntradaAgua = std::abs(jugador->getVY());

    if (puntaje >= std::max(82, dificultad.getPuntajeMinimo() + 10)) {
        calidadEntrada = 2;
    }
    else if (puntaje >= dificultad.getPuntajeMinimo()) {
        calidadEntrada = 1;
    }
    else {
        calidadEntrada = 0;
    }

    jugador->detenerMovimiento();
    jugador->colocarEn(xEntradaAgua - jugador->getAncho() / 2.0f, piscinaFinal.y() + 34.0f);
    dron->aprender(100.0f - puntaje);
    eventosSonido.push_back(SONIDO_AGUA);

    if (resultadoEntradaSuperado) {
        dron->registrarAciertoJugador();
    }
}

void NivelRutaAnillos::calcularPuntaje()
{
    float centroPiscina = piscinaFinal.x() + piscinaFinal.width() / 2.0f;
    float errorEntrada = std::abs(jugador->centro().x() - centroPiscina);
    const float factorPrecision = dificultad.getTipo() == FACIL ? 0.15f :
                                  dificultad.getTipo() == NORMAL ? 0.19f :
                                                                    0.24f;
    int precision = static_cast<int>(std::clamp(40.0f - errorEntrada * factorPrecision, 0.0f, 40.0f));

    const float recorridoTotal = std::max(1.0f, static_cast<float>(piscinaFinal.y()) - 128.0f);
    float progreso = std::clamp((jugador->getY() - 128.0f) / recorridoTotal, 0.0f, 1.0f);
    int ruta = static_cast<int>(progreso * 38.0f);
    int bonusMonedas = std::min(26, monedasRecolectadas * 2) + (monedasRecolectadas >= 8 ? 5 : 0);
    int bonusRitmo = static_cast<int>(std::max(0.0f, tiempoRestante) * 0.12f);
    int bonusVelocidad = velocidadVertical >= 105.0f && velocidadVertical <= 285.0f ? 12 : 7;
    const bool evaluarTecnicaEntrada = entradaAguaActiva ||
                                       piscinaFinal.intersects(jugador->hitboxAjustada()) ||
                                       jugador->getY() >= piscinaFinal.y() - 420.0f ||
                                       nivelPerdido;
    const float factorCorreccion = dificultad.getTipo() == FACIL ? 4.5f :
                                   dificultad.getTipo() == NORMAL ? 6.0f :
                                                                     7.5f;
    int penalizacionCorreccion = evaluarTecnicaEntrada
                                     ? static_cast<int>(std::clamp(tiempoCorreccionLateral * factorCorreccion, 0.0f, 14.0f))
                                     : 0;
    int penalizacionGolpes = golpes * (dificultad.getTipo() == FACIL ? 5 : dificultad.getTipo() == NORMAL ? 7 : 9);
    int penalizacion = penalizacionGolpes + penalizacionCorreccion;
    int calculado = ruta + bonusMonedas + precision + bonusRitmo + bonusVelocidad - penalizacion;

    if (entradaAguaActiva || jugador->getY() >= piscinaFinal.y() - 220.0f) {
        const int pisoFinal = dificultad.getTipo() == FACIL ? 42 : dificultad.getTipo() == NORMAL ? 34 : 24;
        calculado = std::max(calculado, pisoFinal - penalizacionGolpes);
    }

    puntaje = std::clamp(calculado, 0, 100);
}

void NivelRutaAnillos::actualizarCamara()
{
    camaraY = std::clamp(jugador->getY() - 255.0f, 0.0f, alturaMundo - 600.0f);
}

void NivelRutaAnillos::dibujarPiscinaFinal(QPainter& painter)
{
    if (!spriteAguaProfunda.isNull()) {
        painter.drawPixmap(QRect(static_cast<int>(piscinaFinal.x() - 14.0f), static_cast<int>(piscinaFinal.y() + 12.0f), 408, 185), spriteAguaProfunda);
    }

    QRectF exteriorPiscina(piscinaFinal.x() - 165.0f, piscinaFinal.y() - 112.0f,
                           piscinaFinal.width() + 330.0f, 310.0f);
    QRectF huecoPiscina(piscinaFinal.x() - 28.0f, piscinaFinal.y() - 18.0f,
                        piscinaFinal.width() + 56.0f, 162.0f);
    dibujarPisoAlrededorPiscina(painter, spriteAlrededorPiscina, exteriorPiscina, huecoPiscina);
    if (spriteAlrededorPiscina.isNull() && !spriteTexturaTierra.isNull()) {
        painter.fillRect(QRectF(piscinaFinal.x() - 96.0f, piscinaFinal.y() - 18.0f,
                                piscinaFinal.width() + 192.0f, 190.0f),
                         QBrush(spriteTexturaTierra));
        painter.fillRect(QRectF(piscinaFinal.x() - 96.0f, piscinaFinal.y() - 18.0f,
                                piscinaFinal.width() + 192.0f, 190.0f),
                         QColor(8, 42, 36, 72));
    }
    if (!spriteDecoracionIzq.isNull()) {
        painter.drawPixmap(QRect(static_cast<int>(piscinaFinal.x() - 126.0f),
                                 static_cast<int>(piscinaFinal.y() - 58.0f), 128, 180), spriteDecoracionIzq);
    }
    if (!spriteDecoracionDer.isNull()) {
        painter.drawPixmap(QRect(static_cast<int>(piscinaFinal.right() - 4.0f),
                                 static_cast<int>(piscinaFinal.y() - 58.0f), 128, 180), spriteDecoracionDer);
    }

    if (!spritePiscinaPremium.isNull()) {
        painter.drawPixmap(QRect(static_cast<int>(piscinaFinal.x() - 60.0f), static_cast<int>(piscinaFinal.y() - 58.0f), 500, 212), spritePiscinaPremium);
    }
    else if (!spritePiscinaFinal.isNull()) {
        painter.drawPixmap(QRect(static_cast<int>(piscinaFinal.x() - 40.0f), static_cast<int>(piscinaFinal.y() - 45.0f), 460, 158), spritePiscinaFinal);
    }
    else {
        QLinearGradient agua(piscinaFinal.topLeft(), piscinaFinal.bottomLeft());
        agua.setColorAt(0.0, QColor(95, 225, 255));
        agua.setColorAt(1.0, QColor(0, 94, 190));
        painter.setPen(QPen(QColor(210, 250, 255), 2));
        painter.setBrush(agua);
        painter.drawRoundedRect(piscinaFinal, 10, 10);
    }

    painter.setPen(QPen(QColor(255, 255, 255, 95), 2));
    for (int i = 0; i < 6; ++i) {
        float xLinea = piscinaFinal.x() + 35.0f + i * 61.0f;
        painter.drawLine(QPointF(xLinea, piscinaFinal.y() + 14.0f),
                         QPointF(xLinea + 18.0f, piscinaFinal.y() + piscinaFinal.height() - 10.0f));
    }
}

void NivelRutaAnillos::dibujarEntradaAgua(QPainter& painter)
{
    QRectF clipAgua(piscinaFinal.x() - 18.0f, piscinaFinal.y() - 20.0f,
                    piscinaFinal.width() + 36.0f, 200.0f);
    painter.save();
    painter.setClipRect(clipAgua);

    if (!spriteBurbujas.isNull()) {
        int burbujaX = static_cast<int>(xEntradaAgua - 88.0f + std::sin(tiempoEntradaAgua * 4.0f) * 9.0f);
        painter.drawPixmap(QRect(burbujaX, static_cast<int>(piscinaFinal.y() + 12.0f), 176, 190), spriteBurbujas);
    }

    painter.setOpacity(0.48);
    float hundimiento = std::min(95.0f, tiempoEntradaAgua * 46.0f);
    const float xOriginal = jugador->getX();
    const float yOriginal = jugador->getY();
    jugador->colocarEn(xEntradaAgua - jugador->getAncho() / 2.0f, piscinaFinal.y() + 30.0f + hundimiento);
    jugador->dibujar(painter);
    jugador->colocarEn(xOriginal, yOriginal);
    painter.setOpacity(1.0);
    painter.restore();

    const QPixmap* splash = &spriteChapuzonFuerte;
    if (calidadEntrada == 2) {
        splash = &spriteChapuzonLimpio;
    }
    else if (calidadEntrada == 1) {
        splash = &spriteChapuzonMedio;
    }

    if (!splash->isNull() && tiempoEntradaAgua < 1.55f) {
        float pulso = 1.0f + tiempoEntradaAgua * 0.38f;
        int anchoSplash = static_cast<int>((calidadEntrada == 2 ? 210 : calidadEntrada == 1 ? 250 : 290) * pulso);
        int altoSplash = static_cast<int>((calidadEntrada == 2 ? 126 : calidadEntrada == 1 ? 150 : 172) * pulso);
        painter.setOpacity(std::max(0.0f, 1.0f - tiempoEntradaAgua / 1.55f));
        painter.drawPixmap(QRect(static_cast<int>(xEntradaAgua - anchoSplash / 2.0f),
                                 static_cast<int>(piscinaFinal.y() - 92.0f),
                                 anchoSplash,
                                 altoSplash),
                           *splash);
        painter.setOpacity(1.0);
    }
}

void NivelRutaAnillos::dibujarIman(QPainter& painter)
{
    QColor color = jugador->getColorPoder();
    QColor relleno = color;
    color.setAlpha(160);
    relleno.setAlpha(24);

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

void NivelRutaAnillos::dibujarProyectilesDron(QPainter& painter)
{
    for (const auto& proyectil : proyectilesDron) {
        if (proyectil->estaActivo() && estaVisible(proyectil->rect())) {
            proyectil->dibujar(painter);
        }
    }
}

bool NivelRutaAnillos::estaVisible(const QRectF& rect) const
{
    QRectF vista(0.0f, camaraY - 140.0f, 800.0f, 880.0f);
    return rect.intersects(vista);
}

void NivelRutaAnillos::dibujarEscenario(QPainter& painter)
{
    const QRectF visible(0.0f, camaraY, 800.0f, 600.0f);

    if (!spriteFondo.isNull()) {
        float proporcionY = std::clamp(camaraY / std::max(1.0f, alturaMundo - 600.0f), 0.0f, 1.0f);
        int fuenteY = static_cast<int>(proporcionY * std::max(0, spriteFondo.height() - 600));
        painter.drawPixmap(QRect(0, static_cast<int>(camaraY), 800, 600),
                           spriteFondo,
                           QRect(0, fuenteY, spriteFondo.width(), std::min(600, spriteFondo.height() - fuenteY)));
        painter.fillRect(visible, QColor(6, 20, 36, 70));
        if (!spriteTexturaPixel.isNull()) {
            painter.setOpacity(0.24);
            painter.fillRect(visible, QBrush(spriteTexturaPixel));
            painter.setOpacity(1.0);
        }
    }
    else {
        QLinearGradient fondo(0, camaraY, 0, camaraY + 600);
        fondo.setColorAt(0.0, QColor(18, 40, 74));
        fondo.setColorAt(0.55, QColor(28, 125, 166));
        fondo.setColorAt(1.0, QColor(12, 35, 64));
        painter.fillRect(QRectF(0, camaraY, 800, 600), fondo);
    }

    if (estaVisible(zonaViento)) {
        painter.setPen(QPen(QColor(130, 230, 255, 145), 2, Qt::DashLine));
        painter.setBrush(QColor(80, 205, 255, 38));
        painter.drawRoundedRect(zonaViento, 12, 12);
        float direccionZona = std::sin(tiempoNivel * 0.45f) >= 0.0f ? 1.0f : -1.0f;
        const QPixmap& spriteZona = direccionZona > 0.0f ? spriteVientoDerecha : spriteVientoIzquierda;
        if (!spriteZona.isNull()) {
            painter.setOpacity(0.7);
            for (int y = static_cast<int>(zonaViento.y()) + 76; y < zonaViento.bottom() - 70; y += 180) {
                QRect destino(288 + static_cast<int>(std::sin(tiempoNivel * 2.6f + y) * 26.0f), y, 210, 82);
                if (estaVisible(destino)) {
                    painter.drawPixmap(destino, spriteZona);
                }
            }
            painter.setOpacity(1.0);
        }
    }

    if (tiempoRafaga > 0.0f) {
        painter.save();
        painter.setOpacity(0.86);
        float baseY = yRafaga + std::sin(tiempoNivel * 12.0f) * 18.0f;
        const QPixmap& spriteRafaga = direccionRafaga > 0.0f ? spriteVientoDerecha : spriteVientoIzquierda;
        if (!spriteRafaga.isNull()) {
            for (int i = 0; i < 3; ++i) {
                QRect destino(direccionRafaga > 0.0f ? 184 + i * 118 : 392 - i * 118,
                              static_cast<int>(baseY - 66.0f + i * 34.0f),
                              220,
                              88);
                painter.drawPixmap(destino, spriteRafaga);
            }
        }
        else if (!spriteViento.isNull()) {
            QRect destino(295, static_cast<int>(baseY - 58.0f), 230, 92);
            painter.drawPixmap(destino, spriteViento);
        }
        painter.restore();
    }

    if (estaVisible(zonaVelocidad)) {
        painter.setPen(QPen(QColor(255, 230, 95, 150), 2, Qt::DashLine));
        painter.setBrush(QColor(255, 230, 95, 36));
        painter.drawRoundedRect(zonaVelocidad, 12, 12);
    }

    QRect plataformaInicio(44, 160, 110, 105);
    if (estaVisible(plataformaInicio)) {
        if (!spritePlataformaInicio.isNull()) {
            painter.drawPixmap(plataformaInicio, spritePlataformaInicio);
        }
        else {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(27, 126, 174));
            painter.drawRect(QRectF(24, 166, 196, 20));
            painter.setBrush(QColor(35, 58, 70));
            painter.drawRect(QRectF(36, 186, 34, 72));
        }
    }

    if (estaVisible(piscinaFinal.adjusted(-180.0, -130.0, 180.0, 240.0))) {
        dibujarPiscinaFinal(painter);
    }

    for (const auto& anillo : anillos) {
        if (!anillo->estaRecolectado() && estaVisible(anillo->rect())) {
            anillo->dibujar(painter);
        }
    }

    for (const auto& moneda : monedas) {
        if (!moneda->estaRecolectada() && estaVisible(moneda->rect())) {
            moneda->dibujar(painter);
        }
    }

    for (const auto& obstaculo : obstaculos) {
        if (estaVisible(obstaculo->rect())) {
            obstaculo->dibujar(painter);
        }
    }

    if (!saltoInicialPendiente && tiempoEntradaDron >= 2.25f && estaVisible(dron->rect())) {
        dron->dibujar(painter);
    }
    dibujarProyectilesDron(painter);
    if (tiempoIman > 0.0f) {
        dibujarIman(painter);
    }
    if (entradaAguaActiva) {
        dibujarEntradaAgua(painter);
    }
    else {
        jugador->dibujar(painter);
    }
}

void NivelRutaAnillos::dibujarHud(QPainter& painter)
{
    panel(painter, QRectF(18, 18, 334, 188), QColor(3, 12, 25, 204));

    QFont fuente = painter.font();
    fuente.setPointSize(12);
    fuente.setBold(true);
    painter.setFont(fuente);
    painter.setPen(QColor(238, 252, 255));
    painter.drawText(38, 46, "Torre monumental");

    fuente.setPointSize(8);
    fuente.setBold(false);
    painter.setFont(fuente);
    painter.setPen(QColor(185, 230, 246));
    painter.drawText(38, 70, saltoInicialPendiente ? "Espacio: salto parabolico desde la rampa" : "A/D corregir | E/W frenar | S acelerar");

    painter.setPen(QColor(255, 225, 95));
    painter.drawText(38, 100, "Tiempo");
    barra(painter, QRectF(112, 90, 190, 10), tiempoRestante / tiempoTotal, QColor(255, 225, 95));
    painter.drawText(310, 101, QString::number(tiempoRestante, 'f', 1));

    painter.setPen(QColor(120, 235, 255));
    painter.drawText(38, 126, "Descenso");
    barra(painter, QRectF(112, 116, 190, 10), jugador->getY() / (alturaMundo - 80.0f), QColor(120, 235, 255));

    painter.setPen(QColor(230, 245, 255));
    painter.drawText(38, 151, "Monedas: " + QString::number(monedasRecolectadas) + "/" + QString::number(monedas.size()) +
                               "   Golpes: " + QString::number(golpes) + "/4   Pts: " + QString::number(puntaje));

    painter.drawText(38, 178, "Vidas");
    int vidasRestantes = std::max(0, 4 - golpes);
    for (int i = 0; i < 4; ++i) {
        const QPixmap& corazon = i < vidasRestantes ? spriteCorazonLleno : spriteCorazonVacio;
        if (!corazon.isNull()) {
            painter.drawPixmap(QRect(90 + i * 28, 159, 24, 24), corazon);
        }
    }

    panel(painter, QRectF(548, 24, 232, 74), QColor(3, 12, 25, 174));
    painter.setPen(QColor(190, 236, 248));
    if ((tiempoRestante < 10.0f || zonaViento.intersects(jugador->rect())) && !spriteAdvertenciaHud.isNull()) {
        painter.drawPixmap(QRect(556, 41, 28, 28), spriteAdvertenciaHud);
    }
    painter.drawText((tiempoRestante < 10.0f || zonaViento.intersects(jugador->rect())) ? 588 : 568, 52, entradaAguaActiva ? "Analizando entrada" :
                                 tiempoRafaga > 0.0f ? (direccionRafaga > 0.0f ? "Rafaga hacia derecha" : "Rafaga hacia izquierda") :
                                 zonaViento.intersects(jugador->rect()) ? "Viento lateral activo" : "Aire estable");
    painter.drawText(568, 78, tiempoIman > 0.0f ? "Campo iman activo" :
                                cooldownIman > 0.0f ? "Iman recargando" :
                                zonaVelocidad.intersects(jugador->rect()) ? "Zona de velocidad variable" : jugador->getPoder());

    if (entradaAguaActiva) {
        QString evaluacion = calidadEntrada == 2 ? "Entrada limpia" : calidadEntrada == 1 ? "Entrada controlada" : "Mucho splash";
        panel(painter, QRectF(248, 458, 304, 64), QColor(3, 12, 25, 204));
        painter.setPen(QColor(255, 225, 95));
        painter.drawText(QRectF(270, 480, 260, 24), Qt::AlignCenter, evaluacion + "  |  " + QString::number(puntaje) + " pts");
    }
}

void NivelRutaAnillos::dibujar(QPainter& painter)
{
    painter.save();
    painter.translate(0, -camaraY);
    dibujarEscenario(painter);
    painter.restore();

    dibujarHud(painter);

    if (tiempoRestante < 10.0f && !spriteAlarma.isNull()) {
        painter.drawPixmap(QRect(706, 32, 54, 54), spriteAlarma);
    }

    if (nivelSuperado || nivelPerdido) {
        panel(painter, QRectF(196, 200, 430, 190), QColor(248, 252, 255, 240));
        QFont fuente = painter.font();
        fuente.setPointSize(14);
        fuente.setBold(true);
        painter.setFont(fuente);
        painter.setPen(QColor(18, 38, 54));
        painter.drawText(QRectF(220, 232, 382, 32), Qt::AlignCenter,
                         nivelSuperado ? "TORRE SUPERADA" : "TORRE FALLIDA");

        fuente.setPointSize(9);
        fuente.setBold(false);
        painter.setFont(fuente);
        painter.drawText(260, 286, "Puntaje final: " + QString::number(puntaje));
        painter.drawText(260, 314, "Monedas metalicas: " + QString::number(monedasRecolectadas));
        painter.drawText(260, 342, "Golpes recibidos: " + QString::number(golpes));
        painter.drawText(260, 370, "R reinicia el nivel");
    }
}

void NivelRutaAnillos::teclaPresionada(int tecla)
{
    if (tecla == Qt::Key_A || tecla == Qt::Key_Left) {
        teclaIzquierda = true;
        jugador->moverIzquierda(true);
    }
    if (tecla == Qt::Key_D || tecla == Qt::Key_Right) {
        teclaDerecha = true;
        jugador->moverDerecha(true);
    }
    if (tecla == Qt::Key_E || tecla == Qt::Key_Up || tecla == Qt::Key_W) {
        teclaArriba = true;
    }
    if (tecla == Qt::Key_S || tecla == Qt::Key_Down) {
        teclaAbajo = true;
    }

    if (tecla == Qt::Key_Space && saltoInicialPendiente && !jugador->estaEnAire()) {
        saltoInicialPendiente = false;
        dronActivo = false;
        tiempoEntradaDron = 0.0f;
        proyectilesDron.clear();
        dron->colocarEn(880.0f, std::clamp(jugador->getY() - 145.0f, 95.0f, alturaMundo - 250.0f));
        dron->reiniciarMemoriaParcial();
        jugador->setEnAire(true);
        velocidadHorizontal = 168.0f;
        velocidadVertical = -86.0f;
        jugador->setVX(velocidadHorizontal);
        jugador->setVY(velocidadVertical);
        eventosSonido.push_back(SONIDO_SALTO);
    }
    else if (tecla == Qt::Key_R) {
        reiniciarNivel();
    }
}

void NivelRutaAnillos::teclaLiberada(int tecla)
{
    if (tecla == Qt::Key_A || tecla == Qt::Key_Left) {
        teclaIzquierda = false;
        jugador->moverIzquierda(false);
    }
    if (tecla == Qt::Key_D || tecla == Qt::Key_Right) {
        teclaDerecha = false;
        jugador->moverDerecha(false);
    }
    if (tecla == Qt::Key_E || tecla == Qt::Key_Up || tecla == Qt::Key_W) {
        teclaArriba = false;
    }
    if (tecla == Qt::Key_S || tecla == Qt::Key_Down) {
        teclaAbajo = false;
    }
}

void NivelRutaAnillos::mousePresionado(const QPointF& posicion)
{
    Q_UNUSED(posicion);

    if (nivelSuperado || nivelPerdido || entradaAguaActiva) {
        return;
    }

    if (cooldownIman <= 0.0f) {
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

void NivelRutaAnillos::reiniciarNivel()
{
    teclaIzquierda = false;
    teclaDerecha = false;
    teclaArriba = false;
    teclaAbajo = false;
    anillosRecolectados = 0;
    monedasRecolectadas = 0;
    golpes = 0;
    puntaje = 0;
    tiempoRestante = tiempoTotal;
    tiempoNivel = 0.0f;
    camaraY = 0.0f;
    proyectilesDron.clear();
    velocidadHorizontal = 0.0f;
    velocidadVertical = 120.0f;
    tiempoEntradaAgua = 0.0f;
    tiempoIman = 0.0f;
    cooldownIman = 0.0f;
    tiempoProximaRafaga = 1.6f;
    tiempoRafaga = 0.0f;
    intensidadRafaga = 0.0f;
    direccionRafaga = 1.0f;
    yRafaga = 900.0f;
    tiempoEntradaDron = 0.0f;
    tiempoCorreccionLateral = 0.0f;
    tiempoInvulnerable = 0.0f;
    dronActivo = false;
    piscinaFinal.moveLeft(210.0f);
    piscinaVelocidad = 0.0f;
    piscinaAceleracion = 0.0f;
    xEntradaAgua = 0.0f;
    velocidadEntradaAgua = 0.0f;
    calidadEntrada = 0;
    nivelSuperado = false;
    nivelPerdido = false;
    motivoDerrotaActual = "";
    entradaAguaActiva = false;
    resultadoEntradaSuperado = false;
    saltoInicialPendiente = true;
    jugador->colocarEn(82.0f, 128.0f);
    jugador->setEnAire(false);
    jugador->setVX(0.0f);
    jugador->setVY(0.0f);
    dron->colocarEn(880.0f, 95.0f);
    dron->reiniciarMemoriaParcial();

    liberarEntidades();
    crearEntidades();
    crearMonedas();
}

void NivelRutaAnillos::configurarPersonaje(TipoPersonaje tipo)
{
    jugador->configurarTipo(tipo);
    radioIman = jugador->getRadioPoder();
    reiniciarNivel();
}

void NivelRutaAnillos::cambiarDificultad(TipoDificultad tipo)
{
    dificultad.configurar(tipo);

    if (tipo == FACIL) {
        tiempoTotal = 56.0f;
    }
    else if (tipo == NORMAL) {
        tiempoTotal = 46.0f;
    }
    else {
        tiempoTotal = 42.0f;
    }

    reiniciarNivel();
}

bool NivelRutaAnillos::estaSuperado() const
{
    return nivelSuperado;
}

bool NivelRutaAnillos::estaPerdido() const
{
    return nivelPerdido;
}

QString NivelRutaAnillos::motivoDerrota() const
{
    return motivoDerrotaActual.isEmpty()
               ? "No completaste la ruta experimental."
               : motivoDerrotaActual;
}

QString NivelRutaAnillos::nombreNivel() const
{
    return "Torre experimental";
}

QVector<EventoSonidoJuego> NivelRutaAnillos::consumirEventosSonido()
{
    QVector<EventoSonidoJuego> eventos = eventosSonido;
    eventosSonido.clear();
    return eventos;
}
