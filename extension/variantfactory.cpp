/****************************************************************************
**
** Copyright (C) 2006 Trolltech ASA. All rights reserved.
**
** This file is part of the documentation of Qt. It was originally
** published as part of Qt Quarterly.
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation or under the
** terms of the Qt Commercial License Agreement. The respective license
** texts for these are provided with the open source and commercial
** editions of Qt.
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>

#include "variantfactory.h"
#include "variantmanager.h"
#include "fileedit.h"

VariantFactory::~VariantFactory()
{
    QList<FileEdit *> editors = theEditorToProperty.keys();
    QListIterator<FileEdit *> it(editors);
    while (it.hasNext())
        delete it.next();
}

void VariantFactory::connectPropertyManager(QtVariantPropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
                this, SLOT(slotPropertyChanged(QtProperty *, const QVariant &)));
    connect(manager, SIGNAL(attributeChanged(QtProperty *, const QString &, const QVariant &)),
                this, SLOT(slotPropertyAttributeChanged(QtProperty *, const QString &, const QVariant &)));
    QtVariantEditorFactory::connectPropertyManager(manager);
}

QWidget *VariantFactory::createEditor(QtVariantPropertyManager *manager,
        QtProperty *property, QWidget *parent)
{
    if (manager->propertyType(property) == VariantManager::filePathTypeId()) {
        FileEdit *editor;

        //QString keys = property->whatsThis();
        //QMessageBox::information(0, "INFO", keys);

        if( property->whatsThis() == "folder" ) {
            editor = new FileEdit(parent, FileEdit::FOLDER);
        }
        else if( property->whatsThis() == "color" ) {
            editor = new FileEdit(parent, FileEdit::COLOR);
            editor->setIconColor(property->valueColor());
        }
        else if( property->whatsThis() == "keywords" ) {
            editor = new FileEdit(parent, FileEdit::KEYWORDS);
            editor->setKeywords(property->toolTip());
        }
        else if( property->whatsThis() == "runmode" ) {
            editor = new FileEdit(parent, FileEdit::RUNMODE);
        }
        else if( property->whatsThis() == "password" ) {
            editor = new FileEdit(parent, FileEdit::PASSWORD);
        }
        else {
            editor = new FileEdit(parent, FileEdit::FILE);
        }

        QPalette pal;
        if(property->whatsThis() != "color" && property->whatsThis() != "keywords") {
            if(QFileInfo( manager->value(property).toString()).isReadable()) {
                pal.setColor(QPalette::Text, Qt::blue);
            }
            else {
                pal.setColor(QPalette::Text, Qt::red);
            }
        }
        else {
            QColor colorId = QColor(manager->value(property).toString());
            if(colorId.isValid()) {
                pal.setColor(QPalette::Text, colorId);
            }
            else {
                pal.setColor(QPalette::Text, Qt::black);
            }
        }

        editor->setPalette(pal);

        editor->setDialogTitle(property->statusTip());
        editor->setFilePath(manager->value(property).toString());
        editor->setFilter(manager->attributeValue(property, QLatin1String("filter")).toString());
        theCreatedEditors[property].append(editor);
        theEditorToProperty[editor] = property;

        connect(editor, SIGNAL(filePathChanged(const QString &)),
                this, SLOT(slotSetValue(const QString &)));
        connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));
        return editor;
    }

    return QtVariantEditorFactory::createEditor(manager, property, parent);
}

void VariantFactory::disconnectPropertyManager(QtVariantPropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
                this, SLOT(slotPropertyChanged(QtProperty *, const QVariant &)));
    disconnect(manager, SIGNAL(attributeChanged(QtProperty *, const QString &, const QVariant &)),
                this, SLOT(slotPropertyAttributeChanged(QtProperty *, const QString &, const QVariant &)));
    QtVariantEditorFactory::disconnectPropertyManager(manager);
}

void VariantFactory::slotPropertyChanged(QtProperty *property,
                const QVariant &value)
{
    if (!theCreatedEditors.contains(property))
        return;

    QList<FileEdit *> editors = theCreatedEditors[property];
    QListIterator<FileEdit *> itEditor(editors);
    while (itEditor.hasNext())
        itEditor.next()->setFilePath(value.toString());
}

void VariantFactory::slotPropertyAttributeChanged(QtProperty *property,
            const QString &attribute, const QVariant &value)
{
    if (!theCreatedEditors.contains(property))
        return;

    if (attribute != QLatin1String("filter"))
        return;

    QList<FileEdit *> editors = theCreatedEditors[property];
    QListIterator<FileEdit *> itEditor(editors);
    while (itEditor.hasNext())
        itEditor.next()->setFilter(value.toString());
}

void VariantFactory::slotSetValue(const QString &value)
{
    QObject *object = sender();
    QMap<FileEdit *, QtProperty *>::ConstIterator itEditor =
                theEditorToProperty.constBegin();
    while (itEditor != theEditorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtVariantPropertyManager *manager = propertyManager(property);
            if (!manager)
                return;

            FileEdit *editor = static_cast<FileEdit*> (object);

            QPalette pal;
            FileEdit::TYPE type = editor->getType();
            if(type != FileEdit::COLOR && type != FileEdit::KEYWORDS) {
                if(QFileInfo(value).isReadable()) {
                    pal.setColor(QPalette::Text, Qt::blue);
                }
                else {
                    pal.setColor(QPalette::Text, Qt::red);
                }
            }
            else if(type != FileEdit::KEYWORDS) {
                pal.setColor(QPalette::Text, QColor(value));
                editor->setIconColor(value);
                property->setColor(QColor(value));
            }

            editor->setPalette(pal);
            object = static_cast<QObject*> (editor);

            if(type != FileEdit::KEYWORDS) {
                manager->setValue(property, value);
            }
            else {
                QString keywords = editor->getKeywords();
                property->setToolTip(keywords);
            }

            return;
        }
        itEditor++;
    }
}

void VariantFactory::slotEditorDestroyed(QObject *object)
{
    QMap<FileEdit *, QtProperty *>::ConstIterator itEditor =
                theEditorToProperty.constBegin();
    while (itEditor != theEditorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            FileEdit *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            theEditorToProperty.remove(editor);
            theCreatedEditors[property].removeAll(editor);
            if (theCreatedEditors[property].isEmpty())
                theCreatedEditors.remove(property);
            return;
        }
        itEditor++;
    }
}

