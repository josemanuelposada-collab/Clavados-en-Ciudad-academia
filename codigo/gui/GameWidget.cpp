#include "GameWidget.h"
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QLinearGradient>
#include <QRadialGradient>
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
      estadoPantalla(PANTALLA_INTRO),
      dificultadSeleccionada(NORMAL),
      personajeSeleccionado(PERSONAJE_MIKOTO),
      sonidoFondo(nullptr),
      sonidoSalto(nullptr),
      sonidoAnillo(nullptr),
      sonidoColision(nullptr),
      sonidoAgua(nullptr),
      sonidoNivel(nullptr),
      sonidoMenu(nullptr),
      sonidoGameOver(nullptr),
      tiempoIntro(0.0f),
      tiempoGameOver(0.0f),
      tiempoVictoria(0.0f),
      victoriaProcesada(false),
      motivoGameOver(""),
      ultimoError(""),
      hayError(false)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setMinimumSize(960, 540);
    resize(1280, 720);

    cargarNiveles();
    cargarSonidos();
    if (sonidoMenu != nullptr) {
        sonidoMenu->play();
    }

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
    niveles.reserve(2);
    niveles.push_back(std::make_unique<NivelPiscinaEntrenamiento>());
    niveles.push_back(std::make_unique<NivelRutaAnillos>());
}

