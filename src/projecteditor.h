#ifndef PROJECTEDITOR_H
#define PROJECTEDITOR_H

#include <QWidget>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QVector>

class MainWindow;
class QtVariantProperty;
class QtProperty;
class QtTreePropertyBrowser;
class VariantManager;
class QTreeWidgetItem;

namespace Ui {
class ProjectEditor;
}

class ProjectEditor : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectEditor(MainWindow *parent);
    ~ProjectEditor() override;

    void                                reloadFromDisk();
    bool                                confirmHide();

private slots:
    void                                on_btnAddLibrary_clicked();
    void                                on_btnRemoveLibrary_clicked();
    void                                on_btnAddPath_clicked();
    void                                on_btnRemovePath_clicked();
    void                                on_btnSave_clicked();
    void                                on_btnClose_clicked();
    void                                onLibrarySelectionChanged();
    void                                onPathValueChanged(QtProperty *property, const QVariant &value);

private:
    struct LibraryEntry {
        QString                         name;
        QStringList                     paths;
    };

    void                                setLibraries(const QVector<LibraryEntry> &libraries);
    void                                flushCurrentPaths();
    void                                showLibrary(int index);
    void                                refreshLibraryLabel(int index);
    void                                refreshPathLabel(QtProperty *property);
    void                                fitNameColumn();
    QtVariantProperty                  *addPathProperty(const QString &path);
    bool                                pathExists(const QString &path) const;
    QString                             pathLabel(const QString &path) const;
    QList<QPair<QString, QString>>      collectEntries() const;
    bool                                saveToFile(const QString &filePath);
    QString                             viewFileFilter() const;

private:
    Ui::ProjectEditor                    *m_ui = nullptr;
    MainWindow                           *m_mainWindow = nullptr;
    QtTreePropertyBrowser                *m_browser = nullptr;
    VariantManager                       *m_manager = nullptr;
    QtVariantProperty                    *m_paths = nullptr;
    QVector<LibraryEntry>                 m_libraries;
    int                                   m_currentLibrary = -1;
    QString                               m_filePath;
    bool                                  m_modified = false;
    bool                                  m_loading = false;
};

#endif // PROJECTEDITOR_H
