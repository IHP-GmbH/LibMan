/************************************************************************
 *  LibMan – Library & View Manager
 *
 *  Copyright (C) ...
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 ************************************************************************/

#include <QtTest/QtTest>

#include <QCoreApplication>
#include <QListWidget>
#include <QTreeWidget>

#include <functional>

#include "tst_libman_gui.h"

#include "mainwindow.h"

namespace
{

/*!*******************************************************************************************************************
 * \brief Finds a top-level item in a QTreeWidget by its display text.
 *
 * \param tree Tree widget to search in.
 * \param text Text to match (column 0).
 *
 * \return Pointer to the found item or nullptr if not found.
 **********************************************************************************************************************/
static QTreeWidgetItem* findTopItem(QTreeWidget* tree, const QString& text)
{
    if (!tree)
        return nullptr;

    for (int i = 0; i < tree->topLevelItemCount(); ++i) {
        auto* it = tree->topLevelItem(i);
        if (it && it->text(0) == text)
            return it;
    }

    return nullptr;
}

/*!*******************************************************************************************************************
 * \brief Finds an item in a QListWidget by its display text.
 *
 * \param list List widget to search in.
 * \param text Text to match.
 *
 * \return Pointer to the found item or nullptr if not found.
 **********************************************************************************************************************/
static QListWidgetItem* findListItem(QListWidget* list, const QString& text)
{
    if (!list)
        return nullptr;

    for (int i = 0; i < list->count(); ++i) {
        auto* it = list->item(i);
        if (it && it->text() == text)
            return it;
    }

    return nullptr;
}

/*!*******************************************************************************************************************
 * \brief Finds a direct child item of a QTreeWidgetItem by its display text.
 *
 * \param parent Parent item whose direct children will be searched.
 * \param text   Text to match (column 0).
 *
 * \return Pointer to the found child item or nullptr if not found.
 **********************************************************************************************************************/
static QTreeWidgetItem* findChildItemByText(QTreeWidgetItem* parent, const QString& text)
{
    if (!parent)
        return nullptr;

    for (int i = 0; i < parent->childCount(); ++i) {
        auto* ch = parent->child(i);
        if (ch && ch->text(0) == text)
            return ch;
    }

    return nullptr;
}

/*!*******************************************************************************************************************
 * \brief Waits until the given predicate becomes true or a timeout is reached.
 *
 * The function repeatedly evaluates the predicate, sleeping for \p stepMs between
 * evaluations while processing the event loop to allow asynchronous GUI updates.
 *
 * \param predicate Function returning true when the condition is satisfied.
 * \param timeoutMs Maximum time to wait in milliseconds.
 * \param stepMs    Poll interval in milliseconds.
 *
 * \return True if predicate became true within the timeout, false otherwise.
 **********************************************************************************************************************/
static bool waitUntil(std::function<bool()> predicate, int timeoutMs = 3000, int stepMs = 50)
{
    const int steps = qMax(1, timeoutMs / stepMs);

    for (int i = 0; i < steps; ++i) {
        if (predicate())
            return true;

        QTest::qWait(stepMs);
        QCoreApplication::processEvents();
    }

    return predicate();
}

} // namespace

/*!*******************************************************************************************************************
 * \brief Verifies that a GDS view is available and selectable after project loading.
 *
 * The test performs the following steps:
 *  - Loads a .projects file.
 *  - Selects the specified library in the Project tree.
 *  - Selects the corresponding cell/group.
 *  - Verifies that a "gds" view node appears in the View panel.
 *  - Expands the "gds" node and waits for asynchronous GDS parsing to populate cells.
 *  - Selects a specific GDS cell entry and verifies successful selection.
 *
 * This test validates correct interaction between:
 *  - Project loading logic
 *  - Library and cell navigation
 *  - Asynchronous GDS reader integration
 *  - View tree population and selection behavior
 *
 * No external tools are started during this test.
 **********************************************************************************************************************/
void LibManGui::loadProject_clickLib_clickCell_hasGdsView()
{
    const QString projPath = QFINDTESTDATA("data/sg13g2.projects");
    QVERIFY2(!projPath.isEmpty(), "test.projects not found via QFINDTESTDATA");

    const QString expectedLib     = "sg13g2_stdcell";
    const QString expectedCell    = "sg13g2_stdcell";
    const QString expectedGdsCell = "sg13g2_buf_1";

    MainWindow w(projPath, QCoreApplication::applicationDirPath());
    w.show();
    QVERIFY(QTest::qWaitForWindowExposed(&w));

    QTest::qWait(50);

    auto* treeLibs   = w.findChild<QTreeWidget*>("treeLibs");
    auto* listGroups = w.findChild<QListWidget*>("listGroups");
    auto* listViews  = w.findChild<QTreeWidget*>("listViews");

    QVERIFY2(treeLibs, "treeLibs widget not found");
    QVERIFY2(listGroups, "listGroups widget not found");
    QVERIFY2(listViews, "listViews widget not found");

    // ------------------------------------------------------------
    // Click library
    // ------------------------------------------------------------
    QTreeWidgetItem* libItem = findTopItem(treeLibs, expectedLib);
    QVERIFY2(libItem, qPrintable(QString("Library not found in treeLibs: %1").arg(expectedLib)));

    treeLibs->setCurrentItem(libItem);

    QMetaObject::invokeMethod(&w,
                              "on_treeLibs_itemClicked",
                              Qt::DirectConnection,
                              Q_ARG(QTreeWidgetItem*, libItem),
                              Q_ARG(int, 0));

    QTest::qWait(30);

    // ------------------------------------------------------------
    // Click cell/group
    // ------------------------------------------------------------
    QListWidgetItem* cellItem = findListItem(listGroups, expectedCell);
    QVERIFY2(cellItem, qPrintable(QString("Cell not found in listGroups: %1").arg(expectedCell)));

    listGroups->setCurrentItem(cellItem);

    QMetaObject::invokeMethod(&w,
                              "on_listGroups_itemClicked",
                              Qt::DirectConnection,
                              Q_ARG(QListWidgetItem*, cellItem));

    QTest::qWait(30);

    QTreeWidgetItem* gdsRoot = findTopItem(listViews, "gds");
    QVERIFY2(gdsRoot, "Expected 'gds' view root not found in listViews");

    // ------------------------------------------------------------
    // Expand "gds" and wait until async reader populates children
    // ------------------------------------------------------------
    gdsRoot->setExpanded(true);
    listViews->scrollToItem(gdsRoot);

    const bool populated = waitUntil([&]() {
        return gdsRoot->childCount() > 0;
    }, 5000, 100);
    QVERIFY2(populated, "GDS cell list was not populated (timeout)");

    QTreeWidgetItem* cellNode = findChildItemByText(gdsRoot, expectedGdsCell);

    if (!cellNode) {
        const bool foundLater = waitUntil([&]() {
            cellNode = findChildItemByText(gdsRoot, expectedGdsCell);
            return cellNode != nullptr;
        }, 5000, 100);

        QVERIFY2(foundLater,
                 qPrintable(QString("Expected GDS cell not found under 'gds': %1").arg(expectedGdsCell)));
    }

    listViews->setCurrentItem(cellNode);
    listViews->scrollToItem(cellNode);
    QVERIFY2(listViews->currentItem() == cellNode, "Failed to select expected GDS cell node");

    const QVariant gdsPathVar = gdsRoot->data(0, Qt::UserRole + 2 /*RoleGdsPath?*/);
    Q_UNUSED(gdsPathVar);
}

QTEST_MAIN(LibManGui)
