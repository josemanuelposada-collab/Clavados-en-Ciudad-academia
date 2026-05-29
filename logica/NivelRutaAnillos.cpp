#include "NivelRutaAnillos.h"
#include "JuegoException.h"
#include <QKeyEvent>
#include <QLinearGradient>
#include <algorithm>
#include <cmath>

NivelRutaAnillos::NivelRutaAnillos()
    : jugador(new Personaje()),
      dron(new DronVigilante(385.0f, 80.0f)),
      anillosRecolectados(0),
      golpes(0),
      puntaje(0),
      tiempoRestante(42.0f),
      tiempoTotal(42.0f),
      tiempoNivel(0.0f),
      nivelSuperado(false),
      nivelPerdido(false)
{
    dificultad.configurar(NORMAL);
    spriteAlarma.load(":/recursos/sprites/alarma.png");
    jugador->colocarEn(70.0f, 500.0f);
    jugador->setEnAire(true);
    crearEntidades();
}

NivelRutaAnillos::~NivelRutaAnillos()
{
    liberarEntidades();
    delete jugador;
    delete dron;
}

void NivelRutaAnillos::crearEntidades()
{
    anillos.push_back(new Anillo(165.0f, 430.0f));
    anillos.push_back(new Anillo(285.0f, 315.0f));
    anillos.push_back(new Anillo(430.0f, 230.0f));
    anillos.push_back(new Anillo(585.0f, 315.0f));
    anillos.push_back(new Anillo(690.0f, 145.0f));

    float factor = dificultad.getVelocidadPlataforma();
    obstaculos.push_back(new Obstaculo(245.0f, 395.0f, 52.0f, 52.0f, 70.0f * factor, 0.0f, BOYA));
    obstaculos.push_back(new Obstaculo(390.0f, 300.0f, 56.0f, 56.0f, -82.0f * factor, 0.0f, BARRIL));
    obstaculos.push_back(new Obstaculo(560.0f, 200.0f, 50.0f, 50.0f, 96.0f * factor, 0.0f, MINA));

    if (anillos.empty() || obstaculos.empty()) {
        throw JuegoException("El nivel de anillos no pudo crear sus entidades.");
    }
}

void NivelRutaAnillos::liberarEntidades()
{
    for (Anillo* anillo : anillos) {
        delete anillo;
    }
    anillos.clear();

    for (Obstaculo* obstaculo : obstaculos) {
        delete obstaculo;
    }
    obstaculos.clear();
}

void NivelRutaAnillos::actualizar(float dt)
{
    if (nivelSuperado || nivelPerdido) {
        return;
    }

    tiempoNivel += dt;
    tiempoRestante -= dt;

    aplicarMovimientoJugador(dt);

    for (Anillo* anillo : anillos) {
        anillo->actualizar(dt);
    }

    for (Obstaculo* obstaculo : obstaculos) {
        obstaculo->actualizar(dt);
        obstaculo->rebotarHorizontal(80.0f, 760.0f);
    }

    dron->actualizar(dt, *jugador);
    verificarInteracciones();
    calcularPuntaje();

    if (anillosRecolectados == static_cast<int>(anillos.size())) {
        nivelSuperado = true;
        dron->registrarAciertoJugador();
    }

    if (tiempoRestante <= 0.0f || golpes >= 4) {
        nivelPerdido = true;
    }
}

void NivelRutaAnillos::aplicarMovimientoJugador(float dt)
{
    float dx = 0.0f;
    float dy = 0.0f;
    float velocidad = 225.0f;
    float turbulencia = dificultad.getIntensidadViento() * std::sin(tiempoNivel * 2.6f) * 0.12f;

    if (teclas.contains(Qt::Key_A) || teclas.contains(Qt::Key_Left)) {
        dx -= velocidad * dt;
    }
    if (teclas.contains(Qt::Key_D) || teclas.contains(Qt::Key_Right)) {
        dx += velocidad * dt;
    }
    if (teclas.contains(Qt::Key_W) || teclas.contains(Qt::Key_Up)) {
        dy -= velocidad * dt;
    }
    if (teclas.contains(Qt::Key_S) || teclas.contains(Qt::Key_Down)) {
        dy += velocidad * dt;
    }

    float nuevoX = std::clamp(jugador->getX() + dx + turbulencia * dt, 20.0f, 745.0f);
    float nuevoY = std::clamp(jugador->getY() + dy, 90.0f, 510.0f);
    jugador->colocarEn(nuevoX, nuevoY);
}

