/****************************************************************************
** 
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
** 
** This file is part of a Qt Solutions component.
**
** Commercial Usage  
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
** 
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
** 
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.1, included in the file LGPL_EXCEPTION.txt in this
** package.
** 
** GNU General Public License Usage 
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
** 
** Please note Third Party Software included with Qt Solutions may impose
** additional restrictions and it is the user's responsibility to ensure
** that they have met the licensing requirements of the GPL, LGPL, or Qt
** Solutions Commercial license and the relevant license of the Third
** Party Software they are using.
** 
** If you are unsure which license is appropriate for your use, please
** contact Nokia at qt-info@nokia.com.
** 
****************************************************************************/


#include "qttreepropertybrowser.h"
#include <QtCore/QSet>
#include <QtGui/QIcon>
#include <QTreeWidget>
#include <QItemDelegate>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPainter>
#include <QApplication>
#include <QtGui/QFocusEvent>
#include <QStyle>
#include <QFileInfo>
#include <QtGui/QPalette>

#include <QtGui>
#include <QHeaderView>
#include <QStyleOptionViewItem>

#if QT_VERSION >= 0x040400
QT_BEGIN_NAMESPACE
#endif

class QtPropertyEditorView;

class QtTreePropertyBrowserPrivate
{
    QtTreePropertyBrowser *q_ptr;
    Q_DECLARE_PUBLIC(QtTreePropertyBrowser)

public:
    QtTreePropertyBrowserPrivate();
    void init(QWidget *parent);

    void propertyInserted(QtBrowserItem *index, QtBrowserItem *afterIndex);
    void propertyRemoved(QtBrowserItem *index);
    void propertyChanged(QtBrowserItem *index);
    QWidget *createEditor(QtProperty *property, QWidget *parent) const
        { return q_ptr->createEditor(property, parent); }
    QtProperty *indexToProperty(const QModelIndex &index) const;
    QTreeWidgetItem *indexToItem(const QModelIndex &index) const;
    QtBrowserItem *indexToBrowserItem(const QModelIndex &index) const;
    bool lastColumn(int column) const;
    void disableItem(QTreeWidgetItem *item) const;
    void enableItem(QTreeWidgetItem *item) const;
    bool hasValue(QTreeWidgetItem *item) const;

    void slotCollapsed(const QModelIndex &index);
    void slotExpanded(const QModelIndex &index);

    QColor calculatedBackgroundColor(QtBrowserItem *item) const;

    QtPropertyEditorView *treeWidget() const { return m_treeWidget; }
    bool markPropertiesWithoutValue() const { return m_markPropertiesWithoutValue; }

    QtBrowserItem *currentItem() const;
    void setCurrentItem(QtBrowserItem *browserItem, bool block);
    void editItem(QtBrowserItem *browserItem);

    void slotCurrentBrowserItemChanged(QtBrowserItem *item);
    void slotCurrentTreeItemChanged(QTreeWidgetItem *newItem, QTreeWidgetItem *);

    QTreeWidgetItem *editedItem() const;

private:
    void updateItem(QTreeWidgetItem *item);

    QMap<QtBrowserItem *, QTreeWidgetItem *> m_indexToItem;
    QMap<QTreeWidgetItem *, QtBrowserItem *> m_itemToIndex;

    QMap<QtBrowserItem *, QColor> m_indexToBackgroundColor;

    QtPropertyEditorView *m_treeWidget;

    bool m_headerVisible;
    QtTreePropertyBrowser::ResizeMode m_resizeMode;
    class QtPropertyEditorDelegate *m_delegate;
    bool m_markPropertiesWithoutValue;
    bool m_browserChangedBlocked;
    QIcon m_expandIcon;
};

// ------------ QtPropertyEditorView
class QtPropertyEditorView : public QTreeWidget
{
    Q_OBJECT
public:
    QtPropertyEditorView(QWidget *parent = 0);

    void setEditorPrivate(QtTreePropertyBrowserPrivate *editorPrivate)
        { m_editorPrivate = editorPrivate; }

    QTreeWidgetItem *indexToItem(const QModelIndex &index) const
        { return itemFromIndex(index); }

protected:
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    QtTreePropertyBrowserPrivate *m_editorPrivate;
};

