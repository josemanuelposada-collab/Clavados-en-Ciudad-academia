#include "NivelPiscinaEntrenamiento.h"
#include <algorithm>
#include <cmath>
#include <QLinearGradient>

namespace {
void dibujarPanel(QPainter& painter, const QRectF& rect, const QColor& color)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(rect, 8, 8);
}

void dibujarBarra(QPainter& painter, const QRectF& rect, float porcentaje, const QColor& color)
{
    porcentaje = std::max(0.0f, std::min(1.0f, porcentaje));
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 70));
    painter.drawRoundedRect(rect, 5, 5);
    painter.setBrush(color);
    painter.drawRoundedRect(QRectF(rect.x(), rect.y(), rect.width() * porcentaje, rect.height()), 5, 5);
}
}

NivelPiscinaEntrenamiento::NivelPiscinaEntrenamiento()
{
    jugador = new Personaje();
    plataforma = new Plataforma();
    dron = new DronVigilante(560.0f, 145.0f);

    piscina = QRectF(260, 480, 280, 75);
    zonaMeta = QRectF(350, 480, 100, 75);
    zonaViento = QRectF(170, 270, 420, 140);
    suelo = QRectF(0, 550, 800, 50);

    spritePiscina.load(":/recursos/sprites/salpicadura_grande.png");
    spritePiscinaBase.load(":/recursos/sprites/piscina_ciudad_academia.png");
    spriteViento.load(":/recursos/sprites/viento_lateral.png");
    spriteAdvertencia.load(":/recursos/sprites/advertencia.png");
    spriteTemporizador.load(":/recursos/sprites/temporizador.png");

    puntaje = 0;
    mejorPuntaje = 0;
    intentosRestantes = 0;

    intentoTerminado = false;
    intentoGanado = false;
    nivelSuperado = false;
    nivelPerdido = false;
    jugadorEnZonaViento = false;

    vientoLateral = 0.0f;
    gravedad = 820.0f;
    errorEntrada = 0.0f;
    tiempoNivel = 0.0f;

    dificultad.configurar(NORMAL);
    aplicarParametrosDificultad();

    intentosRestantes = dificultad.getIntentosMaximos();

    jugador->colocarEn(
        plataforma->getX() + plataforma->getAncho() / 2.0f - jugador->getAncho() / 2.0f,
        plataforma->getY() - jugador->getAlto() + 8.0f
        );
}

NivelPiscinaEntrenamiento::~NivelPiscinaEntrenamiento()
{
    delete jugador;
    delete plataforma;
    delete dron;
}

void NivelPiscinaEntrenamiento::aplicarParametrosDificultad()
{
    vientoLateral = dificultad.getIntensidadViento();

    plataforma->configurarOscilacion(
        dificultad.getAmplitudPlataforma(),
        dificultad.getVelocidadPlataforma()
        );

    jugador->setEnergiaMaxima(dificultad.getEnergiaInicial());
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
            float turbulencia = std::sin(tiempoNivel * 4.0f + jugador->getY() * 0.01f) * 12.0f;
            jugador->aplicarViento(vientoLateral + turbulencia);
        }

        if (dron->rect().intersects(jugador->rect()) && jugador->estaEnAire()) {
            jugador->aplicarViento(jugador->centro().x() < dron->centro().x() ? -180.0f : 180.0f);
            jugador->setVY(jugador->getVY() + 45.0f);
        }

        jugador->actualizar(dt);
        verificarColisiones();
    }
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

    puntaje = puntajePrecision + bonusVelocidad;

    if (puntaje > 100) {
        puntaje = 100;
    }

    if (puntaje < 0) {
        puntaje = 0;
    }
}

