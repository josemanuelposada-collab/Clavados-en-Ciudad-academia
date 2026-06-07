#include "GameWidget.h"
#include "../render/SpriteCache.h"
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QUrl>
#include <algorithm>
#include <array>
#include <exception>
#include "../logica/NivelPiscinaEntrenamiento.h"
#include "../logica/NivelRutaAnillos.h"
#include "../logica/JuegoException.h"

namespace {
const float ANCHO_BASE = 1280.0f;
const float ALTO_BASE = 720.0f;
const float ANCHO_MUNDO = 800.0f;
const float ALTO_MUNDO = 600.0f;
const float ESCALA_MUNDO = 1.2f;

struct InfoPersonaje
{
    TipoPersonaje tipo;
    QString nombre;
    QString poder;
    QString cualidades;
    QString descripcion;
    QString rutaSprite;
    QColor color;
};

const std::array<InfoPersonaje, 4>& infosPersonajes()
{
    static const std::array<InfoPersonaje, 4> infos = {{
        { PERSONAJE_MIKOTO, "Mikoto Misaka", "Campo electromagnetico",
          "Precisa, impulsiva y competitiva",
          "Mejor correccion de postura, salto fuerte y bonus por buen desempeno.",
          ":/recursos/sprites/personajes/mikoto_misaka/01_mikoto_misaka_salto_desde_altura_01_parado.png",
          QColor(255, 225, 95) },
        { PERSONAJE_ACCELERATOR, "Accelerator", "Control vectorial",
          "Calculador, estable y reactivo",
          "Atrae monedas metalicas en linea directa y estabiliza trayectorias laterales.",
          ":/recursos/sprites/personajes/accelerator/01_accelerator_salto_desde_altura_01_parado.png",
          QColor(230, 245, 255) },
        { PERSONAJE_MUGINO, "Mugino", "Meltdowner lateral",
          "Agresiva, veloz y precisa",
          "Desintegra bonus cercanos al activar Meltdowner y suma recompensa inmediata.",
          ":/recursos/sprites/personajes/mugino/01_mugino_salto_desde_altura_01_parado.png",
          QColor(100, 255, 130) },
        { PERSONAJE_DARK_MATTER, "Dark Matter", "Densidad del aire",
          "Creativo, tecnico y resistente",
          "Altera el aire cercano para orbitar monedas y suavizar la entrada.",
          ":/recursos/sprites/personajes/dark_matter/01_dark_matter_salto_desde_altura_01_parado.png",
          QColor(190, 130, 255) }
    }};
    return infos;
}

const InfoPersonaje& infoPersonaje(TipoPersonaje tipo)
{
    for (const InfoPersonaje& info : infosPersonajes()) {
        if (info.tipo == tipo) {
            return info;
        }
    }
    return infosPersonajes().front();
}

QRectF rectTarjetaPersonaje(int indice)
{
    return QRectF(596.0f + indice * 142.0f, 376.0f, 132.0f, 160.0f);
}
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
    cargarFondosMenu();
    if (sonidoMenu != nullptr) {
        sonidoMenu->play();
    }