QtPropertyEditorView::QtPropertyEditorView(QWidget *parent) :
    QTreeWidget(parent),
    m_editorPrivate(0)
{
    connect(header(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(resizeColumnToContents(int)));
}

void QtPropertyEditorView::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    bool hasValue = true;
    if (m_editorPrivate) {
        QtProperty *property = m_editorPrivate->indexToProperty(index);
        if (property)
            hasValue = property->hasValue();
    }
    if (!hasValue && m_editorPrivate->markPropertiesWithoutValue()) {
        const QColor c = option.palette.color(QPalette::Dark);
        painter->fillRect(option.rect, c);
        opt.palette.setColor(QPalette::AlternateBase, c);
    } else {
        const QColor c = m_editorPrivate->calculatedBackgroundColor(m_editorPrivate->indexToBrowserItem(index));
        if (c.isValid()) {
            painter->fillRect(option.rect, c);
            opt.palette.setColor(QPalette::AlternateBase, c.lighter(112));
        }
    }
    QTreeWidget::drawRow(painter, opt, index);
    QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt));
    painter->save();
    painter->setPen(QPen(color));
    painter->drawLine(opt.rect.x(), opt.rect.bottom(), opt.rect.right(), opt.rect.bottom());
    painter->restore();
}

void QtPropertyEditorView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Space: // Trigger Edit
        if (!m_editorPrivate->editedItem())
            if (const QTreeWidgetItem *item = currentItem())
                if (item->columnCount() >= 2 && ((item->flags() & (Qt::ItemIsEditable | Qt::ItemIsEnabled)) == (Qt::ItemIsEditable | Qt::ItemIsEnabled))) {
                    event->accept();
                    // If the current position is at column 0, move to 1.
                    QModelIndex index = currentIndex();
                    if (index.column() == 0) {
                        index = index.sibling(index.row(), 1);
                        setCurrentIndex(index);
                    }
                    edit(index);
                    return;
                }
        break;
    default:
        break;
    }
    QTreeWidget::keyPressEvent(event);
}

void QtPropertyEditorView::mousePressEvent(QMouseEvent *event)
{
    QTreeWidget::mousePressEvent(event);
    QTreeWidgetItem *item = itemAt(event->pos());

    if (item) {
        if ((item != m_editorPrivate->editedItem()) && (event->button() == Qt::LeftButton)
                && (header()->logicalIndexAt(event->pos().x()) == 1)
                && ((item->flags() & (Qt::ItemIsEditable | Qt::ItemIsEnabled)) == (Qt::ItemIsEditable | Qt::ItemIsEnabled))) {
            editItem(item, 1);
        } else if (!m_editorPrivate->hasValue(item) && m_editorPrivate->markPropertiesWithoutValue() && !rootIsDecorated()) {
            if (event->pos().x() + header()->offset() < 20)
                item->setExpanded(!item->isExpanded());
        }
    }
}

// ------------ QtPropertyEditorDelegate
class QtPropertyEditorDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    QtPropertyEditorDelegate(QObject *parent = 0)
        : QItemDelegate(parent), m_editorPrivate(0), m_editedItem(0), m_editedWidget(0)
        {}

    void setEditorPrivate(QtTreePropertyBrowserPrivate *editorPrivate)
        { m_editorPrivate = editorPrivate; }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
            const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
            const QModelIndex &index) const;

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
            const QModelIndex &index) const;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setModelData(QWidget *, QAbstractItemModel *,
            const QModelIndex &) const {}

    void setEditorData(QWidget *, const QModelIndex &) const {}

    bool eventFilter(QObject *object, QEvent *event);
    void closeEditor(QtProperty *property);

    QTreeWidgetItem *editedItem() const { return m_editedItem; }

private slots:
    void slotEditorDestroyed(QObject *object);

private:
    int indentation(const QModelIndex &index) const;

    typedef QMap<QWidget *, QtProperty *> EditorToPropertyMap;
    mutable EditorToPropertyMap m_editorToProperty;

    typedef QMap<QtProperty *, QWidget *> PropertyToEditorMap;
    mutable PropertyToEditorMap m_propertyToEditor;
    QtTreePropertyBrowserPrivate *m_editorPrivate;
    mutable QTreeWidgetItem *m_editedItem;
    mutable QWidget *m_editedWidget;
};

int QtPropertyEditorDelegate::indentation(const QModelIndex &index) const
{
    if (!m_editorPrivate)
        return 0;

    QTreeWidgetItem *item = m_editorPrivate->indexToItem(index);
    int indent = 0;
    while (item->parent()) {
        item = item->parent();
        ++indent;
    }
    if (m_editorPrivate->treeWidget()->rootIsDecorated())
        ++indent;
    return indent * m_editorPrivate->treeWidget()->indentation();
}

void QtPropertyEditorDelegate::slotEditorDestroyed(QObject *object)
{
    if (QWidget *w = qobject_cast<QWidget *>(object)) {
        const EditorToPropertyMap::iterator it = m_editorToProperty.find(w);
        if (it != m_editorToProperty.end()) {
            m_propertyToEditor.remove(it.value());
            m_editorToProperty.erase(it);
        }
        if (m_editedWidget == w) {
            m_editedWidget = 0;
            m_editedItem = 0;
        }
    }
}

