#include "NivelRutaAnillos.h"
#include "JuegoException.h"
#include <QKeyEvent>
#include <QLinearGradient>
#include <algorithm>
#include <cmath>

namespace {
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
}

NivelRutaAnillos::NivelRutaAnillos()
    : jugador(std::make_unique<Personaje>()),
      dron(std::make_unique<DronVigilante>(385.0f, 160.0f)),
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
      calidadEntrada(0),
      nivelSuperado(false),
      nivelPerdido(false),
      entradaAguaActiva(false),
      resultadoEntradaSuperado(false)
{
    dificultad.configurar(NORMAL);
    spriteAlarma.load(":/recursos/sprites/alarma.png");
    spriteViento.load(":/recursos/sprites/viento_turbulencia.png");
    spritePiscinaFinal.load(":/recursos/sprites/piscina_ciudad_academia.png");
    spriteColumnaTorre.load(":/recursos/sprites/torre_columna_luz.png");
    spriteFondo.load(":/recursos/sprites/fondo_ciudad_academia.png");
    spriteBanderines.load(":/recursos/sprites/banderines_academia.png");
    spriteEdificio.load(":/recursos/sprites/edificio_cristal_academia.png");
    spriteTorreEpica.load(":/recursos/sprites/torre_salto_epica.png");
    spritePiscinaPremium.load(":/recursos/sprites/piscina_final_premium.png");
    spriteAguaProfunda.load(":/recursos/sprites/agua_profunda.png");
    spriteChapuzonLimpio.load(":/recursos/sprites/chapuzon_limpio.png");
    spriteChapuzonMedio.load(":/recursos/sprites/chapuzon_medio.png");
    spriteChapuzonFuerte.load(":/recursos/sprites/chapuzon_fuerte.png");
    spriteBurbujas.load(":/recursos/sprites/burbujas_entrada.png");
    jugador->colocarEn(382.0f, 92.0f);
    jugador->setEnAire(true);
    jugador->setVY(120.0f);
    crearEntidades();
    crearMonedas();
}

NivelRutaAnillos::~NivelRutaAnillos() = default;

