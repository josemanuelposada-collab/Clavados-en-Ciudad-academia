#ifndef SPRITECACHE_H
#define SPRITECACHE_H

#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QString>

class SpriteCache
{
public:
    static void dibujarAjustado(QPainter& painter,
                                const QPixmap& sprite,
                                const QRect& destino,
                                const QString& etiqueta);
};

#endif