void GameWidget::cargarSonidos()
{
    // QSoundEffect hereda de QObject; al usar `this` como padre,
    // Qt libera estos objetos automaticamente junto con GameWidget.
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

    sonidoMenu = new QSoundEffect(this);
    sonidoMenu->setSource(QUrl("qrc:/recursos/audio/intro_menu_epico.wav"));
    sonidoMenu->setLoopCount(QSoundEffect::Infinite);
    sonidoMenu->setVolume(0.22f);

    sonidoGameOver = new QSoundEffect(this);
    sonidoGameOver->setSource(QUrl("qrc:/recursos/audio/game_over.wav"));
    sonidoGameOver->setVolume(0.62f);
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

void GameWidget::aplicarPersonajeSeleccionado()
{
    for (const auto& nivelJuego : niveles) {
        nivelJuego->configurarPersonaje(personajeSeleccionado);
    }
}

void GameWidget::iniciarPartida()
{
    aplicarDificultadSeleccionada();
    aplicarPersonajeSeleccionado();
    nivelActual = 0;
    estadoPantalla = PANTALLA_JUGANDO;
    mostrarAyuda = false;
    tiempoVictoria = 0.0f;
    victoriaProcesada = false;
    motivoGameOver = "";
    ultimoError = "";
    hayError = false;
    if (sonidoMenu != nullptr) {
        sonidoMenu->stop();
    }
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
    tiempoVictoria = 0.0f;
    victoriaProcesada = false;
    motivoGameOver = "";
    ultimoError = "";
    hayError = false;
    if (sonidoMenu != nullptr && !sonidoMenu->isPlaying()) {
        sonidoMenu->play();
    }
    if (sonidoFondo != nullptr) {
        sonidoFondo->stop();
    }
}

void GameWidget::activarGameOver()
{
    estadoPantalla = PANTALLA_GAME_OVER;
    tiempoGameOver = 0.0f;
    motivoGameOver = nivel()->motivoDerrota();
    if (motivoGameOver.trimmed().isEmpty()) {
        motivoGameOver = "No se cumplio el objetivo del nivel.";
    }

    if (sonidoFondo != nullptr) {
        sonidoFondo->stop();
    }
    if (sonidoMenu != nullptr) {
        sonidoMenu->stop();
    }
    if (sonidoGameOver != nullptr) {
        sonidoGameOver->play();
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
    else if (evento == SONIDO_GAME_OVER) {
        efecto = sonidoGameOver;
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

QPointF GameWidget::convertirAVirtual(const QPoint& posicion) const
{
    float escalaX = width() / ANCHO_BASE;
    float escalaY = height() / ALTO_BASE;
    float escala = std::min(escalaX, escalaY);
    float margenX = (width() - ANCHO_BASE * escala) / 2.0f;
    float margenY = (height() - ALTO_BASE * escala) / 2.0f;
    return QPointF((posicion.x() - margenX) / escala, (posicion.y() - margenY) / escala);
}

void GameWidget::dibujarIntro(QPainter& painter)
{
    QLinearGradient fondo(0, 0, 0, 720);
    fondo.setColorAt(0.0, QColor(4, 12, 28));
    fondo.setColorAt(0.48, QColor(9, 70, 102));
    fondo.setColorAt(1.0, QColor(3, 8, 18));
    painter.fillRect(QRectF(0, 0, ANCHO_BASE, ALTO_BASE), fondo);

    float avance = std::clamp(tiempoIntro / 4.2f, 0.0f, 1.0f);
    float barrido = 1280.0f * avance;

    painter.setPen(QPen(QColor(120, 230, 255, 58), 2));
    for (int i = 0; i < 12; ++i) {
        int y = 78 + i * 46;
        painter.drawLine(0, y, 1280, y + 18);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(42, 205, 238, 72));
    painter.drawRect(QRectF(1015, 80 - tiempoIntro * 18.0f, 24, 610));
    painter.setBrush(QColor(255, 225, 95, 200));
    for (int y = 130; y < 640; y += 72) {
        painter.drawRect(QRectF(955, y - tiempoIntro * 10.0f, 132, 8));
    }

    painter.setBrush(QColor(255, 225, 95));
    painter.drawRect(QRectF(150, 540, barrido * 0.46f, 5));
    painter.setBrush(QColor(42, 205, 238));
    painter.drawRect(QRectF(150, 553, barrido * 0.32f, 3));

    QFont titulo = painter.font();
    titulo.setPointSize(34);
    titulo.setBold(true);
    painter.setFont(titulo);
    painter.setPen(QColor(244, 252, 255));
    painter.drawText(QRectF(150, 250, 860, 64), Qt::AlignLeft | Qt::AlignVCenter, "CIUDAD ACADEMIA");

    QFont subtitulo = painter.font();
    subtitulo.setPointSize(15);
    subtitulo.setBold(false);
    painter.setFont(subtitulo);
    painter.setPen(QColor(195, 238, 248));
    painter.drawText(QRectF(154, 326, 720, 34), Qt::AlignLeft, "Torre de entrenamiento electromagnetico");

    painter.setPen(QColor(255, 225, 95));
    painter.drawText(QRectF(154, 590, 500, 28), Qt::AlignLeft, "Click o Enter para continuar");
}

void GameWidget::dibujarInicio(QPainter& painter)
{
    QLinearGradient fondo(0, 0, 0, 720);
    fondo.setColorAt(0.0, QColor(6, 20, 42));
    fondo.setColorAt(0.50, QColor(14, 92, 127));
    fondo.setColorAt(1.0, QColor(5, 12, 28));
    painter.fillRect(QRectF(0, 0, ANCHO_BASE, ALTO_BASE), fondo);

    QRadialGradient luz(920, 210, 620);
    luz.setColorAt(0.0, QColor(255, 223, 92, 52));
    luz.setColorAt(0.45, QColor(43, 194, 230, 38));
    luz.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter.fillRect(QRectF(0, 0, ANCHO_BASE, ALTO_BASE), luz);

    painter.setPen(QPen(QColor(195, 244, 255, 46), 2));
    for (int i = 0; i < 13; ++i) {
        int y = 64 + i * 46;
        painter.drawLine(0, y, 1280, y + (i % 2 == 0 ? 22 : -18));
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 215, 75));
    painter.drawRect(QRectF(96, 96, 8, 510));
    painter.setBrush(QColor(235, 252, 255, 28));
    painter.drawRect(QRectF(120, 96, 840, 2));
    painter.drawRect(QRectF(120, 606, 840, 2));

    QFont titulo = painter.font();
    titulo.setPointSize(38);
    titulo.setBold(true);
    painter.setFont(titulo);
    painter.setPen(QColor(235, 252, 255));
    painter.drawText(QRectF(136, 118, 850, 72), Qt::AlignLeft | Qt::AlignVCenter, "Clavados en Ciudad Academia");

    QFont subtitulo = painter.font();
    subtitulo.setPointSize(14);
    subtitulo.setBold(false);
    painter.setFont(subtitulo);
    painter.setPen(QColor(198, 236, 248));
    painter.drawText(QRectF(140, 206, 690, 52), Qt::AlignLeft | Qt::TextWordWrap,
                     "Salta desde una torre monumental, atraviesa anillos de control y busca una entrada limpia en la piscina de Ciudad Academia.");

    QString dificultad = dificultadSeleccionada == FACIL ? "Facil" : dificultadSeleccionada == NORMAL ? "Normal" : "Dificil";
    painter.setPen(QColor(255, 225, 95));
    painter.drawText(QRectF(140, 292, 360, 24), Qt::AlignLeft | Qt::AlignVCenter, "Dificultad: " + dificultad);

    struct OpcionDificultad {
        QRectF rect;
        QString texto;
        TipoDificultad tipo;
    };

    OpcionDificultad opciones[] = {
        { QRectF(140, 322, 120, 38), "FACIL", FACIL },
        { QRectF(274, 322, 132, 38), "NORMAL", NORMAL },
        { QRectF(420, 322, 132, 38), "DIFICIL", DIFICIL }
    };

    QFont opcion = painter.font();
    opcion.setPointSize(9);
    opcion.setBold(true);
    painter.setFont(opcion);
    for (const OpcionDificultad& item : opciones) {
        bool activo = dificultadSeleccionada == item.tipo;
        painter.setPen(QPen(activo ? QColor(255, 225, 95) : QColor(120, 230, 255, 110), 2));
        painter.setBrush(activo ? QColor(255, 225, 95, 45) : QColor(42, 205, 238, 20));
        painter.drawRect(item.rect);
        painter.setPen(activo ? QColor(255, 236, 150) : QColor(198, 236, 248));
        painter.drawText(item.rect, Qt::AlignCenter, item.texto);
    }

    struct OpcionPersonaje {
        QRectF rect;
        QString nombre;
        QString poder;
        TipoPersonaje tipo;
        QColor color;
    };

    OpcionPersonaje personajes[] = {
        { QRectF(620, 286, 214, 56), "Mikoto", "campo electromagnetico", PERSONAJE_MIKOTO, QColor(255, 225, 95) },
        { QRectF(850, 286, 214, 56), "Accelerator", "control vectorial", PERSONAJE_ACCELERATOR, QColor(230, 245, 255) },
        { QRectF(620, 358, 214, 56), "Mugino", "meltdowner lateral", PERSONAJE_MUGINO, QColor(90, 240, 120) },
        { QRectF(850, 358, 214, 56), "Dark Matter", "densidad del aire", PERSONAJE_DARK_MATTER, QColor(184, 116, 255) }
    };

    painter.setPen(QColor(255, 225, 95));
    painter.drawText(QRectF(620, 252, 444, 26), Qt::AlignLeft | Qt::AlignVCenter, "Personaje y poder fisico");

    for (const OpcionPersonaje& item : personajes) {
        bool activo = personajeSeleccionado == item.tipo;
        painter.setPen(QPen(activo ? item.color : QColor(120, 230, 255, 100), activo ? 3 : 1));
        painter.setBrush(activo ? QColor(item.color.red(), item.color.green(), item.color.blue(), 38) : QColor(4, 18, 32, 130));
        painter.drawRect(item.rect);
        painter.setPen(activo ? item.color : QColor(226, 244, 250));
        painter.drawText(QRectF(item.rect.x() + 14, item.rect.y() + 9, item.rect.width() - 24, 18), Qt::AlignLeft, item.nombre);
        painter.setPen(QColor(184, 222, 234));
        painter.drawText(QRectF(item.rect.x() + 14, item.rect.y() + 31, item.rect.width() - 24, 16), Qt::AlignLeft, item.poder);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 225, 95));
    painter.drawRect(QRectF(140, 432, 330, 58));
    painter.setBrush(QColor(255, 255, 255, 36));
    painter.drawRect(QRectF(470, 432, 80, 58));
    painter.setPen(QColor(20, 28, 36));
    QFont boton = painter.font();
    boton.setPointSize(14);
    boton.setBold(true);
    painter.setFont(boton);
    painter.drawText(QRectF(140, 432, 410, 58), Qt::AlignCenter, "ENTER  INICIAR");

    QFont ayuda = painter.font();
    ayuda.setPointSize(10);
    ayuda.setBold(false);
    painter.setFont(ayuda);
    painter.setPen(QColor(222, 245, 255));
    painter.drawText(QRectF(140, 524, 900, 24), Qt::AlignLeft, "1 Facil   2 Normal   3 Dificil   4 Mikoto   5 Accelerator   6 Mugino   7 Dark Matter");
    painter.drawText(QRectF(140, 554, 900, 24), Qt::AlignLeft, "Controles: WASD/Flechas, click poder fisico, R reiniciar, Esc pausa, F11 pantalla completa");

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(42, 205, 238, 42));
    painter.drawRect(QRectF(1106, 70, 26, 560));
    painter.setBrush(QColor(255, 225, 95, 185));
    for (int y = 128; y < 610; y += 72) {
        painter.drawRect(QRectF(1048, y, 132, 8));
    }
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

void GameWidget::dibujarGameOver(QPainter& painter)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 178));
    painter.drawRect(QRectF(0, 0, ANCHO_BASE, ALTO_BASE));

    QLinearGradient panel(0, 225, 0, 505);
    panel.setColorAt(0.0, QColor(8, 21, 38, 246));
    panel.setColorAt(1.0, QColor(18, 37, 55, 242));
    painter.setBrush(panel);
    painter.drawRoundedRect(QRectF(365, 210, 550, 280), 12, 12);

    painter.setBrush(QColor(230, 60, 60));
    painter.drawRect(QRectF(365, 210, 8, 280));
    painter.setBrush(QColor(255, 225, 95));
    painter.drawRect(QRectF(373, 210, 542, 5));

    QFont fuente = painter.font();
    fuente.setPointSize(34);
    fuente.setBold(true);
    painter.setFont(fuente);
    painter.setPen(QColor(255, 245, 235));
    painter.drawText(QRectF(390, 252, 500, 70), Qt::AlignCenter, "GAME OVER");

    fuente.setPointSize(12);
    fuente.setBold(false);
    painter.setFont(fuente);
    painter.setPen(QColor(214, 238, 246));
    painter.drawText(QRectF(410, 332, 460, 54), Qt::AlignCenter | Qt::TextWordWrap, motivoGameOver);
    painter.drawText(QRectF(410, 396, 460, 28), Qt::AlignCenter, "Regresando al menu...");

    float progreso = std::clamp(tiempoGameOver / 3.0f, 0.0f, 1.0f);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(70, 96, 116));
    painter.drawRect(QRectF(470, 438, 340, 10));
    painter.setBrush(QColor(255, 225, 95));
    painter.drawRect(QRectF(470, 438, 340 * progreso, 10));
}

