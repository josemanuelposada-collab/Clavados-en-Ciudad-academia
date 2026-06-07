#ifndef MODELOSFISICOS_H
#define MODELOSFISICOS_H

#include <cmath>
#include <algorithm>

namespace FisicaJuego
{
    struct ResultadoColision1D
    {
        float velocidadA;
        float velocidadB;
        float impulso;
    };

    inline float distanciaCuadrada(float dx, float dy)
    {
        return dx * dx + dy * dy;
    }

    inline float rapidezCuadrada(float vx, float vy)
    {
        return vx * vx + vy * vy;
    }

    inline void integrarVelocidad(float& velocidad, float aceleracion, float dt)
    {
        velocidad += aceleracion * dt;
    }

    inline void integrarPosicion(float& posicion, float velocidad, float dt)
    {
        posicion += velocidad * dt;
    }

    inline void integrarMovimientoAcelerado(float& posicion, float& velocidad, float aceleracion, float dt)
    {
        posicion += velocidad * dt + 0.5f * aceleracion * dt * dt;
        velocidad += aceleracion * dt;
    }

    inline float aplicarArrastre(float velocidad, float coeficiente, float dt)
    {
        return velocidad * std::clamp(1.0f - coeficiente * dt, 0.0f, 1.0f);
    }

    inline void limitarVector(float& vx, float& vy, float rapidezMaxima)
    {
        float rapidez2 = rapidezCuadrada(vx, vy);
        float maxima2 = rapidezMaxima * rapidezMaxima;
        if (rapidez2 > maxima2 && rapidez2 > 0.0f) {
            float escala = rapidezMaxima / std::sqrt(rapidez2);
            vx *= escala;
            vy *= escala;
        }
    }

    inline ResultadoColision1D resolverChoqueMomentoLineal(float masaA, float velocidadA,
                                                           float masaB, float velocidadB,
                                                           float restitucion)
    {
        restitucion = std::clamp(restitucion, 0.0f, 1.0f);
        float totalMasas = masaA + masaB;
        if (totalMasas <= 0.0f) {
            return {velocidadA, velocidadB, 0.0f};
        }

        float nuevaVelocidadA = ((masaA - restitucion * masaB) * velocidadA +
                                 (1.0f + restitucion) * masaB * velocidadB) / totalMasas;
        float nuevaVelocidadB = ((masaB - restitucion * masaA) * velocidadB +
                                 (1.0f + restitucion) * masaA * velocidadA) / totalMasas;
        float impulso = masaA * (nuevaVelocidadA - velocidadA);

        return {nuevaVelocidadA, nuevaVelocidadB, impulso};
    }

    inline ResultadoColision1D resolverChoqueElastico(float masaA, float velocidadA,
                                                      float masaB, float velocidadB)
    {
        return resolverChoqueMomentoLineal(masaA, velocidadA, masaB, velocidadB, 1.0f);
    }

    inline ResultadoColision1D resolverChoqueInelastico(float masaA, float velocidadA,
                                                        float masaB, float velocidadB)
    {
        return resolverChoqueMomentoLineal(masaA, velocidadA, masaB, velocidadB, 0.28f);
    }
}

class ModeloFisico
{
public:
    virtual ~ModeloFisico() = default;
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
