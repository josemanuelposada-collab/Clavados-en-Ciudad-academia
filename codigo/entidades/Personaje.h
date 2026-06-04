#ifndef PERSONAJE_H
#define PERSONAJE_H

#include <QPainter>
#include <QColor>
#include <QRectF>
#include <QPixmap>
#include <QString>
#include <QStringList>
#include <QVector>
#include "Entidad.h"

enum TipoPersonaje
{
    PERSONAJE_MIKOTO,
    PERSONAJE_ACCELERATOR,
    PERSONAJE_MUGINO,
    PERSONAJE_DARK_MATTER
};

class Personaje : public Entidad
{
private:
    float vx;
    float vy;
    float ax;
    float ay;

    float masa;

    bool enAire;
    bool izquierda;
    bool derecha;
    bool impulsoActivo;

    TipoPersonaje tipo;
    float energia;
    float energiaMaxima;
    float controlLateral;
    float factorGravedad;
    float factorArrastre;
    float radioPoder;
    float atraccionMonedas;
    float tiempoAnimacion;
    QColor colorPoder;

    QVector<QPixmap> spritesIdle;
    QVector<QPixmap> spritesSalto;
    QVector<QPixmap> spritesCaida;
    QVector<QPixmap> spritesIzquierda;
    QVector<QPixmap> spritesDerecha;
    QVector<QPixmap> spritesImpulso;
    QVector<QPixmap> spritesSplash;

    void cargarSprites();
    void cargarSecuencia(QVector<QPixmap>& destino, const QString& carpeta, const QStringList& archivos);
    const QVector<QPixmap>& secuenciaActual() const;

public:
    Personaje();

    void actualizar(float dt) override;
    void dibujar(QPainter& painter) override;

    void saltar();
    void moverIzquierda(bool estado);
    void moverDerecha(bool estado);
    void activarImpulso(bool estado);

    void aplicarGravedad(float gravedad);
    void aplicarViento(float aceleracionViento);
    void aplicarFriccionAire(float factor);
    void aplicarImpulsoElectromagnetico(float dt);
    void aplicarLimites(float anchoPantalla);

    void detenerMovimiento();
    void colocarEn(float nuevoX, float nuevoY);
    void configurarTipo(TipoPersonaje nuevoTipo);

    float getX() const;
    float getY() const;
    float getVX() const;
    float getVY() const;
    float getAncho() const;
    float getAlto() const;
    float getEnergia() const;
    float getEnergiaMaxima() const;
    float getMasa() const;
    float getControlLateral() const;
    float getFactorGravedad() const;
    float getFactorArrastre() const;
    float getRadioPoder() const;
    float getAtraccionMonedas() const;
    QColor getColorPoder() const;
    QString getNombre() const;
    QString getPoder() const;
    TipoPersonaje getTipo() const;
    QRectF hitboxAjustada() const;

    void setVX(float nuevoVX);
    void setVY(float nuevoVY);
    void setEnAire(bool estado);
    void setEnergiaMaxima(float nuevaEnergia);

    bool estaEnAire() const;
    bool estaUsandoImpulso() const;
};

#endif