void QtPropertyEditorDelegate::closeEditor(QtProperty *property)
{
    if (QWidget *w = m_propertyToEditor.value(property, 0))
        w->deleteLater();
}

QWidget *QtPropertyEditorDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &, const QModelIndex &index) const
{
    if (index.column() == 1 && m_editorPrivate) {
        QtProperty *property = m_editorPrivate->indexToProperty(index);
        QTreeWidgetItem *item = m_editorPrivate->indexToItem(index);
        if (property && item && (item->flags() & Qt::ItemIsEnabled)) {
            QWidget *editor = m_editorPrivate->createEditor(property, parent);
            if (editor) {
                editor->setAutoFillBackground(true);
                editor->installEventFilter(const_cast<QtPropertyEditorDelegate *>(this));
                connect(editor, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDestroyed(QObject *)));
                m_propertyToEditor[property] = editor;
                m_editorToProperty[editor] = property;
                m_editedItem = item;
                m_editedWidget = editor;
            }
            return editor;
        }
    }
    return 0;
}

void QtPropertyEditorDelegate::updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect.adjusted(0, 0, 0, -1));
}

void QtPropertyEditorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
            const QModelIndex &index) const
{
    bool hasValue = true;
    if (m_editorPrivate) {
        QtProperty *property = m_editorPrivate->indexToProperty(index);
        if (property)
            hasValue = property->hasValue();
    }

#if QT_VERSION >= 0x050000
    QStyleOptionViewItem opt = option;
#else
    QStyleOptionViewItemV2 opt = option;
#endif

    if ((m_editorPrivate && index.column() == 0) || !hasValue) {
        QtProperty *property = m_editorPrivate->indexToProperty(index);
        if (property && property->isModified()) {
            opt.font.setBold(true);
            opt.fontMetrics = QFontMetrics(opt.font);
        }
    }
    QColor c;
    if (!hasValue && m_editorPrivate->markPropertiesWithoutValue()) {
        c = opt.palette.color(QPalette::Dark);
        opt.palette.setColor(QPalette::Text, opt.palette.color(QPalette::BrightText));
    } else {
        c = m_editorPrivate->calculatedBackgroundColor(m_editorPrivate->indexToBrowserItem(index));
#if QT_VERSION >= 0x050000
        if (c.isValid() && (opt.features & QStyleOptionViewItem::Alternate))
#else
        if (c.isValid() && (opt.features & QStyleOptionViewItemV2::Alternate))
#endif
            c = c.lighter(112);
    }
    if (c.isValid())
        painter->fillRect(option.rect, c);
    opt.state &= ~QStyle::State_HasFocus;
    QItemDelegate::paint(painter, opt, index);

    opt.palette.setCurrentColorGroup(QPalette::Active);
    QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt));
    painter->save();
    painter->setPen(QPen(color));
    if (!m_editorPrivate || (!m_editorPrivate->lastColumn(index.column()) && hasValue)) {
        int right = (option.direction == Qt::LeftToRight) ? option.rect.right() : option.rect.left();
        painter->drawLine(right, option.rect.y(), right, option.rect.bottom());
    }
    painter->restore();
}

QSize QtPropertyEditorDelegate::sizeHint(const QStyleOptionViewItem &option,
            const QModelIndex &index) const
{
    return QItemDelegate::sizeHint(option, index) + QSize(3, 4);
}

bool QtPropertyEditorDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::FocusOut) {
        QFocusEvent *fe = static_cast<QFocusEvent *>(event);
        if (fe->reason() == Qt::ActiveWindowFocusReason)
            return false;
    }
    return QItemDelegate::eventFilter(object, event);
}

//  -------- QtTreePropertyBrowserPrivate implementation
QtTreePropertyBrowserPrivate::QtTreePropertyBrowserPrivate() :
    m_treeWidget(0),
    m_headerVisible(true),
    m_resizeMode(QtTreePropertyBrowser::Stretch),
    m_delegate(0),
    m_markPropertiesWithoutValue(false),
    m_browserChangedBlocked(false)
{
}