void GameWidget::dibujarError(QPainter& painter)
{
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 175));
    painter.drawRect(QRectF(0, 0, ANCHO_BASE, ALTO_BASE));

    painter.setBrush(QColor(36, 12, 18, 242));
    painter.drawRoundedRect(QRectF(310, 230, 660, 210), 10, 10);
    painter.setBrush(QColor(255, 80, 80));
    painter.drawRect(QRectF(310, 230, 8, 210));

    QFont fuente = painter.font();
    fuente.setPointSize(18);
    fuente.setBold(true);
    painter.setFont(fuente);
    painter.setPen(QColor(255, 238, 238));
    painter.drawText(QRectF(340, 262, 600, 38), Qt::AlignLeft | Qt::AlignVCenter, "Error de ejecucion");

    fuente.setPointSize(11);
    fuente.setBold(false);
    painter.setFont(fuente);
    painter.setPen(QColor(245, 220, 220));
    painter.drawText(QRectF(340, 318, 590, 76), Qt::AlignLeft | Qt::TextWordWrap, ultimoError);
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
    painter.drawText(QRectF(435, 381, 410, 28), Qt::AlignCenter, "F11: pantalla completa");
}

void GameWidget::dibujarMarcoJuego(QPainter& painter)
{
    painter.setPen(Qt::NoPen);
    QLinearGradient lateral(960, 0, 1280, 0);
    lateral.setColorAt(0.0, QColor(4, 12, 24));
    lateral.setColorAt(1.0, QColor(10, 24, 38));
    painter.setBrush(lateral);
    painter.drawRect(QRectF(960, 0, 320, 720));
    painter.setBrush(QColor(255, 225, 95));
    painter.drawRect(QRectF(960, 0, 5, 720));
    painter.setBrush(QColor(41, 199, 232, 32));
    painter.drawRect(QRectF(986, 28, 248, 136));
    painter.setPen(QPen(QColor(115, 230, 255, 80), 1));
    painter.drawLine(986, 28, 1234, 28);
    painter.drawLine(986, 164, 1234, 164);

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
        painter.drawRect(QRectF(986, 188, 248, 42));
        painter.setPen(QColor(15, 25, 35));
        painter.drawText(QRectF(986, 188, 248, 42), Qt::AlignCenter, "Enter: siguiente nivel");
    }

    if (mostrarAyuda) {
        painter.setBrush(QColor(41, 199, 232, 26));
        painter.setPen(Qt::NoPen);
        painter.drawRect(QRectF(986, 480, 248, 150));
        painter.setPen(QPen(QColor(115, 230, 255, 70), 1));
        painter.drawLine(986, 480, 1234, 480);
        painter.setPen(Qt::white);
        painter.drawText(1010, 512, "Objetivo:");
        painter.drawText(1010, 538, "Superar ambos niveles.");
        painter.drawText(1010, 566, "1/2/3 dificultad.");
        painter.drawText(1010, 594, "Esc pausa. Enter avanza.");
    }
}

