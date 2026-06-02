#include "NivelPiscinaEntrenamiento.h"
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
    spriteFondoCiudad.load(":/recursos/sprites/fondo_ciudad_academia.png");
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

    puntaje = 0;
    mejorPuntaje = 0;
    intentosRestantes = 0;
    monedasRecolectadas = 0;

    intentoTerminado = false;
    intentoGanado = false;
    nivelSuperado = false;
    nivelPerdido = false;
    jugadorEnZonaViento = false;

    vientoLateral = 0.0f;
    gravedad = 4.2f;
    errorEntrada = 0.0f;
    tiempoNivel = 0.0f;
    tiempoIman = 0.0f;
    cooldownIman = 0.0f;
    radioIman = 172.0f;

    dificultad.configurar(NORMAL);
    aplicarParametrosDificultad();
    plataforma->colocarEn(86.0f, 165.0f);

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

    plataforma->configurarOscilacion(
        dificultad.getAmplitudPlataforma(),
        dificultad.getVelocidadPlataforma()
        );

    jugador->setEnergiaMaxima(dificultad.getEnergiaInicial());

    if (dificultad.getTipo() == FACIL) {
        gravedad = 3.6f;
    }
    else if (dificultad.getTipo() == NORMAL) {
        gravedad = 4.2f;
    }
    else {
        gravedad = 5.0f;
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
    plataforma->actualizar(dt);
    dron->actualizar(dt, *jugador);
    tiempoNivel += dt;
    cooldownIman = std::max(0.0f, cooldownIman - dt);
    tiempoIman = std::max(0.0f, tiempoIman - dt);
    actualizarMonedas(dt);

    if (!jugador->estaEnAire()) {
        jugador->colocarEn(
            plataforma->getX() + plataforma->getAncho() / 2.0f - jugador->getAncho() / 2.0f,
            plataforma->getY() - jugador->getAlto() + 8.0f
            );
    }

    if (!intentoTerminado && !nivelSuperado && !nivelPerdido) {
        verificarZonaViento();

        jugador->aplicarGravedad(gravedad);

        if (jugadorEnZonaViento) {
            float turbulencia = std::sin(tiempoNivel * 4.0f + jugador->getY() * 0.01f) * 26.0f;
            jugador->aplicarViento(vientoLateral + turbulencia);
        }

        if (dron->rect().intersects(jugador->rect()) && jugador->estaEnAire()) {
            jugador->aplicarViento(jugador->centro().x() < dron->centro().x() ? -180.0f : 180.0f);
            jugador->setVY(jugador->getVY() + 45.0f);
        }

        jugador->actualizar(dt);
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
        moneda->actualizar(dt);
        float dx = moneda->centro().x() - objetivo.x();
        float dy = moneda->centro().y() - objetivo.y();
        float distancia = std::sqrt(dx * dx + dy * dy);

        if (tiempoIman > 0.0f && distancia <= radioIman) {
            moneda->atraerHacia(objetivo, 720.0f, radioIman, dt);
        }

        if (!moneda->estaRecolectada() && moneda->rect().intersects(jugador->rect())) {
            moneda->recolectar();
            monedasRecolectadas++;
            puntaje = std::min(100, puntaje + 3);
            eventosSonido.push_back(SONIDO_ANILLO);
        }
    }
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

    if (piscina.intersects(jugador->rect()) && jugador->getVY() > 0.0f) {
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
            intentosRestantes--;

            if (intentosRestantes <= 0) {
                nivelPerdido = true;
            }
        }

        jugador->detenerMovimiento();
        dron->aprender(errorEntrada);
        eventosSonido.push_back(SONIDO_AGUA);
    }
}