// Draw an icon indicating opened/closing branches
static QIcon drawIndicatorIcon(const QPalette &palette, QStyle *style)
{
    QPixmap pix(14, 14);
    pix.fill(Qt::transparent);
    QStyleOption branchOption;
    QRect r(QPoint(0, 0), pix.size());
    branchOption.rect = QRect(2, 2, 9, 9); // ### hardcoded in qcommonstyle.cpp
    branchOption.palette = palette;
    branchOption.state = QStyle::State_Children;

    QPainter p;
    // Draw closed state
    p.begin(&pix);
    style->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, &p);
    p.end();
    QIcon rc = pix;
    rc.addPixmap(pix, QIcon::Selected, QIcon::Off);
    // Draw opened state
    branchOption.state |= QStyle::State_Open;
    pix.fill(Qt::transparent);
    p.begin(&pix);
    style->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, &p);
    p.end();

    rc.addPixmap(pix, QIcon::Normal, QIcon::On);
    rc.addPixmap(pix, QIcon::Selected, QIcon::On);
    return rc;
}

void QtTreePropertyBrowserPrivate::init(QWidget *parent)
{
    QHBoxLayout *layout = new QHBoxLayout(parent);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    layout->setMargin(0);  // Qt5
#else
    layout->setContentsMargins(0, 0, 0, 0);  // Qt6
#endif
    m_treeWidget = new QtPropertyEditorView(parent);
    m_treeWidget->setEditorPrivate(this);
    m_treeWidget->setIconSize(QSize(18, 18));
    layout->addWidget(m_treeWidget);

    m_treeWidget->setColumnCount(2);
    QStringList labels;
#if QT_VERSION >= 0x050000
    labels.append(QApplication::translate("QtTreePropertyBrowser", "Run parameter", 0));
    labels.append(QApplication::translate("QtTreePropertyBrowser", "Value", 0));
#else
    labels.append(QApplication::translate("QtTreePropertyBrowser", "Run parameter", 0, QApplication::UnicodeUTF8));
    labels.append(QApplication::translate("QtTreePropertyBrowser", "Value", 0, QApplication::UnicodeUTF8));
#endif
    m_treeWidget->setHeaderLabels(labels);
    m_treeWidget->setAlternatingRowColors(true);
    m_treeWidget->setEditTriggers(QAbstractItemView::EditKeyPressed);
    m_delegate = new QtPropertyEditorDelegate(parent);
    m_delegate->setEditorPrivate(this);
    m_treeWidget->setItemDelegate(m_delegate);
#if QT_VERSION >= 0x050000
    m_treeWidget->header()->setSectionsMovable(false);
    m_treeWidget->header()->setSectionResizeMode(QHeaderView::Stretch);
#else
    m_treeWidget->header()->setMovable(false);
    m_treeWidget->header()->setResizeMode(QHeaderView::Stretch);
#endif
    m_expandIcon = drawIndicatorIcon(q_ptr->palette(), q_ptr->style());

    QObject::connect(m_treeWidget, SIGNAL(collapsed(const QModelIndex &)), q_ptr, SLOT(slotCollapsed(const QModelIndex &)));
    QObject::connect(m_treeWidget, SIGNAL(expanded(const QModelIndex &)), q_ptr, SLOT(slotExpanded(const QModelIndex &)));
    QObject::connect(m_treeWidget, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), q_ptr, SLOT(slotCurrentTreeItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
}

QtBrowserItem *QtTreePropertyBrowserPrivate::currentItem() const
{
    if (QTreeWidgetItem *treeItem = m_treeWidget->currentItem())
        return m_itemToIndex.value(treeItem);
    return 0;
}

void QtTreePropertyBrowserPrivate::setCurrentItem(QtBrowserItem *browserItem, bool block)
{
    const bool blocked = block ? m_treeWidget->blockSignals(true) : false;
    if (browserItem == 0)
        m_treeWidget->setCurrentItem(0);
    else
        m_treeWidget->setCurrentItem(m_indexToItem.value(browserItem));
    if (block)
        m_treeWidget->blockSignals(blocked);
}

QtProperty *QtTreePropertyBrowserPrivate::indexToProperty(const QModelIndex &index) const
{
    QTreeWidgetItem *item = m_treeWidget->indexToItem(index);
    QtBrowserItem *idx = m_itemToIndex.value(item);
    if (idx)
        return idx->property();
    return 0;
}

QtBrowserItem *QtTreePropertyBrowserPrivate::indexToBrowserItem(const QModelIndex &index) const
{
    QTreeWidgetItem *item = m_treeWidget->indexToItem(index);
    return m_itemToIndex.value(item);
}

QTreeWidgetItem *QtTreePropertyBrowserPrivate::indexToItem(const QModelIndex &index) const
{
    return m_treeWidget->indexToItem(index);
}

bool QtTreePropertyBrowserPrivate::lastColumn(int column) const
{
    return m_treeWidget->header()->visualIndex(column) == m_treeWidget->columnCount() - 1;
}

void QtTreePropertyBrowserPrivate::disableItem(QTreeWidgetItem *item) const
{
    Qt::ItemFlags flags = item->flags();
    if (flags & Qt::ItemIsEnabled) {
        flags &= ~Qt::ItemIsEnabled;
        item->setFlags(flags);
        m_delegate->closeEditor(m_itemToIndex[item]->property());
        const int childCount = item->childCount();
        for (int i = 0; i < childCount; i++) {
            QTreeWidgetItem *child = item->child(i);
            disableItem(child);
        }
    }
}

void QtTreePropertyBrowserPrivate::enableItem(QTreeWidgetItem *item) const
{
    Qt::ItemFlags flags = item->flags();
    flags |= Qt::ItemIsEnabled;
    item->setFlags(flags);
    const int childCount = item->childCount();
    for (int i = 0; i < childCount; i++) {
        QTreeWidgetItem *child = item->child(i);
        QtProperty *property = m_itemToIndex[child]->property();
        if (property->isEnabled()) {
            enableItem(child);
        }
    }
}

bool QtTreePropertyBrowserPrivate::hasValue(QTreeWidgetItem *item) const
{
    QtBrowserItem *browserItem = m_itemToIndex.value(item);
    if (browserItem)
        return browserItem->property()->hasValue();
    return false;
}

void QtTreePropertyBrowserPrivate::propertyInserted(QtBrowserItem *index, QtBrowserItem *afterIndex)
{
    QTreeWidgetItem *afterItem = m_indexToItem.value(afterIndex);
    QTreeWidgetItem *parentItem = m_indexToItem.value(index->parent());

    QTreeWidgetItem *newItem = 0;
    if (parentItem) {
        newItem = new QTreeWidgetItem(parentItem, afterItem);
    } else {
        newItem = new QTreeWidgetItem(m_treeWidget, afterItem);
    }
    m_itemToIndex[newItem] = index;
    m_indexToItem[index] = newItem;

    newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);

#if QT_VERSION >= 0x050000
    newItem->setExpanded(true);
#else
    m_treeWidget->setItemExpanded(newItem, true);
#endif

    updateItem(newItem);
}

