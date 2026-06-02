#include "GameWidget.h"
#include <QPainter>
#include <QApplication>
#include <QLinearGradient>
#include <QUrl>
#include <algorithm>
#include "../logica/NivelPiscinaEntrenamiento.h"
#include "../logica/NivelRutaAnillos.h"
#include "../logica/JuegoException.h"

namespace {
const float ANCHO_BASE = 1280.0f;
const float ALTO_BASE = 720.0f;
const float ANCHO_MUNDO = 800.0f;
const float ALTO_MUNDO = 600.0f;
const float ESCALA_MUNDO = 1.2f;
}

GameWidget::GameWidget(QWidget* parent)
    : QWidget(parent),
      timer(nullptr),
      nivelActual(0),
      mostrarAyuda(true),
      estadoPantalla(PANTALLA_INICIO),
      dificultadSeleccionada(NORMAL),
      sonidoFondo(nullptr),
      sonidoSalto(nullptr),
      sonidoAnillo(nullptr),
      sonidoColision(nullptr),
      sonidoAgua(nullptr),
      sonidoNivel(nullptr)
{
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(960, 540);
    resize(1280, 720);

    cargarNiveles();
    cargarSonidos();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GameWidget::actualizar);
    timer->start(16);
}

GameWidget::~GameWidget() = default;

NivelJuego* GameWidget::nivel()
{
    if (niveles.empty()) {
        throw JuegoException("No hay niveles cargados.");
    }

    return niveles[nivelActual].get();
}

void GameWidget::cargarNiveles()
{
    niveles.push_back(std::make_unique<NivelPiscinaEntrenamiento>());
    niveles.push_back(std::make_unique<NivelRutaAnillos>());
}

void GameWidget::cargarSonidos()
{
    sonidoFondo = new QSoundEffect(this);
    sonidoFondo->setSource(QUrl("qrc:/recursos/audio/fondo_entrenamiento.wav"));
    sonidoFondo->setLoopCount(QSoundEffect::Infinite);
    sonidoFondo->setVolume(0.18f);

    sonidoSalto = new QSoundEffect(this);
    sonidoSalto->setSource(QUrl("qrc:/recursos/audio/salto_impulso.wav"));
    sonidoSalto->setVolume(0.55f);

    sonidoAnillo = new QSoundEffect(this);
    sonidoAnillo->setSource(QUrl("qrc:/recursos/audio/anillo_bonus.wav"));
    sonidoAnillo->setVolume(0.50f);

    sonidoColision = new QSoundEffect(this);
    sonidoColision->setSource(QUrl("qrc:/recursos/audio/colision_alerta.wav"));
    sonidoColision->setVolume(0.48f);

    sonidoAgua = new QSoundEffect(this);
    sonidoAgua->setSource(QUrl("qrc:/recursos/audio/entrada_agua.wav"));
    sonidoAgua->setVolume(0.55f);

    sonidoNivel = new QSoundEffect(this);
    sonidoNivel->setSource(QUrl("qrc:/recursos/audio/nivel_superado.wav"));
    sonidoNivel->setVolume(0.52f);
}

void GameWidget::avanzarNivel()
{
    if (nivel()->estaSuperado() && nivelActual < static_cast<int>(niveles.size()) - 1) {
        nivelActual++;
        nivel()->reiniciarNivel();
        reproducirEventoSonido(SONIDO_NIVEL);
    }
}

void GameWidget::aplicarDificultadSeleccionada()
{
    for (const auto& nivelJuego : niveles) {
        NivelPiscinaEntrenamiento* piscina = dynamic_cast<NivelPiscinaEntrenamiento*>(nivelJuego.get());
        if (piscina != nullptr) {
            piscina->cambiarDificultad(dificultadSeleccionada);
        }

        NivelRutaAnillos* ruta = dynamic_cast<NivelRutaAnillos*>(nivelJuego.get());
        if (ruta != nullptr) {
            ruta->cambiarDificultad(dificultadSeleccionada);
        }
    }
}

void GameWidget::iniciarPartida()
{
    aplicarDificultadSeleccionada();
    nivelActual = 0;
    estadoPantalla = PANTALLA_JUGANDO;
    mostrarAyuda = false;
    if (sonidoFondo != nullptr && !sonidoFondo->isPlaying()) {
        sonidoFondo->play();
    }
    reproducirEventoSonido(SONIDO_NIVEL);
}

