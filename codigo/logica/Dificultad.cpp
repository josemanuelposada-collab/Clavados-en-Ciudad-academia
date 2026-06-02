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
        puntajeMinimo = 40;
        intentosMaximos = 5;
        energiaInicial = 120.0f;
    }
    else if (tipo == NORMAL) {
        intensidadViento = 45.0f;
        velocidadPlataforma = 2.0f;
        amplitudPlataforma = 45.0f;
        puntajeMinimo = 70;
        intentosMaximos = 4;
        energiaInicial = 100.0f;
    }
    else {
        intensidadViento = 75.0f;
        velocidadPlataforma = 2.9f;
        amplitudPlataforma = 68.0f;
        puntajeMinimo = 90;
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