void QtTreePropertyBrowserPrivate::propertyRemoved(QtBrowserItem *index)
{
    QTreeWidgetItem *item = m_indexToItem.value(index);

    if (m_treeWidget->currentItem() == item) {
        m_treeWidget->setCurrentItem(0);
    }

    delete item;

    m_indexToItem.remove(index);
    m_itemToIndex.remove(item);
    m_indexToBackgroundColor.remove(index);
}

void QtTreePropertyBrowserPrivate::propertyChanged(QtBrowserItem *index)
{
    QTreeWidgetItem *item = m_indexToItem.value(index);

    updateItem(item);
}

void QtTreePropertyBrowserPrivate::updateItem(QTreeWidgetItem *item)
{
    QtProperty *property = m_itemToIndex[item]->property();
    QIcon expandIcon;
    if (property->hasValue()) {
        QString toolTip = property->toolTip();
        if (toolTip.isEmpty())
            toolTip = property->valueText();
        item->setToolTip(1, toolTip);
        item->setIcon(1, property->valueIcon());
        item->setText(1, property->valueText());
        //item->setTextColor(1, property->valueColor());
        item->setForeground(1, property->valueColor());
        if(property->whatsThis() == "password") {
            //item->setTextColor(1, Qt::black);
            item->setForeground(1, Qt::black);
            QString passwd = property->valueText();
            QString hpasswd = "";
            for(int i = 0; i < passwd.length(); ++i) {
                hpasswd += "*";
            }
            item->setText(1, hpasswd);
            item->setStatusTip(1, passwd);
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
        }
    } else if (markPropertiesWithoutValue() && !m_treeWidget->rootIsDecorated()) {
        expandIcon = m_expandIcon;
    }
    item->setIcon(0, expandIcon);
    item->setFirstColumnSpanned(!property->hasValue());
    item->setToolTip(0, property->propertyName());
    item->setStatusTip(0, property->statusTip());
    item->setWhatsThis(0, property->whatsThis());
    item->setText(0, property->propertyName());
    bool wasEnabled = item->flags() & Qt::ItemIsEnabled;
    bool isEnabled = wasEnabled;
    if (property->isEnabled()) {
        QTreeWidgetItem *parent = item->parent();
        if (!parent || (parent->flags() & Qt::ItemIsEnabled))
            isEnabled = true;
        else
            isEnabled = false;
    } else {
        isEnabled = false;
    }
    if (wasEnabled != isEnabled) {
        if (isEnabled)
            enableItem(item);
        else
            disableItem(item);
    }
    m_treeWidget->viewport()->update();
}

