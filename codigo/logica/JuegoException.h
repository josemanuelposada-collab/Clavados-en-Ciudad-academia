#ifndef JUEGOEXCEPTION_H
#define JUEGOEXCEPTION_H

#include <stdexcept>
#include <QString>

class JuegoException : public std::runtime_error
{
public:
    explicit JuegoException(const QString& mensaje)
        : std::runtime_error(mensaje.toStdString())
    {
    }
};

#endif