void GameWidget::reiniciarCampania()
{
    for (const auto& nivelJuego : niveles) {
        nivelJuego->reiniciarNivel();
    }

    nivelActual = 0;
    estadoPantalla = PANTALLA_INICIO;
    if (sonidoFondo != nullptr) {
        sonidoFondo->stop();
    }
}

void GameWidget::reproducirEventoSonido(EventoSonidoJuego evento)
{
    QSoundEffect* efecto = nullptr;

    if (evento == SONIDO_SALTO) {
        efecto = sonidoSalto;
    }
    else if (evento == SONIDO_ANILLO) {
        efecto = sonidoAnillo;
    }
    else if (evento == SONIDO_COLISION) {
        efecto = sonidoColision;
    }
    else if (evento == SONIDO_AGUA) {
        efecto = sonidoAgua;
    }
    else if (evento == SONIDO_NIVEL) {
        efecto = sonidoNivel;
    }

    if (efecto != nullptr) {
        efecto->play();
    }
}

void GameWidget::procesarSonidosNivel()
{
    for (EventoSonidoJuego evento : nivel()->consumirEventosSonido()) {
        reproducirEventoSonido(evento);
    }
}

void GameWidget::configurarLienzo(QPainter& painter)
{
    painter.fillRect(rect(), QColor(8, 12, 22));

    float escalaX = width() / ANCHO_BASE;
    float escalaY = height() / ALTO_BASE;
    float escala = std::min(escalaX, escalaY);
    float margenX = (width() - ANCHO_BASE * escala) / 2.0f;
    float margenY = (height() - ALTO_BASE * escala) / 2.0f;

    painter.translate(margenX, margenY);
    painter.scale(escala, escala);
    painter.setClipRect(QRectF(0, 0, ANCHO_BASE, ALTO_BASE));
}

void GameWidget::dibujarInicio(QPainter& painter)
{
    QLinearGradient fondo(0, 0, 1280, 720);
    fondo.setColorAt(0.0, QColor(14, 28, 55));
    fondo.setColorAt(0.45, QColor(27, 111, 145));
    fondo.setColorAt(1.0, QColor(11, 20, 42));
    painter.fillRect(QRectF(0, 0, ANCHO_BASE, ALTO_BASE), fondo);

    painter.setPen(QPen(QColor(150, 230, 255, 70), 2));
    for (int i = 0; i < 14; ++i) {
        int y = 70 + i * 42;
        painter.drawLine(0, y, 1280, y + (i % 2 == 0 ? 24 : -24));
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 24));
    painter.drawRoundedRect(QRectF(245, 88, 790, 450), 14, 14);
    painter.setBrush(QColor(8, 18, 34, 185));
    painter.drawRoundedRect(QRectF(270, 112, 740, 402), 12, 12);

    QFont titulo = painter.font();
    titulo.setPointSize(32);
    titulo.setBold(true);
    painter.setFont(titulo);
    painter.setPen(QColor(235, 252, 255));
    painter.drawText(QRectF(310, 152, 660, 78), Qt::AlignCenter, "Clavados en Ciudad Academia");

    QFont subtitulo = painter.font();
    subtitulo.setPointSize(13);
    subtitulo.setBold(false);
    painter.setFont(subtitulo);
    painter.setPen(QColor(190, 235, 255));
    painter.drawText(QRectF(350, 246, 580, 54), Qt::AlignCenter,
                     "Mikoto entrena en piscinas experimentales con viento artificial, anillos y un dron supervisor adaptativo.");

    QString dificultad = dificultadSeleccionada == FACIL ? "Facil" : dificultadSeleccionada == NORMAL ? "Normal" : "Dificil";
    painter.setPen(QColor(255, 225, 95));
    painter.drawText(QRectF(400, 330, 480, 34), Qt::AlignCenter, "Dificultad seleccionada: " + dificultad);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 225, 95));
    painter.drawRoundedRect(QRectF(490, 390, 300, 54), 8, 8);
    painter.setPen(QColor(20, 28, 36));
    QFont boton = painter.font();
    boton.setPointSize(14);
    boton.setBold(true);
    painter.setFont(boton);
    painter.drawText(QRectF(490, 390, 300, 54), Qt::AlignCenter, "ENTER  INICIAR");

    QFont ayuda = painter.font();
    ayuda.setPointSize(10);
    ayuda.setBold(false);
    painter.setFont(ayuda);
    painter.setPen(QColor(222, 245, 255));
    painter.drawText(QRectF(330, 468, 620, 24), Qt::AlignCenter, "1 Facil   2 Normal   3 Dificil   F11 Pantalla completa");
    painter.drawText(QRectF(330, 498, 620, 24), Qt::AlignCenter, "Controles: Espacio/WASD/Flechas, E impulso, R reiniciar, Esc pausa");
}