QColor QtTreePropertyBrowserPrivate::calculatedBackgroundColor(QtBrowserItem *item) const
{
    QtBrowserItem *i = item;
    const QMap<QtBrowserItem *, QColor>::const_iterator itEnd = m_indexToBackgroundColor.constEnd();
    while (i) {
        QMap<QtBrowserItem *, QColor>::const_iterator it = m_indexToBackgroundColor.constFind(i);
        if (it != itEnd)
            return it.value();
        i = i->parent();
    }
    return QColor();
}

void QtTreePropertyBrowserPrivate::slotCollapsed(const QModelIndex &index)
{
    QTreeWidgetItem *item = indexToItem(index);
    QtBrowserItem *idx = m_itemToIndex.value(item);
    if (item)
        emit q_ptr->collapsed(idx);
}

void QtTreePropertyBrowserPrivate::slotExpanded(const QModelIndex &index)
{
    QTreeWidgetItem *item = indexToItem(index);
    QtBrowserItem *idx = m_itemToIndex.value(item);
    if (item)
        emit q_ptr->expanded(idx);
}

void QtTreePropertyBrowserPrivate::slotCurrentBrowserItemChanged(QtBrowserItem *item)
{
    if (!m_browserChangedBlocked && item != currentItem())
        setCurrentItem(item, true);
}

void QtTreePropertyBrowserPrivate::slotCurrentTreeItemChanged(QTreeWidgetItem *newItem, QTreeWidgetItem *oldItem)
{
    if( oldItem ) {
        QtBrowserItem *prevBrwsItem = m_itemToIndex.value(oldItem);
        if(prevBrwsItem) {
            QString path = prevBrwsItem->property()->valueText();
            if(prevBrwsItem->property()->statusTip() == "file" ) {
                if(QFileInfo(path).isExecutable()) {
                    //oldItem->setTextColor(1, QColor(Qt::blue));
                }
                else {
                    //oldItem->setTextColor(1, QColor(Qt::red));
                }
            }
        }
    }

    QtBrowserItem *browserItem = newItem ? m_itemToIndex.value(newItem) : 0;
    m_browserChangedBlocked = true;
    q_ptr->setCurrentItem(browserItem);
    m_browserChangedBlocked = false;
}

QTreeWidgetItem *QtTreePropertyBrowserPrivate::editedItem() const
{
    return m_delegate->editedItem();
}

void QtTreePropertyBrowserPrivate::editItem(QtBrowserItem *browserItem)
{
    if (QTreeWidgetItem *treeItem = m_indexToItem.value(browserItem, 0)) {
        m_treeWidget->setCurrentItem (treeItem, 1);
        m_treeWidget->editItem(treeItem, 1);
    }
}

/*!
    \class QtTreePropertyBrowser

    \brief The QtTreePropertyBrowser class provides QTreeWidget based
    property browser.

    A property browser is a widget that enables the user to edit a
    given set of properties. Each property is represented by a label
    specifying the property's name, and an editing widget (e.g. a line
    edit or a combobox) holding its value. A property can have zero or
    more subproperties.

    QtTreePropertyBrowser provides a tree based view for all nested
    properties, i.e. properties that have subproperties can be in an
    expanded (subproperties are visible) or collapsed (subproperties
    are hidden) state. For example:

    \image qttreepropertybrowser.png

    Use the QtAbstractPropertyBrowser API to add, insert and remove
    properties from an instance of the QtTreePropertyBrowser class.
    The properties themselves are created and managed by
    implementations of the QtAbstractPropertyManager class.

    \sa QtGroupBoxPropertyBrowser, QtAbstractPropertyBrowser
*/

/*!
    \fn void QtTreePropertyBrowser::collapsed(QtBrowserItem *item)

    This signal is emitted when the \a item is collapsed.

    \sa expanded(), setExpanded()
*/

/*!
    \fn void QtTreePropertyBrowser::expanded(QtBrowserItem *item)

    This signal is emitted when the \a item is expanded.

    \sa collapsed(), setExpanded()
*/

/*!
    Creates a property browser with the given \a parent.
*/
QtTreePropertyBrowser::QtTreePropertyBrowser(QWidget *parent)
    : QtAbstractPropertyBrowser(parent)
{
    d_ptr = new QtTreePropertyBrowserPrivate;
    d_ptr->q_ptr = this;

    d_ptr->init(this);
    connect(this, SIGNAL(currentItemChanged(QtBrowserItem*)), this, SLOT(slotCurrentBrowserItemChanged(QtBrowserItem*)));
}

/*!
    Destroys this property browser.

    Note that the properties that were inserted into this browser are
    \e not destroyed since they may still be used in other
    browsers. The properties are owned by the manager that created
    them.

    \sa QtProperty, QtAbstractPropertyManager
*/
QtTreePropertyBrowser::~QtTreePropertyBrowser()
{
    delete d_ptr;
}

