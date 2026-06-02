#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include <QSoundEffect>
#include <memory>
#include <vector>
#include "../logica/NivelJuego.h"
#include "../logica/Dificultad.h"

enum EstadoPantalla
{
    PANTALLA_INICIO,
    PANTALLA_JUGANDO,
    PANTALLA_PAUSA
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
    QSoundEffect* sonidoFondo;
    QSoundEffect* sonidoSalto;
    QSoundEffect* sonidoAnillo;
    QSoundEffect* sonidoColision;
    QSoundEffect* sonidoAgua;
    QSoundEffect* sonidoNivel;

    NivelJuego* nivel();
    void cargarNiveles();
    void cargarSonidos();
    void avanzarNivel();
    void aplicarDificultadSeleccionada();
    void iniciarPartida();
    void reiniciarCampania();
    void reproducirEventoSonido(EventoSonidoJuego evento);
    void procesarSonidosNivel();
    void configurarLienzo(QPainter& painter);
    void dibujarInicio(QPainter& painter);
    void dibujarPausa(QPainter& painter);
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

private slots:
    void actualizar();
};

#endif
