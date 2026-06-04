# Guia tecnica para sustentacion

Este archivo es una guia de estudio para explicar el proyecto. No reemplaza el informe final: el informe debe redactarse con palabras propias del equipo.

## Idea general

`Clavados en Ciudad Academia` es un videojuego 2D en C++ y Qt Widgets. El jugador controla un personaje de Ciudad Academia en pruebas de clavados dentro de una instalacion tecnologica con viento artificial, plataformas moviles, anillos, obstaculos y un dron supervisor.

## Arquitectura

- `gui/`: contiene `GameWidget`, encargado de pintar la escena, manejar teclado, pantalla de inicio, pausa, pantalla completa, audio y cambio de niveles.
- `logica/`: contiene reglas de juego, niveles, dificultad y excepciones.
- `entidades/`: contiene objetos del mundo: `Entidad`, `Personaje`, `Plataforma`, `Anillo`, `Moneda`, `Obstaculo` y `ProyectilDron`.
- `fisicas/`: contiene modelos fisicos parametrizables.
- `agente/`: contiene el dron supervisor.
- `render/`: contiene `SpriteCache`, usado para evitar reescalados repetidos de sprites.
- `recursos/`: contiene sprites y audios incluidos con `recursos.qrc`.

## POO y herencia propia

La herencia propia principal esta en `Entidad`. De ella heredan `Personaje`, `Plataforma`, `Anillo`, `Obstaculo` y `DronVigilante`. Esto permite compartir posicion, tamano, rectangulo de colision y centro geometrico.

Tambien existe `NivelJuego` como interfaz comun para los niveles. `NivelPiscinaEntrenamiento` y `NivelRutaAnillos` implementan esa interfaz, lo que permite que `GameWidget` los maneje mediante polimorfismo.

## Memoria dinamica y RAII

El proyecto usa memoria dinamica con propiedad explicita mediante `std::unique_ptr`. Esto conserva el uso de objetos creados en tiempo de ejecucion, pero evita fugas si ocurre una excepcion o si se reinicia un nivel.

- `GameWidget` almacena los niveles como `std::vector<std::unique_ptr<NivelJuego>>`.
- `NivelPiscinaEntrenamiento` posee `Personaje`, `Plataforma` y `DronVigilante` con `std::unique_ptr`.
- `NivelRutaAnillos` posee anillos, monedas, obstaculos y proyectiles con `std::vector<std::unique_ptr<...>>`.

Esta decision permite explicar memoria dinamica, polimorfismo y seguridad de recursos bajo el principio RAII.

## Contenedores

- `std::vector<std::unique_ptr<NivelJuego>>` en `GameWidget` para almacenar niveles polimorficos.
- `std::vector<std::unique_ptr<Anillo>>` y `std::vector<std::unique_ptr<Obstaculo>>` en el nivel de torre.
- `std::array<float, 10>` en el dron como memoria circular de errores recientes del jugador.
- `QSet<int>` para registrar teclas presionadas en el nivel de torre.
- `std::vector<std::unique_ptr<Moneda>>` para monedas metalicas atraibles por los poderes.
- `std::vector<std::unique_ptr<ProyectilDron>>` para disparos creados dinamicamente por el agente.

## Eficiencia

- Los sprites escalados se cachean con `SpriteCache` y `QPixmapCache`.
- La logica del agente no decide en cada frame: acumula tiempo y razona cada 0.18 segundos.
- Los niveles recorren colecciones lineales pequenas, por lo que las iteraciones principales son O(n) con n bajo.
- La memoria de aprendizaje del dron usa buffer circular y suma acumulada: insertar un error y recalcular presion es O(1), sin realocaciones.
- Las comparaciones de rango usan distancia al cuadrado cuando no se necesita la distancia real, evitando `sqrt` por entidad.
- Los proyectiles inactivos se eliminan con `erase/remove_if`, manteniendo pequeno el vector activo.
- El lienzo virtual se ajusto a 1280x720 y el mundo de juego se escala a 16:9 sin deformar los controles.
- Las colisiones usan hitboxes ajustadas, no el rectangulo visual completo del sprite, para reducir falsos positivos.