/*!
    \property QtTreePropertyBrowser::indentation
    \brief indentation of the items in the tree view.
*/
int QtTreePropertyBrowser::indentation() const
{
    return d_ptr->m_treeWidget->indentation();
}

void QtTreePropertyBrowser::setIndentation(int i)
{
    d_ptr->m_treeWidget->setIndentation(i);
}

/*!
  \property QtTreePropertyBrowser::rootIsDecorated
  \brief whether to show controls for expanding and collapsing root items.
*/
bool QtTreePropertyBrowser::rootIsDecorated() const
{
    return d_ptr->m_treeWidget->rootIsDecorated();
}

void QtTreePropertyBrowser::setRootIsDecorated(bool show)
{
    d_ptr->m_treeWidget->setRootIsDecorated(show);
    QMapIterator<QTreeWidgetItem *, QtBrowserItem *> it(d_ptr->m_itemToIndex);
    while (it.hasNext()) {
        QtProperty *property = it.next().value()->property();
        if (!property->hasValue())
            d_ptr->updateItem(it.key());
    }
}

/*!
  \property QtTreePropertyBrowser::alternatingRowColors
  \brief whether to draw the background using alternating colors.
  By default this property is set to true.
*/
bool QtTreePropertyBrowser::alternatingRowColors() const
{
    return d_ptr->m_treeWidget->alternatingRowColors();
}

void QtTreePropertyBrowser::setAlternatingRowColors(bool enable)
{
    d_ptr->m_treeWidget->setAlternatingRowColors(enable);
    QMapIterator<QTreeWidgetItem *, QtBrowserItem *> it(d_ptr->m_itemToIndex);
}

/*!
  \property QtTreePropertyBrowser::headerVisible
  \brief whether to show the header.
*/
bool QtTreePropertyBrowser::isHeaderVisible() const
{
    return d_ptr->m_headerVisible;
}

void QtTreePropertyBrowser::setHeaderVisible(bool visible)
{
    if (d_ptr->m_headerVisible == visible)
        return;

    d_ptr->m_headerVisible = visible;
    d_ptr->m_treeWidget->header()->setVisible(visible);
}

/*!
  \enum QtTreePropertyBrowser::ResizeMode

  The resize mode specifies the behavior of the header sections.

  \value Interactive The user can resize the sections.
  The sections can also be resized programmatically using setSplitterPosition().

  \value Fixed The user cannot resize the section.
  The section can only be resized programmatically using setSplitterPosition().

  \value Stretch QHeaderView will automatically resize the section to fill the available space.
  The size cannot be changed by the user or programmatically.

  \value ResizeToContents QHeaderView will automatically resize the section to its optimal
  size based on the contents of the entire column.
  The size cannot be changed by the user or programmatically.

  \sa setResizeMode()
*/

/*!
    \property QtTreePropertyBrowser::resizeMode
    \brief the resize mode of setions in the header.
*/

QtTreePropertyBrowser::ResizeMode QtTreePropertyBrowser::resizeMode() const
{
    return d_ptr->m_resizeMode;
}

void QtTreePropertyBrowser::setResizeMode(QtTreePropertyBrowser::ResizeMode mode)
{
    if (d_ptr->m_resizeMode == mode)
        return;

    d_ptr->m_resizeMode = mode;
    QHeaderView::ResizeMode m = QHeaderView::Stretch;
    switch (mode) {
        case QtTreePropertyBrowser::Interactive:      m = QHeaderView::Interactive;      break;
        case QtTreePropertyBrowser::Fixed:            m = QHeaderView::Fixed;            break;
        case QtTreePropertyBrowser::ResizeToContents: m = QHeaderView::ResizeToContents; break;
        case QtTreePropertyBrowser::Stretch:
        default:                                      m = QHeaderView::Stretch;          break;
    }

#if QT_VERSION >= 0x050000
    d_ptr->m_treeWidget->header()->setSectionResizeMode(m);
#else
    d_ptr->m_treeWidget->header()->setResizeMode(m);
#endif
}

/*!
    \property QtTreePropertyBrowser::splitterPosition
    \brief the position of the splitter between the colunms.
*/

int QtTreePropertyBrowser::splitterPosition() const
{
    return d_ptr->m_treeWidget->header()->sectionSize(0);
}

void QtTreePropertyBrowser::setSplitterPosition(int position)
{
    d_ptr->m_treeWidget->header()->resizeSection(0, position);
}

/*!
    Sets the \a item to either collapse or expanded, depending on the value of \a expanded.

    \sa isExpanded(), expanded(), collapsed()
*/