void GameWidget::dibujarPausa(QPainter& painter)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 145));
    painter.drawRect(QRectF(0, 0, ANCHO_BASE, ALTO_BASE));
    painter.setBrush(QColor(245, 250, 255, 235));
    painter.drawRoundedRect(QRectF(455, 245, 370, 170), 10, 10);
    painter.setPen(QColor(18, 32, 44));
    QFont fuente = painter.font();
    fuente.setPointSize(18);
    fuente.setBold(true);
    painter.setFont(fuente);
    painter.drawText(QRectF(455, 270, 370, 45), Qt::AlignCenter, "PAUSA");

    fuente.setPointSize(10);
    fuente.setBold(false);
    painter.setFont(fuente);
    painter.drawText(QRectF(485, 334, 310, 24), Qt::AlignCenter, "Enter/Esc: continuar");
    painter.drawText(QRectF(485, 362, 310, 24), Qt::AlignCenter, "M: volver al inicio");
}

void GameWidget::dibujarCampaniaCompletada(QPainter& painter)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 135));
    painter.drawRect(QRectF(0, 0, ANCHO_BASE, ALTO_BASE));

    painter.setBrush(QColor(246, 252, 255, 242));
    painter.drawRoundedRect(QRectF(390, 210, 500, 245), 12, 12);

    QFont fuente = painter.font();
    fuente.setPointSize(20);
    fuente.setBold(true);
    painter.setFont(fuente);
    painter.setPen(QColor(12, 36, 56));
    painter.drawText(QRectF(420, 248, 440, 48), Qt::AlignCenter, "CAMPANA COMPLETADA");

    fuente.setPointSize(10);
    fuente.setBold(false);
    painter.setFont(fuente);
    painter.setPen(QColor(30, 55, 72));
    painter.drawText(QRectF(435, 315, 410, 28), Qt::AlignCenter, "Mikoto supero la piscina de entrenamiento y la torre experimental.");
    painter.drawText(QRectF(435, 353, 410, 28), Qt::AlignCenter, "R: repetir nivel final    M: volver al inicio");
    painter.drawText(QRectF(435, 381, 410, 28), Qt::AlignCenter, "Tab: revisar otro nivel    F11: pantalla completa");
}

void GameWidget::dibujarMarcoJuego(QPainter& painter)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(6, 14, 27));
    painter.drawRect(QRectF(960, 0, 320, 720));
    painter.setBrush(QColor(13, 30, 47));
    painter.drawRoundedRect(QRectF(986, 28, 248, 136), 8, 8);

    painter.setPen(QColor(240, 252, 255));
    QFont fuente = painter.font();
    fuente.setPointSize(11);
    fuente.setBold(true);
    painter.setFont(fuente);
    painter.drawText(1010, 62, "Nivel " + QString::number(nivelActual + 1) + "/" + QString::number(niveles.size()));
    painter.drawText(1010, 92, nivel()->nombreNivel());

    fuente.setBold(false);
    painter.setFont(fuente);
    painter.setPen(QColor(190, 230, 245));
    painter.drawText(1010, 124, "H ayuda  |  M menu");
    painter.drawText(1010, 148, "F11 pantalla completa");

    if (nivel()->estaSuperado() && nivelActual < static_cast<int>(niveles.size()) - 1) {
        painter.setBrush(QColor(255, 225, 95));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(QRectF(986, 188, 248, 42), 7, 7);
        painter.setPen(QColor(15, 25, 35));
        painter.drawText(QRectF(986, 188, 248, 42), Qt::AlignCenter, "Enter: siguiente nivel");
    }

    if (mostrarAyuda) {
        painter.setBrush(QColor(18, 40, 60));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(QRectF(986, 480, 248, 150), 8, 8);
        painter.setPen(Qt::white);
        painter.drawText(1010, 512, "Objetivo:");
        painter.drawText(1010, 538, "Superar ambos niveles.");
        painter.drawText(1010, 566, "1/2/3 dificultad.");
        painter.drawText(1010, 594, "Esc pausa. Tab demo.");
    }
}

