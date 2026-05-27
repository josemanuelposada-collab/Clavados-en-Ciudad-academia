#include "NivelPiscinaEntrenamiento.h"
#include <cmath>
#include <QLinearGradient>

NivelPiscinaEntrenamiento::NivelPiscinaEntrenamiento()
{
    jugador = new Personaje();
    plataforma = new Plataforma();

    piscina = QRectF(260, 480, 280, 75);
    zonaMeta = QRectF(350, 480, 100, 75);
    zonaViento = QRectF(170, 270, 420, 140);
    suelo = QRectF(0, 550, 800, 50);

    spritePiscina.load(":/recursos/sprites/salpicadura_grande.png");
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
            jugador->aplicarViento(vientoLateral);
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
        }
        else {
            intentoGanado = false;
            intentosRestantes--;

            if (intentosRestantes <= 0) {
                nivelPerdido = true;
            }
        }

        jugador->detenerMovimiento();
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

    painter.setPen(QColor(70, 160, 255));
    painter.setBrush(QColor(170, 230, 255, 80));
    painter.drawRect(zonaViento);

    if (!spriteViento.isNull()) {
        painter.drawPixmap(QRect(300, 295, 160, 65), spriteViento);
    }

    painter.setPen(Qt::black);
    painter.drawText(20, 30, "Nivel 1: Piscina de entrenamiento");
    painter.drawText(20, 55, "Espacio: saltar | A/D: mover | E: impulso | R: reiniciar");
    painter.drawText(20, 80, "1 Facil | 2 Normal | 3 Dificil");

    painter.drawText(20, 115, "Dificultad: " + dificultad.getNombre());
    painter.drawText(20, 140, "Puntaje actual: " + QString::number(puntaje));
    painter.drawText(20, 165, "Mejor puntaje: " + QString::number(mejorPuntaje));
    painter.drawText(20, 190, "Puntaje minimo: " + QString::number(dificultad.getPuntajeMinimo()));
    painter.drawText(20, 215, "Intentos restantes: " + QString::number(intentosRestantes));
    painter.drawText(20, 240, "Error de entrada: " + QString::number(errorEntrada, 'f', 1));
    painter.drawText(20, 265, "Viento: " + QString::number(vientoLateral));

    if (jugadorEnZonaViento) {
        painter.setPen(Qt::darkBlue);
        painter.drawText(20, 290, "El personaje esta dentro de la zona de viento");
    }

    painter.setPen(Qt::black);
    painter.drawText(20, 320, "Energia electromagnetica:");

    float porcentajeEnergia = 0.0f;

    if (jugador->getEnergiaMaxima() > 0.0f) {
        porcentajeEnergia = jugador->getEnergia() / jugador->getEnergiaMaxima();
    }

    painter.setBrush(QBrush(Qt::white));
    painter.drawRect(QRectF(20, 330, 150, 15));

    painter.setBrush(QBrush(QColor(255, 220, 60)));
    painter.drawRect(QRectF(20, 330, 150 * porcentajeEnergia, 15));

    painter.setPen(QColor(255, 255, 0));
    painter.setBrush(QColor(255, 255, 0, 45));
    painter.drawRect(zonaMeta);

    painter.setPen(Qt::black);
    QLinearGradient aguaGradiente(piscina.topLeft(), piscina.bottomLeft());
    aguaGradiente.setColorAt(0.0, QColor(60, 190, 255));
    aguaGradiente.setColorAt(1.0, QColor(0, 80, 210));

    painter.setPen(QPen(QColor(0, 60, 150), 2));
    painter.setBrush(QBrush(aguaGradiente));
    painter.drawRoundedRect(piscina, 8, 8);

    painter.setPen(QPen(QColor(220, 250, 255), 2));
    for (int i = 0; i < 5; i++) {
        painter.drawLine(
            piscina.x() + 20 + i * 45,
            piscina.y() + 12,
            piscina.x() + 50 + i * 45,
            piscina.y() + 12
            );
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
    jugador->dibujar(painter);

    if (intentoTerminado || nivelSuperado || nivelPerdido) {
        painter.setBrush(QBrush(Qt::white));
        painter.setPen(Qt::black);
        painter.drawRect(QRectF(205, 205, 410, 185));

        if (nivelSuperado) {
            painter.drawText(270, 240, "NIVEL SUPERADO");
        }
        else if (nivelPerdido) {
            painter.drawText(270, 240, "NIVEL FALLIDO");
        }
        else {
            painter.drawText(270, 240, "Intento terminado");
        }

        painter.drawText(270, 270, "Puntaje obtenido: " + QString::number(puntaje));
        painter.drawText(270, 295, "Puntaje minimo: " + QString::number(dificultad.getPuntajeMinimo()));
        painter.drawText(270, 320, "Error de entrada: " + QString::number(errorEntrada, 'f', 1));
        painter.drawText(270, 345, "Intentos restantes: " + QString::number(intentosRestantes));
        painter.drawText(270, 370, "Presiona R para continuar/reiniciar");
    }
}

void NivelPiscinaEntrenamiento::teclaPresionada(int tecla)
{
    if (tecla == Qt::Key_Space) {
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