void NivelRutaAnillos::crearEntidades()
{
    anillos.reserve(12);
    anillos.emplace_back(std::make_unique<Anillo>(386.0f, 360.0f));
    anillos.emplace_back(std::make_unique<Anillo>(190.0f, 740.0f));
    anillos.emplace_back(std::make_unique<Anillo>(570.0f, 1120.0f));
    anillos.emplace_back(std::make_unique<Anillo>(320.0f, 1510.0f));
    anillos.emplace_back(std::make_unique<Anillo>(520.0f, 1930.0f));
    anillos.emplace_back(std::make_unique<Anillo>(240.0f, 2360.0f));
    anillos.emplace_back(std::make_unique<Anillo>(602.0f, 2820.0f));
    anillos.emplace_back(std::make_unique<Anillo>(350.0f, 3310.0f));
    anillos.emplace_back(std::make_unique<Anillo>(170.0f, 3820.0f));
    anillos.emplace_back(std::make_unique<Anillo>(560.0f, 4330.0f));
    anillos.emplace_back(std::make_unique<Anillo>(300.0f, 4920.0f));
    anillos.emplace_back(std::make_unique<Anillo>(420.0f, 5430.0f));

    float factor = dificultad.getVelocidadPlataforma();
    obstaculos.reserve(8);
    obstaculos.emplace_back(std::make_unique<Obstaculo>(130.0f, 720.0f, 54.0f, 54.0f, 52.0f * factor, 0.0f, BOYA));
    obstaculos.emplace_back(std::make_unique<Obstaculo>(575.0f, 1380.0f, 58.0f, 58.0f, -64.0f * factor, 0.0f, BARRIL));
    obstaculos.emplace_back(std::make_unique<Obstaculo>(210.0f, 2050.0f, 54.0f, 54.0f, 72.0f * factor, 0.0f, MINA));
    obstaculos.emplace_back(std::make_unique<Obstaculo>(560.0f, 2700.0f, 56.0f, 56.0f, -78.0f * factor, 0.0f, BLOQUEO));
    obstaculos.emplace_back(std::make_unique<Obstaculo>(120.0f, 3460.0f, 54.0f, 54.0f, 70.0f * factor, 0.0f, BOYA));
    obstaculos.emplace_back(std::make_unique<Obstaculo>(610.0f, 4120.0f, 58.0f, 58.0f, -74.0f * factor, 0.0f, BARRIL));
    obstaculos.emplace_back(std::make_unique<Obstaculo>(230.0f, 4860.0f, 54.0f, 54.0f, 82.0f * factor, 0.0f, MINA));
    obstaculos.emplace_back(std::make_unique<Obstaculo>(560.0f, 5360.0f, 56.0f, 56.0f, -86.0f * factor, 0.0f, BLOQUEO));

    if (anillos.empty() || obstaculos.empty()) {
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

    if (entradaAguaActiva) {
        tiempoEntradaAgua += dt;
        if (tiempoEntradaAgua >= 2.45f) {
            nivelSuperado = resultadoEntradaSuperado;
            nivelPerdido = !resultadoEntradaSuperado;
        }
        return;
    }

    cooldownIman = std::max(0.0f, cooldownIman - dt);
    tiempoIman = std::max(0.0f, tiempoIman - dt);
    tiempoRestante -= dt;

    aplicarMovimientoJugador(dt);
    actualizarMonedas(dt);

    for (const auto& anillo : anillos) {
        anillo->actualizar(dt);
    }

    for (const auto& obstaculo : obstaculos) {
        obstaculo->actualizar(dt);
        obstaculo->rebotarHorizontal(76.0f, 724.0f);
    }

    float yDron = std::clamp(jugador->getY() - 145.0f, 95.0f, alturaMundo - 250.0f);
    dron->colocarEn(dron->getX(), yDron);
    dron->actualizar(dt, *jugador);

    verificarInteracciones();
    calcularPuntaje();
    actualizarCamara();

    if (!entradaAguaActiva && (tiempoRestante <= 0.0f || golpes >= 4)) {
        nivelPerdido = true;
    }
}

void NivelRutaAnillos::actualizarMonedas(float dt)
{
    QPointF objetivo = jugador->centro();
    for (const auto& moneda : monedas) {
        moneda->actualizar(dt);
        float dx = moneda->centro().x() - objetivo.x();
        float dy = moneda->centro().y() - objetivo.y();
        float distancia = std::sqrt(dx * dx + dy * dy);

        if (tiempoIman > 0.0f && distancia <= radioIman) {
            moneda->atraerHacia(objetivo, 740.0f, radioIman, dt);
        }

        if (!moneda->estaRecolectada() && moneda->rect().intersects(jugador->rect())) {
            moneda->recolectar();
            monedasRecolectadas++;
            eventosSonido.push_back(SONIDO_ANILLO);
        }
    }
}

void NivelRutaAnillos::aplicarMovimientoJugador(float dt)
{
    float gravedad = 24.0f + dificultad.getIntensidadViento() * 0.08f;
    float controlHorizontal = 0.0f;

    if (teclas.contains(Qt::Key_A) || teclas.contains(Qt::Key_Left)) {
        controlHorizontal -= 420.0f;
    }
    if (teclas.contains(Qt::Key_D) || teclas.contains(Qt::Key_Right)) {
        controlHorizontal += 420.0f;
    }
    if (teclas.contains(Qt::Key_E) || teclas.contains(Qt::Key_Up) || teclas.contains(Qt::Key_W)) {
        velocidadVertical -= 150.0f * dt;
    }
    if (teclas.contains(Qt::Key_S) || teclas.contains(Qt::Key_Down)) {
        velocidadVertical += 145.0f * dt;
    }

    velocidadHorizontal += controlHorizontal * dt;
    velocidadHorizontal *= 0.955f;
    velocidadVertical += gravedad * dt;

    if (zonaViento.intersects(jugador->rect())) {
        float turbulencia = dificultad.getIntensidadViento() * (0.9f + 0.35f * std::sin(tiempoNivel * 3.4f));
        velocidadHorizontal += turbulencia * dt;
    }

    if (zonaVelocidad.intersects(jugador->rect())) {
        velocidadVertical *= 0.992f;
    }

    velocidadVertical = std::clamp(velocidadVertical, 96.0f, 215.0f);
    velocidadHorizontal = std::clamp(velocidadHorizontal, -245.0f, 245.0f);

    float nuevoX = std::clamp(jugador->getX() + velocidadHorizontal * dt, 40.0f, 725.0f);
    float nuevoY = std::clamp(jugador->getY() + velocidadVertical * dt, 82.0f, alturaMundo - 62.0f);
    jugador->colocarEn(nuevoX, nuevoY);
    jugador->setVX(velocidadHorizontal);
    jugador->setVY(velocidadVertical);
}

void NivelRutaAnillos::verificarInteracciones()
{
    for (const auto& anillo : anillos) {
        if (!anillo->estaRecolectado() && anillo->rect().intersects(jugador->rect())) {
            anillo->recolectar();
            anillosRecolectados++;
            dron->aprender(35.0f);
            eventosSonido.push_back(SONIDO_ANILLO);
        }
    }

    for (const auto& obstaculo : obstaculos) {
        if (obstaculo->rect().intersects(jugador->rect())) {
            golpes++;
            tiempoRestante = std::max(0.0f, tiempoRestante - 2.5f);
            velocidadHorizontal *= -0.45f;
            velocidadVertical = std::max(120.0f, velocidadVertical * 0.72f);
            dron->aprender(165.0f);
            jugador->colocarEn(std::clamp(jugador->getX() + 55.0f, 45.0f, 720.0f), jugador->getY() - 35.0f);
            eventosSonido.push_back(SONIDO_COLISION);
            return;
        }
    }

    if (dron->rect().intersects(jugador->rect())) {
        golpes++;
        tiempoRestante = std::max(0.0f, tiempoRestante - 3.0f);
        velocidadHorizontal += jugador->centro().x() < dron->centro().x() ? -160.0f : 160.0f;
        eventosSonido.push_back(SONIDO_COLISION);
    }

    if (piscinaFinal.intersects(jugador->rect())) {
        registrarEntradaAgua();
    }
}

void NivelRutaAnillos::registrarEntradaAgua()
{
    calcularPuntaje();
    entradaAguaActiva = true;
    resultadoEntradaSuperado = puntaje >= 55;
    tiempoEntradaAgua = 0.0f;
    xEntradaAgua = jugador->centro().x();
    velocidadEntradaAgua = std::abs(jugador->getVY());

    if (puntaje >= 82) {
        calidadEntrada = 2;
    }
    else if (puntaje >= 55) {
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
        eventosSonido.push_back(SONIDO_NIVEL);
    }
}

void NivelRutaAnillos::calcularPuntaje()
{
    float centroPiscina = piscinaFinal.x() + piscinaFinal.width() / 2.0f;
    float errorEntrada = std::abs(jugador->centro().x() - centroPiscina);
    int precision = static_cast<int>(std::clamp(38.0f - errorEntrada * 0.22f, 0.0f, 38.0f));
    int ruta = anillosRecolectados * 5;
    int bonusMonedas = std::min(18, monedasRecolectadas * 2);
    int bonusRitmo = static_cast<int>(std::max(0.0f, tiempoRestante) * 0.28f);
    int bonusVelocidad = velocidadVertical >= 120.0f && velocidadVertical <= 195.0f ? 12 : 2;
    int penalizacion = golpes * 10;
    puntaje = std::clamp(ruta + bonusMonedas + precision + bonusRitmo + bonusVelocidad - penalizacion, 0, 100);
}

void NivelRutaAnillos::actualizarCamara()
{
    camaraY = std::clamp(jugador->getY() - 255.0f, 0.0f, alturaMundo - 600.0f);
}

void NivelRutaAnillos::dibujarPiscinaFinal(QPainter& painter)
{
    if (!spriteAguaProfunda.isNull()) {
        painter.drawPixmap(QRect(196, static_cast<int>(piscinaFinal.y() + 12.0f), 408, 185), spriteAguaProfunda);
    }

    if (!spritePiscinaPremium.isNull()) {
        painter.drawPixmap(QRect(150, static_cast<int>(piscinaFinal.y() - 58.0f), 500, 212), spritePiscinaPremium);
    }
    else if (!spritePiscinaFinal.isNull()) {
        painter.drawPixmap(QRect(170, static_cast<int>(piscinaFinal.y() - 45.0f), 460, 158), spritePiscinaFinal);
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
    painter.setPen(QPen(QColor(255, 225, 95, 160), 3));
    painter.setBrush(QColor(255, 225, 95, 24));
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

void NivelRutaAnillos::dibujarEscenario(QPainter& painter)
{
    if (!spriteFondo.isNull()) {
        painter.drawPixmap(QRect(0, static_cast<int>(camaraY), 800, 600), spriteFondo);
        painter.fillRect(QRectF(0, camaraY, 800, 600), QColor(6, 20, 36, 112));
    }
    else {
        QLinearGradient fondo(0, camaraY, 0, camaraY + 600);
        fondo.setColorAt(0.0, QColor(18, 40, 74));
        fondo.setColorAt(0.55, QColor(28, 125, 166));
        fondo.setColorAt(1.0, QColor(12, 35, 64));
        painter.fillRect(QRectF(0, camaraY, 800, 600), fondo);
    }

    if (!spriteEdificio.isNull()) {
        painter.drawPixmap(QRect(516, static_cast<int>(camaraY + 36.0f), 240, 150), spriteEdificio);
    }

    if (!spriteBanderines.isNull()) {
        painter.drawPixmap(QRect(172, static_cast<int>(camaraY + 22.0f), 420, 84), spriteBanderines);
    }

    if (!spriteTorreEpica.isNull()) {
        painter.drawPixmap(QRect(28, 52, 184, static_cast<int>(alturaMundo - 145.0f)), spriteTorreEpica);
        painter.save();
        painter.setOpacity(0.34);
        painter.drawPixmap(QRect(646, 120, 116, static_cast<int>(alturaMundo - 230.0f)), spriteTorreEpica);
        painter.restore();
    }
    else if (!spriteColumnaTorre.isNull()) {
        for (int y = 72; y < alturaMundo - 120; y += 560) {
            painter.drawPixmap(QRect(36, y, 128, 720), spriteColumnaTorre);
            painter.drawPixmap(QRect(636, y, 128, 720), spriteColumnaTorre);
        }
    }
    else {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(230, 246, 255, 38));
        painter.drawRoundedRect(QRectF(54, 70, 92, alturaMundo - 130), 8, 8);
        painter.drawRoundedRect(QRectF(654, 70, 92, alturaMundo - 130), 8, 8);
    }

    painter.setPen(QPen(QColor(230, 250, 255, 42), 2));
    for (int y = 120; y < alturaMundo - 80; y += 86) {
        painter.drawLine(86, y, 714, y + 22);
    }

    painter.setPen(QPen(QColor(130, 230, 255, 145), 2, Qt::DashLine));
    painter.setBrush(QColor(80, 205, 255, 38));
    painter.drawRoundedRect(zonaViento, 12, 12);
    if (!spriteViento.isNull()) {
        painter.drawPixmap(QRect(298, static_cast<int>(zonaViento.y() + 76), 190, 74), spriteViento);
    }

    painter.setPen(QPen(QColor(255, 230, 95, 150), 2, Qt::DashLine));
    painter.setBrush(QColor(255, 230, 95, 36));
    painter.drawRoundedRect(zonaVelocidad, 12, 12);

    dibujarPiscinaFinal(painter);

    for (const auto& anillo : anillos) {
        anillo->dibujar(painter);
    }

    for (const auto& moneda : monedas) {
        moneda->dibujar(painter);
    }

    for (const auto& obstaculo : obstaculos) {
        obstaculo->dibujar(painter);
    }

    dron->dibujar(painter);
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
    panel(painter, QRectF(18, 18, 334, 162), QColor(3, 12, 25, 204));

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
    painter.drawText(38, 70, "A/D corregir | E/W frenar | S acelerar");

    painter.setPen(QColor(255, 225, 95));
    painter.drawText(38, 100, "Tiempo");
    barra(painter, QRectF(112, 90, 190, 10), tiempoRestante / tiempoTotal, QColor(255, 225, 95));
    painter.drawText(310, 101, QString::number(tiempoRestante, 'f', 1));

    painter.setPen(QColor(120, 235, 255));
    painter.drawText(38, 126, "Descenso");
    barra(painter, QRectF(112, 116, 190, 10), jugador->getY() / (alturaMundo - 80.0f), QColor(120, 235, 255));

    painter.setPen(QColor(230, 245, 255));
    painter.drawText(38, 151, "Anillos: " + QString::number(anillosRecolectados) + "/" + QString::number(anillos.size()) +
                               "   Monedas: " + QString::number(monedasRecolectadas) +
                               "   Golpes: " + QString::number(golpes) + "/4   Pts: " + QString::number(puntaje));

    panel(painter, QRectF(548, 24, 232, 74), QColor(3, 12, 25, 174));
    painter.setPen(QColor(190, 236, 248));
    painter.drawText(568, 52, entradaAguaActiva ? "Analizando entrada" :
                                 zonaViento.intersects(jugador->rect()) ? "Viento lateral activo" : "Aire estable");
    painter.drawText(568, 78, tiempoIman > 0.0f ? "Campo iman activo" :
                                cooldownIman > 0.0f ? "Iman recargando" :
                                zonaVelocidad.intersects(jugador->rect()) ? "Zona de velocidad variable" : "Click: iman metalico");

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
        painter.drawText(260, 314, "Anillos recolectados: " + QString::number(anillosRecolectados));
        painter.drawText(260, 342, "Golpes recibidos: " + QString::number(golpes));
        painter.drawText(260, 370, "R reinicia el nivel");
    }
}

void NivelRutaAnillos::teclaPresionada(int tecla)
{
    teclas.insert(tecla);

    if (tecla == Qt::Key_R) {
        reiniciarNivel();
    }
    else if (tecla == Qt::Key_1) {
        cambiarDificultad(FACIL);
    }
    else if (tecla == Qt::Key_2) {
        cambiarDificultad(NORMAL);
    }
    else if (tecla == Qt::Key_3) {
        cambiarDificultad(DIFICIL);
    }
}

void NivelRutaAnillos::teclaLiberada(int tecla)
{
    teclas.remove(tecla);
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
        eventosSonido.push_back(SONIDO_ANILLO);
    }
}

