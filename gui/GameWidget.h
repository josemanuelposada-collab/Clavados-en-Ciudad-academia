#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include "../logica/NivelPiscinaEntrenamiento.h"

class GameWidget : public QWidget
{
    Q_OBJECT

private:
    QTimer* timer;
    NivelPiscinaEntrenamiento* nivel;

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