## Fisicas implementadas

1. Movimiento parabólico por gravedad: el personaje cambia velocidad vertical durante el salto o descenso.
2. Movimiento oscilatorio: la plataforma usa una funcion senoidal para moverse.
3. Viento lateral y turbulencia: zonas de viento modifican la aceleracion horizontal.
4. Campo de poder parametrizado: consume energia y aplica efectos segun personaje sin anular la gravedad.
5. Zona de velocidad variable: modifica la velocidad vertical durante la torre experimental.
6. Piscina movil con aceleracion: la posicion horizontal cambia por integracion de velocidad y aceleracion senoidal.
7. Colisiones elasticas e inelasticas: los proyectiles del dron alteran la velocidad del jugador usando momento lineal y coeficiente de restitucion.

### Ecuaciones de integracion directa

En el codigo se usa integracion por pasos de tiempo:

```text
v = v + a * dt
x = x + v * dt
```

`dt` es el tiempo entre frames. En el juego se actualiza aproximadamente cada 16 ms, por eso se usa un paso cercano a 0.016 s.

### Choque con momento lineal

Para una colision 1D entre dos cuerpos A y B se usa:

```text
vA' = ((mA - e*mB)*vA + (1 + e)*mB*vB) / (mA + mB)
vB' = ((mB - e*mA)*vB + (1 + e)*mA*vA) / (mA + mB)
```

Donde:

- `mA`, `mB`: masas.
- `vA`, `vB`: velocidades antes del choque.
- `vA'`, `vB'`: velocidades despues del choque.
- `e`: coeficiente de restitucion.

Si `e = 1`, el choque es elastico. Si `e` es menor que 1, el choque es inelastico y pierde parte de la energia cinetica. En el juego se aplica por separado en X y en Y para simular una colision 2D sencilla y sustentable.

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

## Personajes y poderes fisicos

El menu permite elegir entre cuatro personajes. Todos usan la misma clase `Personaje`, pero con parametros fisicos distintos:

- Mikoto: campo electromagnetico, mayor radio de atraccion de monedas metalicas.
- Accelerator: control vectorial, estabiliza velocidad horizontal y puede desviar obstaculos durante el poder.
- Mugino: Meltdowner lateral, mejora correcciones laterales durante el descenso.
- Dark Matter: densidad de aire, aumenta arrastre y suaviza la velocidad de caida.

Esto sirve para justificar abstraccion sin duplicar clases: cambia masa, gravedad efectiva, arrastre, control lateral, radio de poder y sprites.

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
- seleccion de personaje;
- pausa;
- ayuda;
- pantalla completa sobre lienzo virtual 1280x720;
- pantalla de campana completada;
- HUD por nivel;
- cambio entre niveles para demostracion.

## Recursos visuales nuevos

Se agregaron recursos de Ciudad Academia:

- fondo principal del campus deportivo;
- gradas;
- edificio de cristal;
- carriles de piscina;
- banderines;
- brillo de agua.

El fondo principal se genero con herramienta de imagen para el proyecto. Los sprites de detalle se generaron localmente de forma procedimental, sin recursos comerciales.

## Puntos fuertes para mencionar en el video

- Hay separacion por carpetas y responsabilidades.
- Hay dos niveles con dinamicas diferentes.
- La dificultad afecta varios parametros.
- El dron esta modelado como agente con cuatro componentes.
- La memoria dinamica se maneja con RAII y `unique_ptr`.
- El render evita reescalar sprites en cada frame mediante cache.
- Los sonidos se reproducen por eventos de juego, no como llamadas aisladas.
- El juego compila con Qt y genera ejecutable.
