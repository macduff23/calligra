/* This file is part of the KDE project

   Copyright 2018 Dag Andersen <danders@get2net.dk>
   Copyright 2009 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2009 Inge Wallin    <inge@lysator.liu.se>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCHART_TABLE_EDITOR_DIALOG_H
#define KCHART_TABLE_EDITOR_DIALOG_H

#include <QDialog>

#include "ui_ChartTableEditor.h"

class QAbstractItemModel;
class QModelIndex;
class QAction;


namespace KoChart {

class ChartTableView;
class ChartProxyModel;

class TableEditorDialog : public QDialog, public Ui::ChartTableEditor
{
    Q_OBJECT

public:
    TableEditorDialog();
    ~TableEditorDialog();

    void init();
    void setProxyModel(ChartProxyModel *proxyModel);
    void setModel(QAbstractItemModel *model);

protected Q_SLOTS:
    void slotUpdateDialog();
    void slotInsertRowAbovePressed();
    void slotInsertRowBelowPressed();
    void slotInsertColumnLeftPressed();
    void slotInsertColumnRightPressed();
    void slotDeleteSelectionPressed();

    void deleteSelectedRowsOrColumns(Qt::Orientation orientation);
    void slotCurrentIndexChanged(const QModelIndex &index);
    void slotSelectionChanged();
    void slotDataSetsInRowsToggled(bool enabled);

private:
    ChartProxyModel *m_proxyModel;
    ChartTableView *const m_tableView;

    QAction *m_deleteSelectionAction;
    QAction *m_insertRowAboveAction;
    QAction *m_insertColumnLeftAction;
    QAction *m_insertRowBelowAction;
    QAction *m_insertColumnRightAction;
};

} // Namespace KoChart

#endif // KCHART_TABLE_EDITOR_DIALOG_H

