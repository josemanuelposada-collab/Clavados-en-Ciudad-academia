#include "GameWidget.h"
#include <QPainter>

GameWidget::GameWidget(QWidget* parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);

    nivel = new NivelPiscinaEntrenamiento();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GameWidget::actualizar);
    timer->start(16);
}

GameWidget::~GameWidget()
{
    delete nivel;
}

void GameWidget::actualizar()
{
    nivel->actualizar(0.016f);
    update();
}

void GameWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    nivel->dibujar(painter);
}

void GameWidget::keyPressEvent(QKeyEvent* event)
{
    nivel->teclaPresionada(event->key());
}

void GameWidget::keyReleaseEvent(QKeyEvent* event)
{
    nivel->teclaLiberada(event->key());
}