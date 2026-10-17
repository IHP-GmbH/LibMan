#include "keybindings.h"

#include <QSettings>

namespace KeyBindings {

/*!*********************************************************************************************************************
 * \brief Returns the built-in list of bindable actions and their default shortcuts.
 **********************************************************************************************************************/
QVector<KeyBindingEntry> catalog()
{
    using KS = QKeySequence;
    return {
        {QStringLiteral("open_project"), QStringLiteral("File"),
         QStringLiteral("Open project…"), KS(Qt::CTRL | Qt::Key_O)},
        {QStringLiteral("open_most_recent"), QStringLiteral("File"),
         QStringLiteral("Open most recent project"), KS(Qt::CTRL | Qt::Key_R)},
        {QStringLiteral("save_project"), QStringLiteral("File"),
         QStringLiteral("Save project"), KS(Qt::CTRL | Qt::Key_S)},
        {QStringLiteral("save_project_as"), QStringLiteral("File"),
         QStringLiteral("Save project as…"), KS()},
        {QStringLiteral("edit_project"), QStringLiteral("File"),
         QStringLiteral("Project editor…"), KS(Qt::CTRL | Qt::Key_E)},
        {QStringLiteral("import"), QStringLiteral("File"),
         QStringLiteral("Import…"), KS()},
        {QStringLiteral("export"), QStringLiteral("File"),
         QStringLiteral("Export…"), KS()},
        {QStringLiteral("exit"), QStringLiteral("File"),
         QStringLiteral("Exit"), KS(Qt::CTRL | Qt::Key_Q)},
        {QStringLiteral("reload"), QStringLiteral("View"),
         QStringLiteral("Reload"), KS(Qt::Key_F5)},
        {QStringLiteral("projects_dialog"), QStringLiteral("Settings"),
         QStringLiteral("Projects…"), KS()},
        {QStringLiteral("tools_dialog"), QStringLiteral("Settings"),
         QStringLiteral("Tools…"), KS()},
        {QStringLiteral("key_bindings"), QStringLiteral("Settings"),
         QStringLiteral("Key Bindings…"), KS()},
    };
}

/*!*********************************************************************************************************************
 * \brief Default shortcut map (id → sequence).
 **********************************************************************************************************************/
QMap<QString, QKeySequence> defaults()
{
    QMap<QString, QKeySequence> map;
    for (const KeyBindingEntry &e : catalog()) {
        map.insert(e.id, e.defaultShortcut);
    }
    return map;
}

/*!*********************************************************************************************************************
 * \brief Loads shortcuts from the \c KeyBindings settings group (falls back to defaults).
 **********************************************************************************************************************/
QMap<QString, QKeySequence> load(QSettings &settings)
{
    QMap<QString, QKeySequence> map = defaults();
    settings.beginGroup(QStringLiteral("KeyBindings"));
    for (auto it = map.begin(); it != map.end(); ++it) {
        if (!settings.contains(it.key())) {
            continue;
        }
        const QString text = settings.value(it.key()).toString();
        it.value() = text.isEmpty() ? QKeySequence() : QKeySequence(text, QKeySequence::PortableText);
    }
    settings.endGroup();
    return map;
}

/*!*********************************************************************************************************************
 * \brief Writes shortcuts into the \c KeyBindings settings group (portable text).
 **********************************************************************************************************************/
void save(QSettings &settings, const QMap<QString, QKeySequence> &bindings)
{
    settings.beginGroup(QStringLiteral("KeyBindings"));
    settings.remove(QString());
    for (auto it = bindings.constBegin(); it != bindings.constEnd(); ++it) {
        settings.setValue(it.key(), it.value().toString(QKeySequence::PortableText));
    }
    settings.endGroup();
}

} // namespace KeyBindings