    relojFrame.start();
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
        nivelJuego->cambiarDificultad(dificultadSeleccionada);
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

void GameWidget::cargarFondosMenu()
{
    fondoIntroPixel = SpriteCache::obtener(":/recursos/sprites/fondo_carga_ciudad_academia_pixel.png");
    fondoMenuPixel = SpriteCache::obtener(":/recursos/sprites/fondo_menu_ciudad_academia_pixel.png");
}

void GameWidget::dibujarFondoPixel(QPainter& painter, const QPixmap& fondo)
{
    if (fondo.isNull()) {
        QLinearGradient respaldo(0, 0, 0, ALTO_BASE);
        respaldo.setColorAt(0.0, QColor(4, 12, 28));
        respaldo.setColorAt(0.52, QColor(9, 70, 102));
        respaldo.setColorAt(1.0, QColor(3, 8, 18));
        painter.fillRect(QRectF(0, 0, ANCHO_BASE, ALTO_BASE), respaldo);
        return;
    }

    const float destinoAspecto = ANCHO_BASE / ALTO_BASE;
    const float fuenteAspecto = static_cast<float>(fondo.width()) / std::max(1, fondo.height());
    QRectF fuente(0, 0, fondo.width(), fondo.height());

    if (fuenteAspecto > destinoAspecto) {
        const float anchoRecorte = fondo.height() * destinoAspecto;
        fuente.setX((fondo.width() - anchoRecorte) * 0.5f);
        fuente.setWidth(anchoRecorte);
    }
    else {
        const float altoRecorte = fondo.width() / destinoAspecto;
        fuente.setY((fondo.height() - altoRecorte) * 0.5f);
        fuente.setHeight(altoRecorte);
    }

    painter.drawPixmap(QRectF(0, 0, ANCHO_BASE, ALTO_BASE), fondo, fuente);
}

void GameWidget::dibujarIntro(QPainter& painter)
{
    dibujarFondoPixel(painter, fondoIntroPixel);
    painter.fillRect(QRectF(0, 0, ANCHO_BASE, ALTO_BASE), QColor(4, 8, 22, 82));

    float avance = std::clamp(tiempoIntro / 4.2f, 0.0f, 1.0f);
    float barrido = 1280.0f * avance;

    painter.setPen(QPen(QColor(120, 230, 255, 58), 2));
    for (int i = 0; i < 12; ++i) {
        int y = 78 + i * 46;
        painter.drawLine(0, y, 1280, y + 18);
    }

    painter.setPen(Qt::NoPen);
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

void GameWidget::dibujarTarjetaPersonaje(QPainter& painter, TipoPersonaje tipo, const QRectF& rect, bool activo)
{
    const InfoPersonaje& info = infoPersonaje(tipo);
    painter.save();
    painter.setPen(QPen(activo ? info.color : QColor(120, 230, 255, 105), activo ? 3 : 1));
    painter.setBrush(activo ? QColor(info.color.red(), info.color.green(), info.color.blue(), 44) : QColor(4, 12, 28, 184));
    painter.drawRect(rect);

    const QPixmap& sprite = SpriteCache::obtener(info.rutaSprite);
    QRectF areaSprite(rect.x() + 36.0f, rect.y() + 14.0f, 60.0f, 72.0f);
    if (!sprite.isNull()) {
        painter.drawPixmap(areaSprite, sprite, QRectF(0, 0, sprite.width(), sprite.height()));
    }
    else {
        painter.setPen(info.color);
        painter.setBrush(QColor(info.color.red(), info.color.green(), info.color.blue(), 45));
        painter.drawRect(areaSprite);
        painter.drawText(areaSprite, Qt::AlignCenter, info.nombre.left(2).toUpper());
    }

    QFont fuente = painter.font();
    fuente.setPointSize(8);
    fuente.setBold(true);
    painter.setFont(fuente);
    painter.setPen(activo ? info.color : QColor(236, 248, 255));
    painter.drawText(QRectF(rect.x() + 8.0f, rect.y() + 92.0f, rect.width() - 16.0f, 18.0f),
                     Qt::AlignCenter, info.nombre);

    fuente.setPointSize(7);
    fuente.setBold(false);
    painter.setFont(fuente);
    painter.setPen(QColor(198, 226, 238));
    painter.drawText(QRectF(rect.x() + 8.0f, rect.y() + 112.0f, rect.width() - 16.0f, 40.0f),
                     Qt::AlignCenter | Qt::TextWordWrap, "Poder: " + info.poder);
    painter.restore();
}

void GameWidget::dibujarSelectorPersonajes(QPainter& painter)
{
    const InfoPersonaje& seleccionado = infoPersonaje(personajeSeleccionado);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(2, 8, 22, 188));
    painter.drawRect(QRectF(572, 112, 622, 462));
    painter.setBrush(QColor(255, 225, 95, 210));
    painter.drawRect(QRectF(572, 112, 6, 462));
    painter.setPen(QPen(QColor(120, 230, 255, 88), 1));
    painter.drawLine(QPointF(590, 112), QPointF(1194, 112));
    painter.drawLine(QPointF(590, 574), QPointF(1194, 574));

    QFont fuente = painter.font();
    fuente.setPointSize(12);
    fuente.setBold(true);
    painter.setFont(fuente);
    painter.setPen(QColor(255, 225, 95));
    painter.drawText(QRectF(600, 132, 560, 28), Qt::AlignLeft, "Seleccion de personaje");

    const QPixmap& sprite = SpriteCache::obtener(seleccionado.rutaSprite);
    QRectF preview(620, 178, 92, 126);
    if (!sprite.isNull()) {
        painter.drawPixmap(preview, sprite, QRectF(0, 0, sprite.width(), sprite.height()));
    }
    else {
        painter.setPen(seleccionado.color);
        painter.drawRect(preview);
    }

    fuente.setPointSize(17);
    fuente.setBold(true);
    painter.setFont(fuente);
    painter.setPen(QColor(244, 252, 255));
    painter.drawText(QRectF(735, 174, 390, 32), Qt::AlignLeft, seleccionado.nombre);

    fuente.setPointSize(10);
    fuente.setBold(false);
    painter.setFont(fuente);
    painter.setPen(seleccionado.color);
    painter.drawText(QRectF(735, 214, 390, 24), Qt::AlignLeft, "Poder: " + seleccionado.poder);
    painter.setPen(QColor(210, 235, 244));
    painter.drawText(QRectF(735, 244, 390, 24), Qt::AlignLeft, "Cualidades: " + seleccionado.cualidades);
    painter.drawText(QRectF(735, 276, 390, 58), Qt::AlignLeft | Qt::TextWordWrap, "Efecto: " + seleccionado.descripcion);

    int indice = 0;
    for (const InfoPersonaje& info : infosPersonajes()) {
        dibujarTarjetaPersonaje(painter, info.tipo, rectTarjetaPersonaje(indice), info.tipo == personajeSeleccionado);
        ++indice;
    }
}

void GameWidget::dibujarInicio(QPainter& painter)
{
    dibujarFondoPixel(painter, fondoMenuPixel);
    painter.fillRect(QRectF(0, 0, ANCHO_BASE, ALTO_BASE), QColor(2, 7, 24, 104));

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
    painter.setBrush(QColor(2, 8, 22, 190));
    painter.drawRect(QRectF(94, 86, 452, 520));
    painter.setBrush(QColor(255, 215, 75));
    painter.drawRect(QRectF(96, 96, 8, 510));
    painter.setBrush(QColor(235, 252, 255, 28));
    painter.drawRect(QRectF(120, 96, 840, 2));
    painter.drawRect(QRectF(120, 606, 840, 2));

    QFont titulo = painter.font();
    titulo.setPointSize(30);
    titulo.setBold(true);
    painter.setFont(titulo);
    painter.setPen(QColor(235, 252, 255));
    painter.drawText(QRectF(136, 112, 380, 92), Qt::AlignLeft | Qt::AlignVCenter | Qt::TextWordWrap,
                     "Clavados en Ciudad Academia");

    QFont subtitulo = painter.font();
    subtitulo.setPointSize(11);
    subtitulo.setBold(false);
    painter.setFont(subtitulo);
    painter.setPen(QColor(198, 236, 248));
    painter.drawText(QRectF(140, 214, 365, 86), Qt::AlignLeft | Qt::TextWordWrap,
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

    dibujarSelectorPersonajes(painter);

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
    painter.drawText(QRectF(140, 524, 390, 24), Qt::AlignLeft, "1 Facil  2 Normal  3 Dificil");
    painter.drawText(QRectF(140, 554, 390, 24), Qt::AlignLeft, "4-7 personaje  |  F11 pantalla completa");

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
        float dt = std::clamp(relojFrame.restart() / 1000.0f, 0.001f, 0.033f);

        if (estadoPantalla == PANTALLA_INTRO) {
            if (sonidoMenu != nullptr && !sonidoMenu->isPlaying()) {
                sonidoMenu->play();
            }
            tiempoIntro += dt;
            if (tiempoIntro >= 4.6f) {
                estadoPantalla = PANTALLA_INICIO;
            }
            update();
            return;
        }

        if (estadoPantalla == PANTALLA_GAME_OVER) {
            tiempoGameOver += dt;
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

        nivel()->actualizar(dt);
        procesarSonidosNivel();
        if (campaniaCompletada()) {
            if (!victoriaProcesada) {
                reproducirEventoSonido(SONIDO_NIVEL);
                victoriaProcesada = true;
                tiempoVictoria = 0.0f;
            }
            tiempoVictoria += dt;
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
    catch (const std::exception& error) {
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
    catch (const std::exception& error) {
        hayError = true;
        ultimoError = QString::fromUtf8(error.what());
        qWarning() << error.what();
        painter.fillRect(QRectF(0, 0, ANCHO_BASE, ALTO_BASE), QColor(25, 25, 25));
        painter.setPen(Qt::white);
        painter.drawText(60, 80, "Error estandar:");
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

        const TipoPersonaje tipos[] = {
            PERSONAJE_MIKOTO,
            PERSONAJE_ACCELERATOR,
            PERSONAJE_MUGINO,
            PERSONAJE_DARK_MATTER
        };

        for (int i = 0; i < 4; ++i) {
            if (rectTarjetaPersonaje(i).contains(virtualPos)) {
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
