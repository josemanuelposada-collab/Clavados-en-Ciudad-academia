#include "DronVigilante.h"
#include "../fisicas/ModelosFisicos.h"
#include "../logica/JuegoException.h"
#include "../render/SpriteCache.h"
#include <algorithm>
#include <cmath>

DronVigilante::DronVigilante(float xInicial, float yInicial)
    : Entidad(xInicial, yInicial, 70.0f, 48.0f),
      xBase(xInicial),
      velocidad(95.0f),
      tiempo(0.0f),
      tiempoDecision(0.0f),
      aciertosJugador(0),
      estado(PATRULLA),
      memoriaCantidad(0),
      memoriaIndice(0),
      sumaErrores(0.0f),
      ultimoError(160.0f),
      tendenciaError(0.0f),
      objetivoSuavizado(xInicial),
      presionActual(0.0f),
      tiempoDisparo(0.0f),
      tiempoDesdeImpacto(0.0f),
      impactosJugador(0),
      evasionesJugador(0),
      bonusRecolectados(0),
      usosPoderObservados(0),
      promedioPuntaje(50.0f)
{
    memoriaErrores.fill(0.0f);
    spriteNormal = SpriteCache::obtener(":/recursos/sprites/dron_normal.png");
    spriteEscaneo = SpriteCache::obtener(":/recursos/sprites/dron_escaneo.png");
    spriteAlerta = SpriteCache::obtener(":/recursos/sprites/dron_alerta.png");

    if (spriteNormal.isNull()) {
        throw JuegoException("No se pudo cargar el sprite principal del dron vigilante.");
    }
    if (spriteEscaneo.isNull()) {
        spriteEscaneo = spriteNormal;
    }
    if (spriteAlerta.isNull()) {
        spriteAlerta = spriteNormal;
    }
}

void DronVigilante::actualizar(float dt)
{
    tiempo += dt;
    tiempoDesdeImpacto += dt;
    x = xBase + 120.0f * std::sin(tiempo * 1.4f);
}

void DronVigilante::actualizar(float dt, const Personaje& jugador)
{
    tiempo += dt;
    tiempoDecision += dt;
    tiempoDesdeImpacto += dt;

    PercepcionDron percepcion = percibir(jugador);
    if (tiempoDecision >= 0.18f) {
        estado = razonar(percepcion);
        tiempoDecision = 0.0f;
    }

    actuar(dt, percepcion);
}

