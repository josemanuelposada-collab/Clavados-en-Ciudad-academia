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
    painter.drawRoundedRect(rect, 8, 8);
}

void barra(QPainter& painter, const QRectF& rect, float porcentaje, const QColor& color)
{
    porcentaje = std::clamp(porcentaje, 0.0f, 1.0f);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 62));
    painter.drawRoundedRect(rect, 5, 5);
    painter.setBrush(color);
    painter.drawRoundedRect(QRectF(rect.x(), rect.y(), rect.width() * porcentaje, rect.height()), 5, 5);
}
}

NivelRutaAnillos::NivelRutaAnillos()
    : jugador(new Personaje()),
      dron(new DronVigilante(385.0f, 160.0f)),
      piscinaFinal(250.0f, 1510.0f, 300.0f, 78.0f),
      zonaViento(76.0f, 410.0f, 648.0f, 230.0f),
      zonaVelocidad(96.0f, 870.0f, 608.0f, 170.0f),
      anillosRecolectados(0),
      golpes(0),
      puntaje(0),
      tiempoRestante(46.0f),
      tiempoTotal(46.0f),
      tiempoNivel(0.0f),
      alturaMundo(1620.0f),
      camaraY(0.0f),
      velocidadVertical(0.0f),
      velocidadHorizontal(0.0f),
      nivelSuperado(false),
      nivelPerdido(false)
{
    dificultad.configurar(NORMAL);
    spriteAlarma.load(":/recursos/sprites/alarma.png");
    spriteViento.load(":/recursos/sprites/viento_turbulencia.png");
    spritePiscinaFinal.load(":/recursos/sprites/piscina_ciudad_academia.png");
    spriteColumnaTorre.load(":/recursos/sprites/torre_columna_luz.png");
    jugador->colocarEn(382.0f, 92.0f);
    jugador->setEnAire(true);
    jugador->setVY(120.0f);
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
    anillos.push_back(new Anillo(386.0f, 245.0f));
    anillos.push_back(new Anillo(175.0f, 455.0f));
    anillos.push_back(new Anillo(575.0f, 645.0f));
    anillos.push_back(new Anillo(300.0f, 850.0f));
    anillos.push_back(new Anillo(525.0f, 1080.0f));
    anillos.push_back(new Anillo(382.0f, 1310.0f));

    float factor = dificultad.getVelocidadPlataforma();
    obstaculos.push_back(new Obstaculo(130.0f, 360.0f, 54.0f, 54.0f, 76.0f * factor, 0.0f, BOYA));
    obstaculos.push_back(new Obstaculo(575.0f, 585.0f, 58.0f, 58.0f, -92.0f * factor, 0.0f, BARRIL));
    obstaculos.push_back(new Obstaculo(210.0f, 810.0f, 54.0f, 54.0f, 105.0f * factor, 0.0f, MINA));
    obstaculos.push_back(new Obstaculo(560.0f, 1190.0f, 56.0f, 56.0f, -115.0f * factor, 0.0f, BLOQUEO));

    if (anillos.empty() || obstaculos.empty()) {
        throw JuegoException("La torre experimental no pudo crear sus entidades.");
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
        obstaculo->rebotarHorizontal(76.0f, 724.0f);
    }

    float yDron = std::clamp(jugador->getY() - 145.0f, 95.0f, alturaMundo - 250.0f);
    dron->colocarEn(dron->getX(), yDron);
    dron->actualizar(dt, *jugador);

    verificarInteracciones();
    calcularPuntaje();
    actualizarCamara();

    if (tiempoRestante <= 0.0f || golpes >= 4) {
        nivelPerdido = true;
    }
}

