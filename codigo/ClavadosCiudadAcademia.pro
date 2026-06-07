QT += widgets multimedia

CONFIG += c++17

SOURCES += \
    agente/DronVigilante.cpp \
    entidades/Anillo.cpp \
    entidades/Entidad.cpp \
    entidades/Moneda.cpp \
    entidades/Obstaculo.cpp \
    entidades/Personaje.cpp \
    entidades/Plataforma.cpp \
    entidades/ProyectilDron.cpp \
    fisicas/ModelosFisicos.cpp \
    gui/GameWidget.cpp \
    render/SpriteCache.cpp \
    logica/Dificultad.cpp \
    logica/NivelRutaAnillos.cpp \
    logica/NivelPiscinaEntrenamiento.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    agente/AgenteInteligente.h \
    agente/DronVigilante.h \
    entidades/Anillo.h \
    entidades/Entidad.h \
    entidades/Moneda.h \
    entidades/Obstaculo.h \
    entidades/Personaje.h \
    entidades/Plataforma.h \
    entidades/ProyectilDron.h \
    fisicas/ModelosFisicos.h \
    gui/GameWidget.h \
    render/SpriteCache.h \
    logica/Dificultad.h \
    logica/JuegoException.h \
    logica/NivelJuego.h \
    logica/NivelRutaAnillos.h \
    logica/NivelPiscinaEntrenamiento.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    recursos.qrc

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