void NivelRutaAnillos::verificarInteracciones()
{
    for (Anillo* anillo : anillos) {
        if (!anillo->estaRecolectado() && anillo->rect().intersects(jugador->rect())) {
            anillo->recolectar();
            anillosRecolectados++;
        }
    }

    for (Obstaculo* obstaculo : obstaculos) {
        if (obstaculo->rect().intersects(jugador->rect())) {
            golpes++;
            dron->aprender(170.0f);
            jugador->colocarEn(70.0f, 500.0f);
            return;
        }
    }

    if (dron->rect().intersects(jugador->rect())) {
        golpes++;
        tiempoRestante = std::max(0.0f, tiempoRestante - 3.0f);
        jugador->colocarEn(70.0f, 500.0f);
    }
}

void NivelRutaAnillos::calcularPuntaje()
{
    int base = anillosRecolectados * 18;
    int bonusTiempo = static_cast<int>(std::max(0.0f, tiempoRestante) * 0.7f);
    int penalizacion = golpes * 12;
    puntaje = std::clamp(base + bonusTiempo - penalizacion, 0, 100);
}

void NivelRutaAnillos::dibujar(QPainter& painter)
{
    QLinearGradient fondo(0, 0, 800, 600);
    fondo.setColorAt(0.0, QColor(25, 55, 95));
    fondo.setColorAt(0.45, QColor(45, 150, 190));
    fondo.setColorAt(1.0, QColor(20, 95, 145));
    painter.fillRect(QRectF(0, 0, 800, 600), fondo);

    painter.setPen(QPen(QColor(210, 245, 255, 70), 2));
    for (int i = 0; i < 9; ++i) {
        int y = 110 + i * 52;
        painter.drawLine(35, y, 765, y + static_cast<int>(18 * std::sin(tiempoNivel + i)));
    }

    painter.setPen(Qt::white);
    painter.drawText(20, 30, "Nivel 2: Ruta de anillos gravitacionales");
    painter.drawText(20, 55, "WASD/Flechas: nadar | R: reiniciar | 1/2/3: dificultad");
    painter.drawText(20, 85, "Anillos: " + QString::number(anillosRecolectados) + "/" + QString::number(anillos.size()));
    painter.drawText(20, 110, "Golpes: " + QString::number(golpes) + "/4");
    painter.drawText(20, 135, "Tiempo: " + QString::number(tiempoRestante, 'f', 1));
    painter.drawText(20, 160, "Puntaje: " + QString::number(puntaje));

    if (tiempoRestante < 10.0f && !spriteAlarma.isNull()) {
        painter.drawPixmap(QRect(690, 25, 64, 64), spriteAlarma);
    }

    for (Anillo* anillo : anillos) {
        anillo->dibujar(painter);
    }

    for (Obstaculo* obstaculo : obstaculos) {
        obstaculo->dibujar(painter);
    }

    dron->dibujar(painter);
    jugador->dibujar(painter);

    if (nivelSuperado || nivelPerdido) {
        painter.setBrush(QColor(255, 255, 255, 235));
        painter.setPen(Qt::black);
        painter.drawRect(QRectF(205, 210, 410, 165));
        painter.drawText(270, 248, nivelSuperado ? "NIVEL 2 SUPERADO" : "NIVEL 2 FALLIDO");
        painter.drawText(270, 278, "Puntaje final: " + QString::number(puntaje));
        painter.drawText(270, 308, "Anillos recolectados: " + QString::number(anillosRecolectados));
        painter.drawText(270, 338, "Presiona R para reiniciar");
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

void NivelRutaAnillos::reiniciarNivel()
{
    teclas.clear();
    anillosRecolectados = 0;
    golpes = 0;
    puntaje = 0;
    tiempoRestante = tiempoTotal;
    tiempoNivel = 0.0f;
    nivelSuperado = false;
    nivelPerdido = false;
    jugador->colocarEn(70.0f, 500.0f);
    jugador->setEnAire(true);
    dron->reiniciarMemoriaParcial();

    liberarEntidades();
    crearEntidades();
}

void NivelRutaAnillos::cambiarDificultad(TipoDificultad tipo)
{
    dificultad.configurar(tipo);

    if (tipo == FACIL) {
        tiempoTotal = 48.0f;
    }
    else if (tipo == NORMAL) {
        tiempoTotal = 42.0f;
    }
    else {
        tiempoTotal = 35.0f;
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
    return "Ruta de anillos gravitacionales";
}
