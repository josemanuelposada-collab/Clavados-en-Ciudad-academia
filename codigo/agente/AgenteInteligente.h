#ifndef AGENTEINTELIGENTE_H
#define AGENTEINTELIGENTE_H

class AgenteInteligente
{
public:
    virtual ~AgenteInteligente() = default;

    virtual void registrarResultado(float errorEntrada, float puntaje, bool usoPoder) = 0;
    virtual void registrarImpactoJugador() = 0;
    virtual void registrarEvasionJugador() = 0;
    virtual void registrarBonusRecolectado() = 0;
    virtual float obtenerPresionAdaptativa() const = 0;
};

#endif
