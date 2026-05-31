#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include <QVector>
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
    QVector<NivelJuego*> niveles;
    int nivelActual;
    bool mostrarAyuda;
    EstadoPantalla estadoPantalla;
    TipoDificultad dificultadSeleccionada;

    NivelJuego* nivel();
    void cargarNiveles();
    void avanzarNivel();
    void aplicarDificultadSeleccionada();
    void iniciarPartida();
    void reiniciarCampania();
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
