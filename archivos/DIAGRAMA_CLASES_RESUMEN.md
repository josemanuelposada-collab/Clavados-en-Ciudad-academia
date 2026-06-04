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
        -TipoPersonaje tipo
        -float factorGravedad
        -float factorArrastre
        +saltar()
        +configurarTipo(TipoPersonaje)
        +hitboxAjustada() QRectF
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
    class SpriteCache {
        +dibujarAjustado(QPainter, QPixmap, QRect, QString)
    }
    class EventoSonidoJuego
    class TipoPersonaje
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
    GameWidget ..> SpriteCache
    GameWidget ..> EventoSonidoJuego
    GameWidget ..> TipoPersonaje
    NivelJuego ..> EventoSonidoJuego
    NivelJuego ..> TipoPersonaje
    NivelPiscinaEntrenamiento o-- Personaje
    NivelPiscinaEntrenamiento o-- Plataforma
    NivelPiscinaEntrenamiento o-- DronVigilante
    NivelPiscinaEntrenamiento o-- Dificultad
    NivelRutaAnillos o-- Personaje
    NivelRutaAnillos o-- DronVigilante
    NivelRutaAnillos o-- Anillo
    NivelRutaAnillos o-- Obstaculo
    NivelRutaAnillos o-- Dificultad
    Personaje ..> SpriteCache
    Plataforma ..> SpriteCache
    Anillo ..> SpriteCache
    Obstaculo ..> SpriteCache
    DronVigilante ..> SpriteCache
```

## Puntos para sustentar

- Herencia propia: `Entidad` es la base de personajes, plataforma, anillos, obstaculos y dron. `NivelJuego` permite manejar niveles distintos con polimorfismo.
- Memoria dinamica: los niveles y entidades usan `std::unique_ptr`, manteniendo memoria dinamica con propiedad clara y liberacion automatica.
- Contenedores: `NivelRutaAnillos` usa `std::vector` para anillos y obstaculos; `DronVigilante` usa `QVector` como memoria de aprendizaje.
- Eficiencia: `SpriteCache` evita reescalados repetidos de pixmaps durante el render.
- Fisicas: gravedad/parabola, viento/turbulencia, friccion, poderes parametrizados, piscina con aceleracion y oscilacion senoidal.
- Agente inteligente: el dron separa percepcion, razonamiento, accion y aprendizaje.
- Sonido: cada nivel emite eventos (`salto`, `anillo`, `colision`, `agua`, `nivel`) y `GameWidget` los reproduce con `QSoundEffect`.
