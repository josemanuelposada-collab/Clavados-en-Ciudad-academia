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
const float ANCHO_BASE = 800.0f;
const float ALTO_BASE = 600.0f;
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
    setMinimumSize(800, 600);

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
    QLinearGradient fondo(0, 0, 800, 600);
    fondo.setColorAt(0.0, QColor(14, 28, 55));
    fondo.setColorAt(0.45, QColor(27, 111, 145));
    fondo.setColorAt(1.0, QColor(11, 20, 42));
    painter.fillRect(QRectF(0, 0, 800, 600), fondo);

    painter.setPen(QPen(QColor(150, 230, 255, 70), 2));
    for (int i = 0; i < 12; ++i) {
        int y = 85 + i * 38;
        painter.drawLine(0, y, 800, y + (i % 2 == 0 ? 18 : -18));
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 24));
    painter.drawRoundedRect(QRectF(92, 76, 616, 392), 14, 14);
    painter.setBrush(QColor(8, 18, 34, 185));
    painter.drawRoundedRect(QRectF(112, 96, 576, 352), 12, 12);

    QFont titulo = painter.font();
    titulo.setPointSize(25);
    titulo.setBold(true);
    painter.setFont(titulo);
    painter.setPen(QColor(235, 252, 255));
    painter.drawText(QRectF(135, 125, 530, 70), Qt::AlignCenter, "Clavados en Ciudad Academia");

    QFont subtitulo = painter.font();
    subtitulo.setPointSize(11);
    subtitulo.setBold(false);
    painter.setFont(subtitulo);
    painter.setPen(QColor(190, 235, 255));
    painter.drawText(QRectF(155, 198, 490, 48), Qt::AlignCenter,
                     "Mikoto entrena en piscinas experimentales con viento artificial, anillos y un dron supervisor adaptativo.");

    QString dificultad = dificultadSeleccionada == FACIL ? "Facil" : dificultadSeleccionada == NORMAL ? "Normal" : "Dificil";
    painter.setPen(QColor(255, 225, 95));
    painter.drawText(QRectF(185, 270, 430, 34), Qt::AlignCenter, "Dificultad seleccionada: " + dificultad);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 225, 95));
    painter.drawRoundedRect(QRectF(265, 320, 270, 46), 8, 8);
    painter.setPen(QColor(20, 28, 36));
    QFont boton = painter.font();
    boton.setPointSize(13);
    boton.setBold(true);
    painter.setFont(boton);
    painter.drawText(QRectF(265, 320, 270, 46), Qt::AlignCenter, "ENTER  INICIAR");

    QFont ayuda = painter.font();
    ayuda.setPointSize(9);
    ayuda.setBold(false);
    painter.setFont(ayuda);
    painter.setPen(QColor(222, 245, 255));
    painter.drawText(QRectF(135, 392, 530, 22), Qt::AlignCenter, "1 Facil   2 Normal   3 Dificil   F11 Pantalla completa");
    painter.drawText(QRectF(135, 420, 530, 22), Qt::AlignCenter, "Controles: Espacio/WASD/Flechas, E impulso, R reiniciar, Esc pausa");
}

void GameWidget::dibujarPausa(QPainter& painter)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 145));
    painter.drawRect(QRectF(0, 0, 800, 600));
    painter.setBrush(QColor(245, 250, 255, 235));
    painter.drawRoundedRect(QRectF(230, 205, 340, 160), 10, 10);
    painter.setPen(QColor(18, 32, 44));
    QFont fuente = painter.font();
    fuente.setPointSize(18);
    fuente.setBold(true);
    painter.setFont(fuente);
    painter.drawText(QRectF(230, 230, 340, 45), Qt::AlignCenter, "PAUSA");

    fuente.setPointSize(10);
    fuente.setBold(false);
    painter.setFont(fuente);
    painter.drawText(QRectF(250, 292, 300, 24), Qt::AlignCenter, "Enter/Esc: continuar");
    painter.drawText(QRectF(250, 320, 300, 24), Qt::AlignCenter, "M: volver al inicio");
}

void GameWidget::dibujarCampaniaCompletada(QPainter& painter)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 135));
    painter.drawRect(QRectF(0, 0, 800, 600));

    painter.setBrush(QColor(246, 252, 255, 242));
    painter.drawRoundedRect(QRectF(170, 160, 460, 235), 12, 12);

    QFont fuente = painter.font();
    fuente.setPointSize(20);
    fuente.setBold(true);
    painter.setFont(fuente);
    painter.setPen(QColor(12, 36, 56));
    painter.drawText(QRectF(190, 198, 420, 48), Qt::AlignCenter, "CAMPANA COMPLETADA");

    fuente.setPointSize(10);
    fuente.setBold(false);
    painter.setFont(fuente);
    painter.setPen(QColor(30, 55, 72));
    painter.drawText(QRectF(220, 260, 360, 28), Qt::AlignCenter, "Mikoto supero la piscina de entrenamiento y la torre experimental.");
    painter.drawText(QRectF(220, 298, 360, 28), Qt::AlignCenter, "R: repetir nivel final    M: volver al inicio");
    painter.drawText(QRectF(220, 326, 360, 28), Qt::AlignCenter, "Tab: revisar otro nivel    F11: pantalla completa");
}

void GameWidget::dibujarMarcoJuego(QPainter& painter)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(5, 12, 22, 150));
    painter.drawRoundedRect(QRectF(592, 14, 188, 84), 8, 8);

    painter.setPen(QColor(240, 252, 255));
    QFont fuente = painter.font();
    fuente.setPointSize(8);
    fuente.setBold(true);
    painter.setFont(fuente);
    painter.drawText(610, 35, "Nivel " + QString::number(nivelActual + 1) + "/" + QString::number(niveles.size()));
    painter.drawText(610, 58, nivel()->nombreNivel());

    fuente.setBold(false);
    painter.setFont(fuente);
    painter.setPen(QColor(190, 230, 245));
    painter.drawText(610, 82, "H ayuda  |  M menu");

    if (nivel()->estaSuperado() && nivelActual < static_cast<int>(niveles.size()) - 1) {
        painter.setBrush(QColor(255, 225, 95));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(QRectF(565, 110, 205, 34), 7, 7);
        painter.setPen(QColor(15, 25, 35));
        painter.drawText(QRectF(565, 110, 205, 34), Qt::AlignCenter, "Enter: siguiente nivel");
    }

    if (mostrarAyuda) {
        painter.setBrush(QColor(5, 12, 22, 178));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(QRectF(470, 486, 310, 92), 8, 8);
        painter.setPen(Qt::white);
        painter.drawText(490, 512, "Objetivo: superar ambos niveles.");
        painter.drawText(490, 536, "1/2/3 dificultad. Esc pausa. M menu.");
        painter.drawText(490, 560, "Tab permite revisar niveles para demo.");
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

        nivel()->dibujar(painter);
        dibujarMarcoJuego(painter);

        if (campaniaCompletada()) {
            dibujarCampaniaCompletada(painter);
        }

        if (estadoPantalla == PANTALLA_PAUSA) {
            dibujarPausa(painter);
        }
    }
    catch (const JuegoException& error) {
        painter.fillRect(QRectF(0, 0, 800, 600), QColor(25, 25, 25));
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
