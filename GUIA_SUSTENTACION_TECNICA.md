# Guia tecnica para sustentacion

Este archivo es una guia de estudio para explicar el proyecto. No reemplaza el informe final: el informe debe redactarse con palabras propias del equipo.

## Idea general

`Clavados en Ciudad Academia` es un videojuego 2D en C++ y Qt Widgets. El jugador controla a Mikoto en pruebas de clavados dentro de una instalacion tecnologica con viento artificial, plataformas moviles, anillos, obstaculos y un dron supervisor.

## Arquitectura

- `gui/`: contiene `GameWidget`, encargado de pintar la escena, manejar teclado, pantalla de inicio, pausa, pantalla completa, audio y cambio de niveles.
- `logica/`: contiene reglas de juego, niveles, dificultad y excepciones.
- `entidades/`: contiene objetos del mundo: `Entidad`, `Personaje`, `Plataforma`, `Anillo` y `Obstaculo`.
- `fisicas/`: contiene modelos fisicos parametrizables.
- `agente/`: contiene el dron supervisor.
- `recursos/`: contiene sprites y audios incluidos con `recursos.qrc`.

## POO y herencia propia

La herencia propia principal esta en `Entidad`. De ella heredan `Personaje`, `Plataforma`, `Anillo`, `Obstaculo` y `DronVigilante`. Esto permite compartir posicion, tamano, rectangulo de colision y centro geometrico.

Tambien existe `NivelJuego` como interfaz comun para los niveles. `NivelPiscinaEntrenamiento` y `NivelRutaAnillos` implementan esa interfaz, lo que permite que `GameWidget` los maneje mediante polimorfismo.

## Memoria dinamica

El proyecto usa memoria dinamica con punteros para niveles, entidades y componentes del juego. Ejemplos:

- `GameWidget` crea niveles y los libera en su destructor.
- `NivelPiscinaEntrenamiento` crea `Personaje`, `Plataforma` y `DronVigilante`.
- `NivelRutaAnillos` crea anillos y obstaculos con `new` y los libera en `liberarEntidades`.

Esta decision permite trabajar con polimorfismo y colecciones de objetos creados durante la ejecucion.

## Contenedores

- `QVector<NivelJuego*>` en `GameWidget` para almacenar niveles.
- `std::vector<Anillo*>` y `std::vector<Obstaculo*>` en el nivel de torre.
- `QVector<float>` en el dron para recordar errores recientes del jugador.
- `QSet<int>` para registrar teclas presionadas en el nivel de torre.

## Fisicas implementadas

1. Movimiento parabólico por gravedad: el personaje cambia velocidad vertical durante el salto o descenso.
2. Movimiento oscilatorio: la plataforma usa una funcion senoidal para moverse.
3. Viento lateral y turbulencia: zonas de viento modifican la aceleracion horizontal.
4. Impulso electromagnetico: consume energia y corrige la trayectoria.
5. Zona de velocidad variable: modifica la velocidad vertical durante la torre experimental.

El movimiento rectilineo simple no se usa como modelo principal de fisica evaluable.

## Dificultad

La clase `Dificultad` no solo recorta tiempo. Tambien modifica:

- intensidad del viento;
- velocidad de plataforma;
- amplitud de plataforma;
- energia disponible;
- puntaje minimo;
- intentos;
- tiempo y presion del nivel de torre.

Esto permite justificar una abstraccion real de dificultad.

## Agente inteligente

El agente es `DronVigilante`.

- Percepcion: mide distancia al jugador, velocidad e impulso activo.
- Razonamiento: decide entre patrulla, escaneo e intercepcion.
- Accion: se mueve, presiona la trayectoria y puede causar colision.
- Aprendizaje: guarda errores recientes y aumenta presion si el jugador mejora.

No es inteligencia avanzada, pero cumple la estructura de agente simple pedida por el curso.

## Sonido

El proyecto usa `QSoundEffect` y audios WAV generados localmente, sin musica comercial:

- fondo de entrenamiento;
- salto/impulso;
- anillo bonus;
- colision;
- entrada al agua;
- nivel superado.

Los niveles emiten eventos de sonido mediante `EventoSonidoJuego`, y `GameWidget` los consume y reproduce.

## Excepciones

`JuegoException` se usa cuando el juego no puede crear entidades necesarias o no hay niveles cargados. Esto permite manejar fallos de inicializacion sin cerrar silenciosamente.

## GUI y experiencia

`GameWidget` incluye:

- pantalla de inicio;
- seleccion de dificultad;
- pausa;
- ayuda;
- pantalla completa con escalado proporcional;
- pantalla de campana completada;
- HUD por nivel;
- cambio entre niveles para demostracion.

## Puntos fuertes para mencionar en el video

- Hay separacion por carpetas y responsabilidades.
- Hay dos niveles con dinamicas diferentes.
- La dificultad afecta varios parametros.
- El dron esta modelado como agente con cuatro componentes.
- Los sonidos se reproducen por eventos de juego, no como llamadas aisladas.
- El juego compila con Qt y genera ejecutable.
