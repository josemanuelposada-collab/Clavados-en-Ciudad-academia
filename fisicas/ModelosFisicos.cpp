#include "ModelosFisicos.h"

ModeloOscilatorio::ModeloOscilatorio(float amplitudInicial, float frecuenciaInicial)
    : amplitud(amplitudInicial),
      frecuencia(frecuenciaInicial)
{
}

float ModeloOscilatorio::calcular(float tiempo, float parametro) const
{
    return parametro + amplitud * std::sin(frecuencia * tiempo);
}

void ModeloOscilatorio::configurar(float nuevaAmplitud, float nuevaFrecuencia)
{
    amplitud = nuevaAmplitud;
    frecuencia = nuevaFrecuencia;
}

ModeloCampoVariable::ModeloCampoVariable(float intensidadInicial, float turbulenciaInicial)
    : intensidadBase(intensidadInicial),
      turbulencia(turbulenciaInicial)
{
}

float ModeloCampoVariable::calcular(float tiempo, float parametro) const
{
    return intensidadBase + turbulencia * std::sin(tiempo * 3.1f + parametro * 0.013f);
}

void ModeloCampoVariable::configurar(float nuevaIntensidad, float nuevaTurbulencia)
{
    intensidadBase = nuevaIntensidad;
    turbulencia = nuevaTurbulencia;
}

ModeloImpulso::ModeloImpulso(float fuerzaInicial, float consumoInicial)
    : fuerza(fuerzaInicial),
      consumo(consumoInicial)
{
}

float ModeloImpulso::calcular(float tiempo, float parametro) const
{
    (void)tiempo;
    (void)parametro;
    return fuerza;
}

float ModeloImpulso::calcularConsumo(float dt) const
{
    return consumo * dt;
}
