#ifndef COREKLAYOUTBRIDGE_H
#define COREKLAYOUTBRIDGE_H

#include <QString>
#include <QStringList>

QString coreLayoutPathForKLayout(const QString &viewPath, QStringList *errors = nullptr);

#endif // COREKLAYOUTBRIDGE_H