void NivelRutaAnillos::reiniciarNivel()
{
    teclas.clear();
    anillosRecolectados = 0;
    monedasRecolectadas = 0;
    golpes = 0;
    puntaje = 0;
    tiempoRestante = tiempoTotal;
    tiempoNivel = 0.0f;
    camaraY = 0.0f;
    velocidadHorizontal = 0.0f;
    velocidadVertical = 120.0f;
    tiempoEntradaAgua = 0.0f;
    tiempoIman = 0.0f;
    cooldownIman = 0.0f;
    xEntradaAgua = 0.0f;
    velocidadEntradaAgua = 0.0f;
    calidadEntrada = 0;
    nivelSuperado = false;
    nivelPerdido = false;
    entradaAguaActiva = false;
    resultadoEntradaSuperado = false;
    jugador->colocarEn(382.0f, 92.0f);
    jugador->setEnAire(true);
    jugador->setVX(0.0f);
    jugador->setVY(velocidadVertical);
    dron->colocarEn(385.0f, 160.0f);
    dron->reiniciarMemoriaParcial();

    liberarEntidades();
    crearEntidades();
    crearMonedas();
}

void NivelRutaAnillos::cambiarDificultad(TipoDificultad tipo)
{
    dificultad.configurar(tipo);

    if (tipo == FACIL) {
        tiempoTotal = 56.0f;
    }
    else if (tipo == NORMAL) {
        tiempoTotal = 48.0f;
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