bool GameWidget::campaniaCompletada()
{
    return nivelActual == static_cast<int>(niveles.size()) - 1 && nivel()->estaSuperado();
}

void GameWidget::actualizar()
{
    try {
        if (estadoPantalla != PANTALLA_JUGANDO) {
            update();
            return;
        }

        nivel()->actualizar(0.016f);
        procesarSonidosNivel();
    }
    catch (const JuegoException&) {
        QApplication::beep();
    }

    update();
}

void GameWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    configurarLienzo(painter);

    try {
        if (estadoPantalla == PANTALLA_INICIO) {
            dibujarInicio(painter);
            return;
        }

        painter.fillRect(QRectF(0, 0, ANCHO_BASE, ALTO_BASE), QColor(8, 12, 22));
        painter.save();
        painter.scale(ESCALA_MUNDO, ESCALA_MUNDO);
        painter.setClipRect(QRectF(0, 0, ANCHO_MUNDO, ALTO_MUNDO));
        nivel()->dibujar(painter);
        painter.restore();
        dibujarMarcoJuego(painter);

        if (campaniaCompletada()) {
            dibujarCampaniaCompletada(painter);
        }

        if (estadoPantalla == PANTALLA_PAUSA) {
            dibujarPausa(painter);
        }
    }
    catch (const JuegoException& error) {
        painter.fillRect(QRectF(0, 0, ANCHO_BASE, ALTO_BASE), QColor(25, 25, 25));
        painter.setPen(Qt::white);
        painter.drawText(60, 80, "Error del juego:");
        painter.drawText(60, 110, error.what());
    }
}

void GameWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_F11) {
        if (window()->isFullScreen()) {
            window()->showNormal();
        }
        else {
            window()->showFullScreen();
        }
        return;
    }

    if (event->key() == Qt::Key_H) {
        mostrarAyuda = !mostrarAyuda;
        return;
    }

    if (event->key() == Qt::Key_1 || event->key() == Qt::Key_2 || event->key() == Qt::Key_3) {
        dificultadSeleccionada = event->key() == Qt::Key_1 ? FACIL : event->key() == Qt::Key_2 ? NORMAL : DIFICIL;
        aplicarDificultadSeleccionada();
    }

    if (estadoPantalla == PANTALLA_INICIO) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Space) {
            iniciarPartida();
        }
        return;
    }

    if (event->key() == Qt::Key_Escape) {
        estadoPantalla = estadoPantalla == PANTALLA_PAUSA ? PANTALLA_JUGANDO : PANTALLA_PAUSA;
        if (estadoPantalla == PANTALLA_PAUSA && sonidoFondo != nullptr) {
            sonidoFondo->setVolume(0.08f);
        }
        else if (sonidoFondo != nullptr) {
            sonidoFondo->setVolume(0.18f);
        }
        return;
    }

    if (estadoPantalla == PANTALLA_PAUSA) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            estadoPantalla = PANTALLA_JUGANDO;
        }
        else if (event->key() == Qt::Key_M) {
            reiniciarCampania();
        }
        return;
    }

    if (event->key() == Qt::Key_M) {
        reiniciarCampania();
        return;
    }

    if (event->key() == Qt::Key_Tab) {
        nivelActual = (nivelActual + 1) % niveles.size();
        niveles[nivelActual]->reiniciarNivel();
        reproducirEventoSonido(SONIDO_NIVEL);
        return;
    }

    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) && nivel()->estaSuperado()) {
        avanzarNivel();
        return;
    }

    nivel()->teclaPresionada(event->key());
}

void GameWidget::keyReleaseEvent(QKeyEvent* event)
{
    if (estadoPantalla != PANTALLA_JUGANDO) {
        return;
    }

    nivel()->teclaLiberada(event->key());
}