bool GameWidget::campaniaCompletada()
{
    return nivelActual == static_cast<int>(niveles.size()) - 1 && nivel()->estaSuperado();
}

void GameWidget::actualizar()
{
    try {
        if (estadoPantalla == PANTALLA_INTRO) {
            if (sonidoMenu != nullptr && !sonidoMenu->isPlaying()) {
                sonidoMenu->play();
            }
            tiempoIntro += 0.016f;
            if (tiempoIntro >= 4.6f) {
                estadoPantalla = PANTALLA_INICIO;
            }
            update();
            return;
        }

        if (estadoPantalla == PANTALLA_GAME_OVER) {
            tiempoGameOver += 0.016f;
            if (tiempoGameOver >= 3.0f) {
                reiniciarCampania();
            }
            update();
            return;
        }

        if (estadoPantalla != PANTALLA_JUGANDO) {
            if (estadoPantalla == PANTALLA_INICIO && sonidoMenu != nullptr && !sonidoMenu->isPlaying()) {
                sonidoMenu->play();
            }
            update();
            return;
        }

        nivel()->actualizar(0.016f);
        procesarSonidosNivel();
        if (campaniaCompletada()) {
            if (!victoriaProcesada) {
                reproducirEventoSonido(SONIDO_NIVEL);
                victoriaProcesada = true;
                tiempoVictoria = 0.0f;
            }
            tiempoVictoria += 0.016f;
            if (tiempoVictoria >= 4.0f) {
                reiniciarCampania();
            }
            update();
            return;
        }
        if (nivel()->estaPerdido()) {
            activarGameOver();
        }
    }
    catch (const JuegoException& error) {
        hayError = true;
        ultimoError = QString::fromUtf8(error.what());
        qWarning() << error.what();
        QApplication::beep();
        update();
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
        if (estadoPantalla == PANTALLA_INTRO) {
            dibujarIntro(painter);
            return;
        }

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

        if (estadoPantalla == PANTALLA_GAME_OVER) {
            dibujarGameOver(painter);
        }

        if (hayError) {
            dibujarError(painter);
        }
    }
    catch (const JuegoException& error) {
        hayError = true;
        ultimoError = QString::fromUtf8(error.what());
        qWarning() << error.what();
        painter.fillRect(QRectF(0, 0, ANCHO_BASE, ALTO_BASE), QColor(25, 25, 25));
        painter.setPen(Qt::white);
        painter.drawText(60, 80, "Error del juego:");
        painter.drawText(60, 110, error.what());
    }
}

