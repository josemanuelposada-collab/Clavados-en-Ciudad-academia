#include "SpriteCache.h"

#include <QHash>
#include <QPixmapCache>

const QPixmap& SpriteCache::obtener(const QString& ruta)
{
    static QHash<QString, QPixmap> cacheOriginales;

    auto it = cacheOriginales.find(ruta);
    if (it == cacheOriginales.end()) {
        it = cacheOriginales.insert(ruta, QPixmap(ruta));
    }

    return it.value();
}

void SpriteCache::dibujarAjustado(QPainter& painter,
                                  const QPixmap& sprite,
                                  const QRect& destino,
                                  const QString& etiqueta)
{
    if (sprite.isNull() || destino.isEmpty()) {
        return;
    }

    QSize tamano = sprite.size();
    tamano.scale(destino.size(), Qt::KeepAspectRatio);

    const QString clave = QString("%1:%2:%3x%4")
                              .arg(etiqueta)
                              .arg(sprite.cacheKey())
                              .arg(tamano.width())
                              .arg(tamano.height());

    QPixmap escalado;
    if (!QPixmapCache::find(clave, &escalado)) {
        escalado = sprite.scaled(tamano, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPixmapCache::insert(clave, escalado);
    }

    QRect centrado(destino);
    centrado.setSize(tamano);
    centrado.moveCenter(destino.center());
    painter.drawPixmap(centrado, escalado);
}