void NivelRutaAnillos::aplicarMovimientoJugador(float dt)
{
    float gravedad = 260.0f + dificultad.getIntensidadViento() * 0.35f;
    float controlHorizontal = 0.0f;

    if (teclas.contains(Qt::Key_A) || teclas.contains(Qt::Key_Left)) {
        controlHorizontal -= 500.0f;
    }
    if (teclas.contains(Qt::Key_D) || teclas.contains(Qt::Key_Right)) {
        controlHorizontal += 500.0f;
    }
    if (teclas.contains(Qt::Key_E) || teclas.contains(Qt::Key_Up) || teclas.contains(Qt::Key_W)) {
        velocidadVertical -= 220.0f * dt;
    }
    if (teclas.contains(Qt::Key_S) || teclas.contains(Qt::Key_Down)) {
        velocidadVertical += 180.0f * dt;
    }

    velocidadHorizontal += controlHorizontal * dt;
    velocidadHorizontal *= 0.93f;
    velocidadVertical += gravedad * dt;

    if (zonaViento.intersects(jugador->rect())) {
        float turbulencia = dificultad.getIntensidadViento() * (0.9f + 0.35f * std::sin(tiempoNivel * 3.4f));
        velocidadHorizontal += turbulencia * dt;
    }

    if (zonaVelocidad.intersects(jugador->rect())) {
        velocidadVertical *= 0.986f;
    }

    velocidadVertical = std::clamp(velocidadVertical, 80.0f, 580.0f);
    velocidadHorizontal = std::clamp(velocidadHorizontal, -260.0f, 260.0f);

    float nuevoX = std::clamp(jugador->getX() + velocidadHorizontal * dt, 40.0f, 725.0f);
    float nuevoY = std::clamp(jugador->getY() + velocidadVertical * dt, 82.0f, alturaMundo - 62.0f);
    jugador->colocarEn(nuevoX, nuevoY);
    jugador->setVX(velocidadHorizontal);
    jugador->setVY(velocidadVertical);
}

void NivelRutaAnillos::verificarInteracciones()
{
    for (Anillo* anillo : anillos) {
        if (!anillo->estaRecolectado() && anillo->rect().intersects(jugador->rect())) {
            anillo->recolectar();
            anillosRecolectados++;
            dron->aprender(35.0f);
            eventosSonido.push_back(SONIDO_ANILLO);
        }
    }

    for (Obstaculo* obstaculo : obstaculos) {
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
        calcularPuntaje();
        nivelSuperado = puntaje >= 55;
        nivelPerdido = !nivelSuperado;
        eventosSonido.push_back(SONIDO_AGUA);
        if (nivelSuperado) {
            dron->registrarAciertoJugador();
            eventosSonido.push_back(SONIDO_NIVEL);
        }
    }
}

void NivelRutaAnillos::calcularPuntaje()
{
    int base = anillosRecolectados * 13;
    int bonusTiempo = static_cast<int>(std::max(0.0f, tiempoRestante) * 0.65f);
    int bonusControl = zonaVelocidad.intersects(jugador->rect()) ? 6 : 0;
    int penalizacion = golpes * 12;
    puntaje = std::clamp(base + bonusTiempo + bonusControl - penalizacion, 0, 100);
}

void NivelRutaAnillos::actualizarCamara()
{
    camaraY = std::clamp(jugador->getY() - 255.0f, 0.0f, alturaMundo - 600.0f);
}