void NivelPiscinaEntrenamiento::verificarSuelo()
{
    if (intentoTerminado || nivelSuperado || nivelPerdido) {
        return;
    }

    if (suelo.intersects(jugador->rect()) && !piscina.intersects(jugador->rect())) {
        intentoTerminado = true;
        puntaje = 0;
        errorEntrada = 999.0f;
        intentosRestantes--;

        if (intentosRestantes <= 0) {
            nivelPerdido = true;
        }

        jugador->detenerMovimiento();
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

    puntaje = puntajePrecision + bonusVelocidad + bonusMonedas;

    if (puntaje > 100) {
        puntaje = 100;
    }

    if (puntaje < 0) {
        puntaje = 0;
    }
}

void NivelPiscinaEntrenamiento::dibujarEscenario(QPainter& painter)
{
    for (int y = 0; y < alturaMundo; y += 600) {
        if (!spriteFondoCiudad.isNull()) {
            painter.drawPixmap(QRect(0, y, 800, 600), spriteFondoCiudad);
            painter.fillRect(QRectF(0, y, 800, 600), QColor(7, 22, 38, 108));
        }
        else {
            QLinearGradient cielo(0, y, 0, y + 600);
            cielo.setColorAt(0.0, QColor(170, 220, 255));
            cielo.setColorAt(0.7, QColor(90, 190, 235));
            cielo.setColorAt(1.0, QColor(40, 140, 200));
            painter.fillRect(QRectF(0, y, 800, 600), cielo);
        }
    }

    if (!spriteEdificio.isNull()) {
        painter.drawPixmap(QRect(540, 110, 192, 120), spriteEdificio);
        painter.drawPixmap(QRect(520, 3600, 192, 120), spriteEdificio);
    }

    painter.setPen(QPen(QColor(230, 250, 255, 36), 2));
    for (int y = 120; y < alturaMundo - 120; y += 88) {
        painter.drawLine(90, y, 710, y + 22);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(42, 205, 238, 55));
    painter.drawRect(QRectF(62, 90, 42, alturaMundo - 180));
    painter.setBrush(QColor(255, 225, 95, 205));
    for (int y = 142; y < alturaMundo - 160; y += 210) {
        painter.drawRect(QRectF(38, y, 158, 10));
    }

    dibujarRafagasViento(painter);

    if (!spriteBanderines.isNull()) {
        painter.drawPixmap(QRect(145, 245, 420, 84), spriteBanderines);
    }

    const QRectF pisoVisual(0, 3820, 800, alturaMundo - 3820);
    QLinearGradient pisoFinal(0, pisoVisual.y(), 0, pisoVisual.y() + pisoVisual.height());
    pisoFinal.setColorAt(0.0, QColor(218, 232, 232));
    pisoFinal.setColorAt(0.36, QColor(126, 158, 142));
    pisoFinal.setColorAt(1.0, QColor(21, 112, 66));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(pisoFinal));
    painter.drawRect(pisoVisual);

    painter.setPen(QPen(QColor(255, 255, 255, 72), 2));
    for (int x = 55; x < 800; x += 92) {
        painter.drawLine(x, pisoVisual.y() + 12.0f, x - 35, pisoVisual.y() + pisoVisual.height());
    }

    painter.setPen(QPen(QColor(255, 225, 95, 190), 4));
    painter.drawLine(QPointF(0, pisoVisual.y() + 6.0f), QPointF(800, pisoVisual.y() + 6.0f));

    painter.setBrush(QColor(4, 26, 36, 110));
    painter.setPen(Qt::NoPen);
    painter.drawRect(QRectF(170, piscina.y() - 18.0f, 460, 150));

    if (!spriteAguaProfunda.isNull()) {
        painter.drawPixmap(QRect(222, static_cast<int>(piscina.y() + 18.0f), 356, 136), spriteAguaProfunda);
    }

    if (!spritePiscinaPremium.isNull()) {
        painter.drawPixmap(QRect(176, static_cast<int>(piscina.y() - 54.0f), 448, 190), spritePiscinaPremium);
    }
    else if (!spritePiscinaBase.isNull()) {
        painter.drawPixmap(QRect(198, static_cast<int>(piscina.y() - 34.0f), 402, 140), spritePiscinaBase);
    }

    for (const auto& moneda : monedas) {
        moneda->dibujar(painter);
    }

    plataforma->dibujar(painter);
    dron->dibujar(painter);

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
    painter.setPen(QPen(QColor(150, 235, 255, jugadorEnZonaViento ? 150 : 82), 3));
    for (int i = 0; i < 22; ++i) {
        float y = zonaViento.y() + 45.0f + i * 72.0f;
        float offset = std::sin(tiempoNivel * 3.0f + i) * 34.0f;
        painter.drawArc(QRectF(120 + offset, y, 560, 38), 0, 180 * 16);
        painter.drawLine(QPointF(170 + offset, y + 18), QPointF(620 + offset, y + 18));
    }
}

void NivelPiscinaEntrenamiento::dibujarIman(QPainter& painter)
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
    painter.drawText(38, 176, "Iman: " + QString(tiempoIman > 0.0f ? "activo" : cooldownIman > 0.0f ? "recargando" : "listo"));

    dibujarPanel(painter, QRectF(548, 24, 232, 74), QColor(3, 12, 25, 174));
    painter.setPen(QColor(190, 236, 248));
    painter.drawText(568, 52, jugadorEnZonaViento ? "Rafagas visibles activas" : "Aire estable");
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
            jugador->setVX(70.0f);
            jugador->setVY(72.0f);
        }
    }

    if (tecla == Qt::Key_A) {
        jugador->moverIzquierda(true);
    }

    if (tecla == Qt::Key_D) {
        jugador->moverDerecha(true);
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
    intentoTerminado = false;
    intentoGanado = false;
    jugadorEnZonaViento = false;
    tiempoIman = 0.0f;
    cooldownIman = 0.0f;
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

    intentoTerminado = false;
    intentoGanado = false;
    nivelSuperado = false;
    nivelPerdido = false;
    jugadorEnZonaViento = false;
    tiempoNivel = 0.0f;
    camaraY = 0.0f;
    tiempoIman = 0.0f;
    cooldownIman = 0.0f;
    dron->reiniciarMemoriaParcial();

    intentosRestantes = dificultad.getIntentosMaximos();

    jugador->setEnAire(false);
    jugador->setVX(0.0f);
    jugador->setVY(0.0f);
    jugador->activarImpulso(false);
    jugador->setEnergiaMaxima(dificultad.getEnergiaInicial());
    plataforma->colocarEn(86.0f, 165.0f);
    crearMonedas();

    jugador->colocarEn(
        plataforma->getX() + plataforma->getAncho() / 2.0f - jugador->getAncho() / 2.0f,
        plataforma->getY() - jugador->getAlto() + 8.0f
        );
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