void NivelPiscinaEntrenamiento::dibujar(QPainter& painter)
{
    QLinearGradient cielo(0, 0, 0, 600);
    cielo.setColorAt(0.0, QColor(170, 220, 255));
    cielo.setColorAt(0.7, QColor(90, 190, 235));
    cielo.setColorAt(1.0, QColor(40, 140, 200));

    painter.fillRect(QRectF(0, 0, 800, 600), cielo);

    QLinearGradient aguaFondo(0, 410, 0, 550);
    aguaFondo.setColorAt(0.0, QColor(90, 210, 255));
    aguaFondo.setColorAt(1.0, QColor(20, 130, 210));

    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(aguaFondo));
    painter.drawRect(QRectF(0, 410, 800, 140));

    QLinearGradient sueloGradiente(suelo.topLeft(), suelo.bottomLeft());
    sueloGradiente.setColorAt(0.0, QColor(40, 190, 90));
    sueloGradiente.setColorAt(1.0, QColor(20, 120, 50));

    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(sueloGradiente));
    painter.drawRect(suelo);

    painter.setPen(QPen(QColor(255, 255, 255, 45), 2));
    for (int i = 0; i < 8; ++i) {
        int x = 70 + i * 95;
        painter.drawLine(x, 430, x + 45, 550);
    }

    painter.setPen(QPen(QColor(150, 230, 255, 135), 2, Qt::DashLine));
    painter.setBrush(QColor(170, 230, 255, jugadorEnZonaViento ? 78 : 34));
    painter.drawRoundedRect(zonaViento, 10, 10);

    if (!spriteViento.isNull()) {
        painter.drawPixmap(QRect(300, 295, 160, 65), spriteViento);
    }

    float porcentajeEnergia = 0.0f;

    if (jugador->getEnergiaMaxima() > 0.0f) {
        porcentajeEnergia = jugador->getEnergia() / jugador->getEnergiaMaxima();
    }

    dibujarPanel(painter, QRectF(18, 18, 286, 180), QColor(7, 18, 31, 185));

    QFont tituloHud = painter.font();
    tituloHud.setPointSize(12);
    tituloHud.setBold(true);
    painter.setFont(tituloHud);
    painter.setPen(QColor(238, 252, 255));
    painter.drawText(34, 46, "Piscina de entrenamiento");

    QFont textoHud = painter.font();
    textoHud.setPointSize(8);
    textoHud.setBold(false);
    painter.setFont(textoHud);
    painter.setPen(QColor(180, 226, 242));
    painter.drawText(34, 70, "Espacio salto  |  A/D correccion  |  E impulso");

    painter.setPen(QColor(255, 225, 95));
    painter.drawText(34, 98, "Puntaje");
    dibujarBarra(painter, QRectF(108, 88, 160, 12), puntaje / 100.0f, QColor(255, 225, 95));
    painter.drawText(274, 99, QString::number(puntaje));

    painter.setPen(QColor(120, 235, 255));
    painter.drawText(34, 124, "Energia");
    dibujarBarra(painter, QRectF(108, 114, 160, 12), porcentajeEnergia, QColor(120, 235, 255));

    painter.setPen(QColor(222, 240, 250));
    painter.drawText(34, 151, "Intentos: " + QString::number(intentosRestantes) +
                               "   Minimo: " + QString::number(dificultad.getPuntajeMinimo()));
    painter.drawText(34, 174, "Viento: " + QString::number(vientoLateral, 'f', 0) +
                               "   Error: " + QString::number(errorEntrada, 'f', 1));

    dibujarPanel(painter, QRectF(325, 18, 198, 60), QColor(255, 255, 255, 185));
    painter.setPen(QColor(20, 38, 52));
    painter.drawText(343, 42, "Dron: " + QString(dron->getEstado() == PATRULLA ? "patrulla" :
                                                 dron->getEstado() == ESCANEO ? "escaneo" : "intercepta"));
    painter.drawText(343, 64, jugadorEnZonaViento ? "Zona de viento activa" : "Trayectoria estable");

    painter.setPen(QColor(255, 255, 0));
    painter.setBrush(QColor(255, 255, 0, 45));
    painter.drawRect(zonaMeta);

    if (!spritePiscinaBase.isNull()) {
        painter.drawPixmap(QRect(224, 440, 352, 122), spritePiscinaBase);
    }
    else {
        QLinearGradient aguaGradiente(piscina.topLeft(), piscina.bottomLeft());
        aguaGradiente.setColorAt(0.0, QColor(60, 190, 255));
        aguaGradiente.setColorAt(1.0, QColor(0, 80, 210));

        painter.setPen(QPen(QColor(0, 60, 150), 2));
        painter.setBrush(QBrush(aguaGradiente));
        painter.drawRoundedRect(piscina, 8, 8);
    }

    if (intentoTerminado && !spritePiscina.isNull()) {
        QRect splashRect(
            static_cast<int>(piscina.x() + piscina.width() / 2.0f - 70),
            static_cast<int>(piscina.y() - 45),
            140,
            90
            );

        painter.drawPixmap(
            splashRect,
            spritePiscina.scaled(140, 90, Qt::KeepAspectRatio, Qt::SmoothTransformation)
            );
    }

    painter.setPen(Qt::white);
    painter.drawText(piscina.x() + 95, piscina.y() + 43, "Piscina");

    painter.setPen(Qt::black);

    plataforma->dibujar(painter);
    dron->dibujar(painter);
    jugador->dibujar(painter);

    if (intentoTerminado || nivelSuperado || nivelPerdido) {
        dibujarPanel(painter, QRectF(196, 195, 430, 205), QColor(248, 252, 255, 240));

        QFont resultado = painter.font();
        resultado.setPointSize(14);
        resultado.setBold(true);
        painter.setFont(resultado);
        painter.setPen(QColor(18, 38, 54));

        if (nivelSuperado) {
            painter.drawText(QRectF(220, 225, 382, 30), Qt::AlignCenter, "NIVEL SUPERADO");
        }
        else if (nivelPerdido) {
            painter.drawText(QRectF(220, 225, 382, 30), Qt::AlignCenter, "NIVEL FALLIDO");
        }
        else {
            painter.drawText(QRectF(220, 225, 382, 30), Qt::AlignCenter, "Intento terminado");
        }

        resultado.setPointSize(9);
        resultado.setBold(false);
        painter.setFont(resultado);
        painter.drawText(255, 275, "Puntaje obtenido: " + QString::number(puntaje));
        painter.drawText(255, 302, "Puntaje minimo: " + QString::number(dificultad.getPuntajeMinimo()));
        painter.drawText(255, 329, "Error de entrada: " + QString::number(errorEntrada, 'f', 1));
        painter.drawText(255, 356, "Intentos restantes: " + QString::number(intentosRestantes));
        painter.drawText(255, 383, "R reinicia  |  Enter avanza si superaste el nivel");
    }
}