void NivelRutaAnillos::dibujarEscenario(QPainter& painter)
{
    QLinearGradient fondo(0, camaraY, 0, camaraY + 600);
    fondo.setColorAt(0.0, QColor(18, 40, 74));
    fondo.setColorAt(0.55, QColor(28, 125, 166));
    fondo.setColorAt(1.0, QColor(12, 35, 64));
    painter.fillRect(QRectF(0, camaraY, 800, 600), fondo);

    if (!spriteColumnaTorre.isNull()) {
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

    if (!spritePiscinaFinal.isNull()) {
        painter.drawPixmap(QRect(205, static_cast<int>(piscinaFinal.y() - 38), 390, 134), spritePiscinaFinal);
    }
    else {
        QLinearGradient agua(piscinaFinal.topLeft(), piscinaFinal.bottomLeft());
        agua.setColorAt(0.0, QColor(95, 225, 255));
        agua.setColorAt(1.0, QColor(0, 94, 190));
        painter.setPen(QPen(QColor(210, 250, 255), 2));
        painter.setBrush(agua);
        painter.drawRoundedRect(piscinaFinal, 10, 10);
    }
    painter.setPen(QColor(255, 255, 255));
    painter.drawText(QRectF(piscinaFinal.x(), piscinaFinal.y() + 24, piscinaFinal.width(), 28), Qt::AlignCenter, "Piscina final");

    for (Anillo* anillo : anillos) {
        anillo->dibujar(painter);
    }

    for (Obstaculo* obstaculo : obstaculos) {
        obstaculo->dibujar(painter);
    }

    dron->dibujar(painter);
    jugador->dibujar(painter);
}

void NivelRutaAnillos::dibujarHud(QPainter& painter)
{
    panel(painter, QRectF(18, 18, 320, 154), QColor(5, 14, 28, 188));

    QFont fuente = painter.font();
    fuente.setPointSize(12);
    fuente.setBold(true);
    painter.setFont(fuente);
    painter.setPen(QColor(238, 252, 255));
    painter.drawText(34, 46, "Torre experimental");

    fuente.setPointSize(8);
    fuente.setBold(false);
    painter.setFont(fuente);
    painter.setPen(QColor(185, 230, 246));
    painter.drawText(34, 70, "A/D o Flechas: corregir | E/W: frenar caida | S: acelerar");

    painter.setPen(QColor(255, 225, 95));
    painter.drawText(34, 100, "Tiempo");
    barra(painter, QRectF(104, 90, 170, 12), tiempoRestante / tiempoTotal, QColor(255, 225, 95));
    painter.drawText(282, 101, QString::number(tiempoRestante, 'f', 1));

    painter.setPen(QColor(120, 235, 255));
    painter.drawText(34, 126, "Descenso");
    barra(painter, QRectF(104, 116, 170, 12), jugador->getY() / (alturaMundo - 80.0f), QColor(120, 235, 255));

    painter.setPen(QColor(230, 245, 255));
    painter.drawText(34, 151, "Anillos: " + QString::number(anillosRecolectados) + "/" + QString::number(anillos.size()) +
                               "   Golpes: " + QString::number(golpes) + "/4   Pts: " + QString::number(puntaje));

    panel(painter, QRectF(562, 155, 208, 54), QColor(255, 255, 255, 190));
    painter.setPen(QColor(20, 38, 52));
    painter.drawText(580, 178, zonaViento.intersects(jugador->rect()) ? "Viento lateral activo" : "Viento fuera de alcance");
    painter.drawText(580, 198, zonaVelocidad.intersects(jugador->rect()) ? "Zona de velocidad variable" : "Dron supervisando");
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

void NivelRutaAnillos::reiniciarNivel()
{
    teclas.clear();
    anillosRecolectados = 0;
    golpes = 0;
    puntaje = 0;
    tiempoRestante = tiempoTotal;
    tiempoNivel = 0.0f;
    camaraY = 0.0f;
    velocidadHorizontal = 0.0f;
    velocidadVertical = 120.0f;
    nivelSuperado = false;
    nivelPerdido = false;
    jugador->colocarEn(382.0f, 92.0f);
    jugador->setEnAire(true);
    jugador->setVX(0.0f);
    jugador->setVY(velocidadVertical);
    dron->colocarEn(385.0f, 160.0f);
    dron->reiniciarMemoriaParcial();

    liberarEntidades();
    crearEntidades();
}

void NivelRutaAnillos::cambiarDificultad(TipoDificultad tipo)
{
    dificultad.configurar(tipo);

    if (tipo == FACIL) {
        tiempoTotal = 54.0f;
    }
    else if (tipo == NORMAL) {
        tiempoTotal = 46.0f;
    }
    else {
        tiempoTotal = 38.0f;
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
