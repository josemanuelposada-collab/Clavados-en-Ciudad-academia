#ifndef DIFICULTAD_H
#define DIFICULTAD_H

#include <QString>

enum TipoDificultad
{
    FACIL,
    NORMAL,
    DIFICIL
};

class Dificultad
{
private:
    TipoDificultad tipo;

    float intensidadViento;
    float velocidadPlataforma;
    float amplitudPlataforma;
    float factorGravedad;
    float factorPiscinaMovil;
    float factorRafagas;
    float factorAgente;
    int puntajeMinimo;
    int intentosMaximos;
    float energiaInicial;

public:
    Dificultad();
    Dificultad(TipoDificultad tipo);

    void configurar(TipoDificultad nuevoTipo);

    TipoDificultad getTipo() const;
    float getIntensidadViento() const;
    float getVelocidadPlataforma() const;
    float getAmplitudPlataforma() const;
    float getFactorGravedad() const;
    float getFactorPiscinaMovil() const;
    float getFactorRafagas() const;
    float getFactorAgente() const;
    int getPuntajeMinimo() const;
    int getIntentosMaximos() const;
    float getEnergiaInicial() const;

    QString getNombre() const;
};

#endif