void GameWidget::keyPressEvent(QKeyEvent* event)
{
    if (estadoPantalla == PANTALLA_INTRO) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter || event->key() == Qt::Key_Space) {
            estadoPantalla = PANTALLA_INICIO;
            update();
        }
        return;
    }

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

    if (estadoPantalla == PANTALLA_GAME_OVER) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter ||
            event->key() == Qt::Key_Space || event->key() == Qt::Key_M ||
            event->key() == Qt::Key_Escape) {
            reiniciarCampania();
            update();
        }
        return;
    }

    if (estadoPantalla == PANTALLA_INICIO &&
        (event->key() == Qt::Key_1 || event->key() == Qt::Key_2 || event->key() == Qt::Key_3)) {
        dificultadSeleccionada = event->key() == Qt::Key_1 ? FACIL : event->key() == Qt::Key_2 ? NORMAL : DIFICIL;
        aplicarDificultadSeleccionada();
        update();
        return;
    }

    if (estadoPantalla == PANTALLA_INICIO &&
        (event->key() == Qt::Key_4 || event->key() == Qt::Key_5 ||
         event->key() == Qt::Key_6 || event->key() == Qt::Key_7)) {
        personajeSeleccionado = event->key() == Qt::Key_4 ? PERSONAJE_MIKOTO :
                                event->key() == Qt::Key_5 ? PERSONAJE_ACCELERATOR :
                                event->key() == Qt::Key_6 ? PERSONAJE_MUGINO :
                                                            PERSONAJE_DARK_MATTER;
        aplicarPersonajeSeleccionado();
        update();
        return;
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

void GameWidget::mousePressEvent(QMouseEvent* event)
{
    QPointF virtualPos = convertirAVirtual(event->pos());

    if (estadoPantalla == PANTALLA_INTRO) {
        estadoPantalla = PANTALLA_INICIO;
        update();
        return;
    }

    if (estadoPantalla == PANTALLA_GAME_OVER) {
        reiniciarCampania();
        update();
        return;
    }

    if (estadoPantalla == PANTALLA_INICIO) {
        if (QRectF(140, 432, 410, 58).contains(virtualPos)) {
            iniciarPartida();
            return;
        }

        if (QRectF(140, 322, 120, 38).contains(virtualPos)) {
            dificultadSeleccionada = FACIL;
            aplicarDificultadSeleccionada();
            update();
            return;
        }
        if (QRectF(274, 322, 132, 38).contains(virtualPos)) {
            dificultadSeleccionada = NORMAL;
            aplicarDificultadSeleccionada();
            update();
            return;
        }
        if (QRectF(420, 322, 132, 38).contains(virtualPos)) {
            dificultadSeleccionada = DIFICIL;
            aplicarDificultadSeleccionada();
            update();
            return;
        }

        const QRectF areasPersonaje[] = {
            QRectF(620, 286, 214, 56),
            QRectF(850, 286, 214, 56),
            QRectF(620, 358, 214, 56),
            QRectF(850, 358, 214, 56)
        };
        const TipoPersonaje tipos[] = {
            PERSONAJE_MIKOTO,
            PERSONAJE_ACCELERATOR,
            PERSONAJE_MUGINO,
            PERSONAJE_DARK_MATTER
        };

        for (int i = 0; i < 4; ++i) {
            if (areasPersonaje[i].contains(virtualPos)) {
                personajeSeleccionado = tipos[i];
                aplicarPersonajeSeleccionado();
                update();
                return;
            }
        }
    }

    if (estadoPantalla == PANTALLA_JUGANDO) {
        nivel()->mousePresionado(QPointF(virtualPos.x() / ESCALA_MUNDO, virtualPos.y() / ESCALA_MUNDO));
    }
}