void QtTreePropertyBrowser::setExpanded(QtBrowserItem *item, bool expanded)
{
    QTreeWidgetItem *treeItem = d_ptr->m_indexToItem.value(item);
    if (treeItem)
        treeItem->setExpanded(expanded);
}

/*!
    Returns true if the \a item is expanded; otherwise returns false.

    \sa setExpanded()
*/

bool QtTreePropertyBrowser::isExpanded(QtBrowserItem *item) const
{
    QTreeWidgetItem *treeItem = d_ptr->m_indexToItem.value(item);
    if (treeItem)
        return treeItem->isExpanded();
    return false;
}

/*!
    Returns true if the \a item is visible; otherwise returns false.

    \sa setItemVisible()
    \since 4.5
*/

bool QtTreePropertyBrowser::isItemVisible(QtBrowserItem *item) const
{
    if (const QTreeWidgetItem *treeItem = d_ptr->m_indexToItem.value(item))
        return !treeItem->isHidden();
    return false;
}

/*!
    Sets the \a item to be visible, depending on the value of \a visible.

   \sa isItemVisible()
   \since 4.5
*/

void QtTreePropertyBrowser::setItemVisible(QtBrowserItem *item, bool visible)
{
    if (QTreeWidgetItem *treeItem = d_ptr->m_indexToItem.value(item))
        treeItem->setHidden(!visible);
}

/*!
    Sets the \a item's background color to \a color. Note that while item's background
    is rendered every second row is being drawn with alternate color (which is a bit lighter than items \a color)

    \sa backgroundColor(), calculatedBackgroundColor()
*/

void QtTreePropertyBrowser::setBackgroundColor(QtBrowserItem *item, const QColor &color)
{
    if (!d_ptr->m_indexToItem.contains(item))
        return;
    if (color.isValid())
        d_ptr->m_indexToBackgroundColor[item] = color;
    else
        d_ptr->m_indexToBackgroundColor.remove(item);
    d_ptr->m_treeWidget->viewport()->update();
}

/*!
    Returns the \a item's color. If there is no color set for item it returns invalid color.

    \sa calculatedBackgroundColor(), setBackgroundColor()
*/

QColor QtTreePropertyBrowser::backgroundColor(QtBrowserItem *item) const
{
    return d_ptr->m_indexToBackgroundColor.value(item);
}

/*!
    Returns the \a item's color. If there is no color set for item it returns parent \a item's
    color (if there is no color set for parent it returns grandparent's color and so on). In case
    the color is not set for \a item and it's top level item it returns invalid color.

    \sa backgroundColor(), setBackgroundColor()
*/

QColor QtTreePropertyBrowser::calculatedBackgroundColor(QtBrowserItem *item) const
{
    return d_ptr->calculatedBackgroundColor(item);
}

/*!
    \property QtTreePropertyBrowser::propertiesWithoutValueMarked
    \brief whether to enable or disable marking properties without value.

    When marking is enabled the item's background is rendered in dark color and item's
    foreground is rendered with light color.

    \sa propertiesWithoutValueMarked()
*/
void QtTreePropertyBrowser::setPropertiesWithoutValueMarked(bool mark)
{
    if (d_ptr->m_markPropertiesWithoutValue == mark)
        return;

    d_ptr->m_markPropertiesWithoutValue = mark;
    QMapIterator<QTreeWidgetItem *, QtBrowserItem *> it(d_ptr->m_itemToIndex);
    while (it.hasNext()) {
        QtProperty *property = it.next().value()->property();
        if (!property->hasValue())
            d_ptr->updateItem(it.key());
    }
    d_ptr->m_treeWidget->viewport()->update();
}

bool QtTreePropertyBrowser::propertiesWithoutValueMarked() const
{
    return d_ptr->m_markPropertiesWithoutValue;
}

/*!
    \reimp
*/
void QtTreePropertyBrowser::itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem)
{
    d_ptr->propertyInserted(item, afterItem);
}

/*!
    \reimp
*/
void QtTreePropertyBrowser::itemRemoved(QtBrowserItem *item)
{
    d_ptr->propertyRemoved(item);
}

/*!
    \reimp
*/
void QtTreePropertyBrowser::itemChanged(QtBrowserItem *item)
{
    d_ptr->propertyChanged(item);
}

/*!
    Sets the current item to \a item and opens the relevant editor for it.
*/
void QtTreePropertyBrowser::editItem(QtBrowserItem *item)
{
    d_ptr->editItem(item);
}

#if QT_VERSION >= 0x040400
QT_END_NAMESPACE
#endif

#include "moc_qttreepropertybrowser.cpp"
#include "qttreepropertybrowser.moc"
