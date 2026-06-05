#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QSoundEffect>
#include <memory>
#include <vector>
#include "../logica/NivelJuego.h"
#include "../logica/Dificultad.h"

enum EstadoPantalla
{
    PANTALLA_INTRO,
    PANTALLA_INICIO,
    PANTALLA_JUGANDO,
    PANTALLA_PAUSA,
    PANTALLA_GAME_OVER
};

class GameWidget : public QWidget
{
    Q_OBJECT

private:
    QTimer* timer;
    std::vector<std::unique_ptr<NivelJuego>> niveles;
    int nivelActual;
    bool mostrarAyuda;
    EstadoPantalla estadoPantalla;
    TipoDificultad dificultadSeleccionada;
    TipoPersonaje personajeSeleccionado;
    QSoundEffect* sonidoFondo;
    QSoundEffect* sonidoSalto;
    QSoundEffect* sonidoAnillo;
    QSoundEffect* sonidoColision;
    QSoundEffect* sonidoAgua;
    QSoundEffect* sonidoNivel;
    QSoundEffect* sonidoMenu;
    QSoundEffect* sonidoGameOver;
    float tiempoIntro;
    float tiempoGameOver;
    float tiempoVictoria;
    bool victoriaProcesada;

    NivelJuego* nivel();
    void cargarNiveles();
    void cargarSonidos();
    void avanzarNivel();
    void aplicarDificultadSeleccionada();
    void aplicarPersonajeSeleccionado();
    void iniciarPartida();
    void reiniciarCampania();
    void activarGameOver();
    void reproducirEventoSonido(EventoSonidoJuego evento);
    void procesarSonidosNivel();
    void configurarLienzo(QPainter& painter);
    QPointF convertirAVirtual(const QPoint& posicion) const;
    void dibujarIntro(QPainter& painter);
    void dibujarInicio(QPainter& painter);
    void dibujarPausa(QPainter& painter);
    void dibujarGameOver(QPainter& painter);
    void dibujarMarcoJuego(QPainter& painter);
    void dibujarCampaniaCompletada(QPainter& painter);
    bool campaniaCompletada();

public:
    explicit GameWidget(QWidget* parent = nullptr);
    ~GameWidget();

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void actualizar();
};

#endif