void NivelPiscinaEntrenamiento::teclaPresionada(int tecla)
{
    if (tecla == Qt::Key_Space) {
        if (!jugador->estaEnAire()) {
            eventosSonido.push_back(SONIDO_SALTO);
        }
        jugador->saltar();
    }

    if (tecla == Qt::Key_A) {
        jugador->moverIzquierda(true);
    }

    if (tecla == Qt::Key_D) {
        jugador->moverDerecha(true);
    }

    if (tecla == Qt::Key_E) {
        jugador->activarImpulso(true);
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

    if (tecla == Qt::Key_E) {
        jugador->activarImpulso(false);
    }
}

void NivelPiscinaEntrenamiento::reiniciarIntento()
{
    if (nivelSuperado || nivelPerdido) {
        return;
    }

    puntaje = 0;
    errorEntrada = 0.0f;
    intentoTerminado = false;
    intentoGanado = false;
    jugadorEnZonaViento = false;
    dron->reiniciarMemoriaParcial();

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

    intentoTerminado = false;
    intentoGanado = false;
    nivelSuperado = false;
    nivelPerdido = false;
    jugadorEnZonaViento = false;
    tiempoNivel = 0.0f;
    dron->reiniciarMemoriaParcial();

    intentosRestantes = dificultad.getIntentosMaximos();

    jugador->setEnAire(false);
    jugador->setVX(0.0f);
    jugador->setVY(0.0f);
    jugador->activarImpulso(false);
    jugador->setEnergiaMaxima(dificultad.getEnergiaInicial());

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
