#ifndef ROOMKLAYOUTBRIDGE_H
#define ROOMKLAYOUTBRIDGE_H

#include <QString>
#include <QStringList>

QString roomLayoutPathForKLayout(const QString &viewPath, QStringList *errors = nullptr);

#endif // ROOMKLAYOUTBRIDGE_H
