QT += widgets

CONFIG += c++17

SOURCES += \
    entidades/Personaje.cpp \
    entidades/Plataforma.cpp \
    gui/GameWidget.cpp \
    logica/NivelPiscinaEntrenamiento.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    entidades/Personaje.h \
    entidades/Plataforma.h \
    gui/GameWidget.h \
    logica/NivelPiscinaEntrenamiento.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target