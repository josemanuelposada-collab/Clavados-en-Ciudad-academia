#include "Dificultad.h"

Dificultad::Dificultad()
{
    configurar(NORMAL);
}

Dificultad::Dificultad(TipoDificultad tipo)
{
    configurar(tipo);
}

void Dificultad::configurar(TipoDificultad nuevoTipo)
{
    tipo = nuevoTipo;

    if (tipo == FACIL) {
        intensidadViento = 22.0f;
        velocidadPlataforma = 1.2f;
        amplitudPlataforma = 28.0f;
        factorGravedad = 0.92f;
        factorPiscinaMovil = 0.72f;
        factorRafagas = 0.70f;
        factorAgente = 0.78f;
        puntajeMinimo = 35;
        intentosMaximos = 5;
        energiaInicial = 120.0f;
    }
    else if (tipo == NORMAL) {
        intensidadViento = 45.0f;
        velocidadPlataforma = 2.0f;
        amplitudPlataforma = 45.0f;
        factorGravedad = 1.0f;
        factorPiscinaMovil = 1.0f;
        factorRafagas = 1.0f;
        factorAgente = 1.0f;
        puntajeMinimo = 50;
        intentosMaximos = 4;
        energiaInicial = 100.0f;
    }
    else {
        intensidadViento = 75.0f;
        velocidadPlataforma = 2.9f;
        amplitudPlataforma = 68.0f;
        factorGravedad = 1.10f;
        factorPiscinaMovil = 1.30f;
        factorRafagas = 1.28f;
        factorAgente = 1.22f;
        puntajeMinimo = 65;
        intentosMaximos = 3;
        energiaInicial = 80.0f;
    }
}

TipoDificultad Dificultad::getTipo() const
{
    return tipo;
}

float Dificultad::getIntensidadViento() const
{
    return intensidadViento;
}

float Dificultad::getVelocidadPlataforma() const
{
    return velocidadPlataforma;
}

float Dificultad::getAmplitudPlataforma() const
{
    return amplitudPlataforma;
}

float Dificultad::getFactorGravedad() const
{
    return factorGravedad;
}

float Dificultad::getFactorPiscinaMovil() const
{
    return factorPiscinaMovil;
}

float Dificultad::getFactorRafagas() const
{
    return factorRafagas;
}

float Dificultad::getFactorAgente() const
{
    return factorAgente;
}

int Dificultad::getPuntajeMinimo() const
{
    return puntajeMinimo;
}

int Dificultad::getIntentosMaximos() const
{
    return intentosMaximos;
}

float Dificultad::getEnergiaInicial() const
{
    return energiaInicial;
}

QString Dificultad::getNombre() const
{
    if (tipo == FACIL) {
        return "Facil";
    }

    if (tipo == NORMAL) {
        return "Normal";
    }

    return "Dificil";
}
