# Diagrama de clases resumido

```mermaid
classDiagram
    class Entidad {
        <<abstract>>
        #float x
        #float y
        #float ancho
        #float alto
        +actualizar(float dt)*
        +dibujar(QPainter painter)*
        +rect() QRectF
        +centro() QPointF
    }

    class Personaje {
        -float vx
        -float vy
        -float energia
        +saltar()
        +aplicarGravedad(float)
        +aplicarViento(float)
        +aplicarImpulsoElectromagnetico(float)
    }

    class Plataforma {
        -ModeloOscilatorio oscilacion
        +configurarOscilacion(float, float)
    }

    class Obstaculo {
        -TipoObstaculo tipo
        +rebotarHorizontal(float, float)
    }

    class Anillo {
        -bool recolectado
        +recolectar()
    }

    class DronVigilante {
        -EstadoDron estado
        -QVector~float~ memoriaErrores
        +percibir(Personaje) PercepcionDron
        +razonar(PercepcionDron) EstadoDron
        +actuar(float, Personaje)
        +aprender(float)
    }

    class NivelJuego {
        <<abstract>>
        +actualizar(float dt)*
        +dibujar(QPainter painter)*
        +teclaPresionada(int)*
        +reiniciarNivel()*
        +consumirEventosSonido() QVector
    }

    class NivelPiscinaEntrenamiento
    class NivelRutaAnillos
    class GameWidget
    class EventoSonidoJuego
    class Dificultad
    class ModeloFisico
    class ModeloOscilatorio
    class ModeloCampoVariable
    class ModeloImpulso

    Entidad <|-- Personaje
    Entidad <|-- Plataforma
    Entidad <|-- Obstaculo
    Entidad <|-- Anillo
    Entidad <|-- DronVigilante
    NivelJuego <|-- NivelPiscinaEntrenamiento
    NivelJuego <|-- NivelRutaAnillos
    ModeloFisico <|-- ModeloOscilatorio
    ModeloFisico <|-- ModeloCampoVariable
    ModeloFisico <|-- ModeloImpulso

    GameWidget o-- NivelJuego
    GameWidget ..> EventoSonidoJuego
    NivelJuego ..> EventoSonidoJuego
    NivelPiscinaEntrenamiento o-- Personaje
    NivelPiscinaEntrenamiento o-- Plataforma
    NivelPiscinaEntrenamiento o-- DronVigilante
    NivelPiscinaEntrenamiento o-- Dificultad
    NivelRutaAnillos o-- Personaje
    NivelRutaAnillos o-- DronVigilante
    NivelRutaAnillos o-- Anillo
    NivelRutaAnillos o-- Obstaculo
    NivelRutaAnillos o-- Dificultad
```

## Puntos para sustentar

- Herencia propia: `Entidad` es la base de personajes, plataforma, anillos, obstaculos y dron. `NivelJuego` permite manejar niveles distintos con polimorfismo.
- Memoria dinamica: los niveles crean objetos con `new` y los liberan en destructores o en `liberarEntidades`.
- Contenedores: `NivelRutaAnillos` usa `std::vector` para anillos y obstaculos; `DronVigilante` usa `QVector` como memoria de aprendizaje.
- Fisicas: gravedad/parabola, viento/turbulencia, friccion, impulso electromagnetico y oscilacion senoidal.
- Agente inteligente: el dron separa percepcion, razonamiento, accion y aprendizaje.
- Sonido: cada nivel emite eventos (`salto`, `anillo`, `colision`, `agua`, `nivel`) y `GameWidget` los reproduce con `QSoundEffect`.