void DronVigilante::dibujar(QPainter& painter)
{
    const QPixmap* sprite = &spriteNormal;
    if ((estado == ESCANEO || estado == ANTICIPA) && !spriteEscaneo.isNull()) {
        sprite = &spriteEscaneo;
    }
    else if (estado == INTERCEPTA && !spriteAlerta.isNull()) {
        sprite = &spriteAlerta;
    }

    QRect area = rect().toRect();
    if (!sprite->isNull()) {
        SpriteCache::dibujarAjustado(painter, *sprite, area, "dron");
    }
    else {
        painter.setPen(Qt::black);
        painter.setBrush(QColor(80, 80, 180));
        painter.drawRoundedRect(rect(), 6, 6);
    }

    if (estado != PATRULLA) {
        painter.setPen(QPen(QColor(255, 80, 80, 130), 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(QRectF(x - 45, y - 38, ancho + 90, alto + 76));
    }
}

PercepcionDron DronVigilante::percibir(const Personaje& jugador) const
{
    float dx = jugador.centro().x() - centro().x();
    float dy = jugador.centro().y() - centro().y();
    float distancia2 = FisicaJuego::distanciaCuadrada(dx, dy);
    float rapidez2 = FisicaJuego::rapidezCuadrada(jugador.getVX(), jugador.getVY());
    float presion = calcularPresionDificultad();
    float horizonte = 0.24f + presion * 0.42f;

    PercepcionDron percepcion;
    percepcion.distanciaCuadrada = distancia2;
    percepcion.rapidezCuadrada = rapidez2;
    percepcion.dxJugador = dx;
    percepcion.dyJugador = dy;
    percepcion.prediccionX = jugador.centro().x() + jugador.getVX() * horizonte;
    percepcion.presionAprendida = presion;
    percepcion.jugadorCerca = distancia2 < 230.0f * 230.0f;
    percepcion.jugadorRapido = rapidez2 > 520.0f * 520.0f;
    percepcion.jugadorImpulsando = jugador.estaUsandoImpulso();
    return percepcion;
}

EstadoDron DronVigilante::razonar(const PercepcionDron& percepcion)
{
    if (percepcion.jugadorCerca && (percepcion.jugadorImpulsando || percepcion.jugadorRapido || percepcion.presionAprendida > 0.62f)) {
        return INTERCEPTA;
    }

    if (percepcion.distanciaCuadrada < 330.0f * 330.0f && (percepcion.presionAprendida > 0.35f || std::abs(percepcion.dxJugador) < 190.0f)) {
        return ANTICIPA;
    }

    if (percepcion.distanciaCuadrada < 330.0f * 330.0f) {
        return ESCANEO;
    }

    return PATRULLA;
}

void DronVigilante::actuar(float dt, const PercepcionDron& percepcion)
{
    if (estado == PATRULLA) {
        float amplitud = 112.0f + presionActual * 36.0f;
        x = xBase + amplitud * std::sin(tiempo * 1.2f);
        return;
    }

    float objetivo = std::clamp(percepcion.prediccionX - ancho / 2.0f, 0.0f, 800.0f - ancho);
    objetivoSuavizado = objetivoSuavizado * 0.82f + objetivo * 0.18f;

    float multiplicador = 1.0f + percepcion.presionAprendida * 0.32f;
    if (estado == ANTICIPA) {
        multiplicador = 1.42f + percepcion.presionAprendida * 0.45f;
    }
    else if (estado == INTERCEPTA) {
        multiplicador = 2.02f + percepcion.presionAprendida * 0.85f;
    }

    float pasoMaximo = velocidad * multiplicador * dt;
    float delta = std::clamp(objetivoSuavizado - x, -pasoMaximo, pasoMaximo);
    x += delta;
}

void DronVigilante::aprender(float errorEntrada)
{
    float error = std::clamp(errorEntrada, 0.0f, 220.0f);

    if (memoriaCantidad < TAMANO_MEMORIA) {
        memoriaErrores[memoriaIndice] = error;
        sumaErrores += error;
        memoriaCantidad++;
    }
    else {
        sumaErrores -= memoriaErrores[memoriaIndice];
        memoriaErrores[memoriaIndice] = error;
        sumaErrores += error;
    }

    tendenciaError = error - ultimoError;
    ultimoError = error;
    memoriaIndice = (memoriaIndice + 1) % TAMANO_MEMORIA;
    presionActual = calcularPresionDificultad();
}

void DronVigilante::registrarAciertoJugador()
{
    aciertosJugador++;
}

void DronVigilante::registrarResultado(float errorEntrada, float puntaje, bool usoPoder)
{
    aprender(errorEntrada);
    promedioPuntaje = promedioPuntaje * 0.74f + std::clamp(puntaje, 0.0f, 100.0f) * 0.26f;
    if (usoPoder) {
        usosPoderObservados++;
    }
    presionActual = calcularPresionDificultad();
}

void DronVigilante::registrarImpactoJugador()
{
    impactosJugador++;
    evasionesJugador = std::max(0, evasionesJugador - 1);
    tiempoDesdeImpacto = 0.0f;
    presionActual = calcularPresionDificultad();
}

void DronVigilante::registrarEvasionJugador()
{
    evasionesJugador++;
    presionActual = calcularPresionDificultad();
}

void DronVigilante::registrarBonusRecolectado()
{
    bonusRecolectados++;
    presionActual = calcularPresionDificultad();
}

void DronVigilante::reiniciarMemoriaParcial()
{
    estado = PATRULLA;
    tiempoDecision = 0.0f;
    objetivoSuavizado = x;
    tiempoDisparo = 0.0f;
}

void DronVigilante::colocarEn(float nuevoX, float nuevoY)
{
    xBase = nuevoX;
    x = nuevoX;
    y = nuevoY;
    if (std::abs(objetivoSuavizado - nuevoX) > 260.0f) {
        objetivoSuavizado = nuevoX;
    }
}

void DronVigilante::colocarY(float nuevoY)
{
    y = nuevoY;
}

bool DronVigilante::solicitarDisparo(float dt, const Personaje& jugador)
{
    tiempoDisparo += dt;

    if (!jugador.estaEnAire() || estado == PATRULLA) {
        return false;
    }

    PercepcionDron percepcion = percibir(jugador);
    if (percepcion.distanciaCuadrada > 520.0f * 520.0f) {
        return false;
    }

    float espera = std::clamp(2.20f - percepcion.presionAprendida * 0.82f, 1.15f, 2.20f);
    if (estado == INTERCEPTA) {
        espera *= 0.82f;
    }

    if (tiempoDisparo >= espera) {
        tiempoDisparo = 0.0f;
        return true;
    }

    return false;
}

QPointF DronVigilante::calcularVectorDisparo(const Personaje& jugador, float rapidez) const
{
    float presion = calcularPresionDificultad();
    QPointF origen = centro();
    float dxActual = jugador.centro().x() - origen.x();
    float dyActual = jugador.centro().y() - origen.y();
    float distanciaActual = std::sqrt(FisicaJuego::distanciaCuadrada(dxActual, dyActual));
    float rapidezJugador = std::sqrt(FisicaJuego::rapidezCuadrada(jugador.getVX(), jugador.getVY()));
    float rapidezRelativa = std::max(90.0f, rapidez * 0.88f + rapidezJugador * 0.58f);
    float horizonte = std::clamp(distanciaActual / rapidezRelativa + presion * 0.16f, 0.18f, 1.45f);

    QPointF objetivo(jugador.centro().x() + jugador.getVX() * horizonte,
                     jugador.centro().y() + jugador.getVY() * horizonte + 0.5f * 30.0f * horizonte * horizonte);
    float dx = objetivo.x() - origen.x();
    float dy = objetivo.y() - origen.y();
    float distancia = std::sqrt(FisicaJuego::distanciaCuadrada(dx, dy));

    if (distancia < 1.0f) {
        return QPointF(0.0f, rapidez);
    }

    return QPointF(dx / distancia * rapidez, dy / distancia * rapidez);
}

float DronVigilante::calcularPresionDificultad() const
{
    if (memoriaCantidad == 0) {
        float presionInicial = aciertosJugador * 0.12f + impactosJugador * 0.08f +
                                bonusRecolectados * 0.018f + usosPoderObservados * 0.025f -
                                evasionesJugador * 0.035f;
        return std::clamp(presionInicial, 0.0f, 0.45f);
    }

    float promedio = sumaErrores / memoriaCantidad;
    float precisionJugador = 1.0f - std::min(promedio / 160.0f, 1.0f);
    float racha = std::min(aciertosJugador * 0.12f, 0.36f);
    float presionImpactos = std::min(impactosJugador * 0.08f, 0.32f);
    float alivioEvasiones = std::min(evasionesJugador * 0.035f, 0.22f);
    float presionBonus = std::min(bonusRecolectados * 0.018f, 0.16f);
    float presionPoder = std::min(usosPoderObservados * 0.025f, 0.18f);
    float ajustePuntaje = promedioPuntaje >= 78.0f ? 0.10f :
                          promedioPuntaje <= 42.0f ? -0.14f :
                                                       0.0f;
    float descanso = tiempoDesdeImpacto > 6.0f ? -0.08f : 0.0f;
    float tendencia = tendenciaError < -4.0f ? 0.10f : tendenciaError > 8.0f ? -0.06f : 0.0f;
    return std::clamp(precisionJugador + racha + presionImpactos + presionBonus + presionPoder +
                          ajustePuntaje + tendencia + descanso - alivioEvasiones,
                      0.0f, 1.0f);
}

float DronVigilante::obtenerPresionAdaptativa() const
{
    return calcularPresionDificultad();
}

EstadoDron DronVigilante::getEstado() const
{
    return estado;
}
