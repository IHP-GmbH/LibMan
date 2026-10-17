#ifndef KEYBINDINGS_H
#define KEYBINDINGS_H

#include <QKeySequence>
#include <QMap>
#include <QString>
#include <QVector>

class QSettings;

/*!*********************************************************************************************************************
 * \brief One configurable LibMan shortcut (id ↔ menu action).
 **********************************************************************************************************************/
struct KeyBindingEntry
{
    QString     id;
    QString     category;
    QString     label;
    QKeySequence defaultShortcut;
};

/*!*********************************************************************************************************************
 * \brief Catalog, persistence and defaults for LibMan key bindings.
 **********************************************************************************************************************/
namespace KeyBindings {

QVector<KeyBindingEntry> catalog();
QMap<QString, QKeySequence> defaults();
QMap<QString, QKeySequence> load(QSettings &settings);
void save(QSettings &settings, const QMap<QString, QKeySequence> &bindings);

} // namespace KeyBindings

#endif // KEYBINDINGS_H
