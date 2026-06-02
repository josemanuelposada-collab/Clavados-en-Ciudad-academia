#ifndef MODELOSFISICOS_H
#define MODELOSFISICOS_H

#include <cmath>

class ModeloFisico
{
public:
    virtual ~ModeloFisico() {}
    virtual float calcular(float tiempo, float parametro) const = 0;
};

class ModeloOscilatorio : public ModeloFisico
{
private:
    float amplitud;
    float frecuencia;

public:
    ModeloOscilatorio(float amplitudInicial, float frecuenciaInicial);
    float calcular(float tiempo, float parametro) const override;
    void configurar(float nuevaAmplitud, float nuevaFrecuencia);
};

class ModeloCampoVariable : public ModeloFisico
{
private:
    float intensidadBase;
    float turbulencia;

public:
    ModeloCampoVariable(float intensidadInicial, float turbulenciaInicial);
    float calcular(float tiempo, float parametro) const override;
    void configurar(float nuevaIntensidad, float nuevaTurbulencia);
};

class ModeloImpulso : public ModeloFisico
{
private:
    float fuerza;
    float consumo;

public:
    ModeloImpulso(float fuerzaInicial, float consumoInicial);
    float calcular(float tiempo, float parametro) const override;
    float calcularConsumo(float dt) const;
};

#endif
