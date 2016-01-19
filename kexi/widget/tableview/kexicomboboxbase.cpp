/* This file is part of the KDE project
   Copyright (C) 2002 Peter Simonsson <psn@linux.se>
   Copyright (C) 2003-2016 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QApplication>
#include <QDesktopWidget>
#include <QLayout>
#include <QStyle>
#include <QWindowsStyle>
#include <QPainter>
#include <QScrollBar>

#include <kexi_global.h>
#include "kexicomboboxbase.h"
#include <widget/utils/kexicomboboxdropdownbutton.h>
#include "kexicomboboxpopup.h"
#include "KexiTableScrollArea.h"
#include "KexiTableScrollAreaWidget.h"
#include "kexi.h"

KexiComboBoxBase::KexiComboBoxBase()
{
    m_internalEditorValueChanged = false; //user has text or other value inside editor
    m_slotInternalEditorValueChanged_enabled = true;
    m_mouseBtnPressedWhenPopupVisible = false;
    m_insideCreatePopup = false;
    m_setValueOrTextInInternalEditor_enabled = true;
    m_updatePopupSelectionOnShow = true;
    m_moveCursorToEndInInternalEditor_enabled = true;
    m_selectAllInInternalEditor_enabled = true;
    m_setValueInInternalEditor_enabled = true;
    m_setVisibleValueOnSetValueInternal = false;
    m_reinstantiatePopupOnShow = false;
    m_focusPopupBeforeShow = false;
}

KexiComboBoxBase::~KexiComboBoxBase()
{
}

KexiDB::LookupFieldSchema *KexiComboBoxBase::lookupFieldSchema() const
{
    if (field() && field()->table()) {
        KexiDB::LookupFieldSchema *lookupFieldSchema = field()->table()->lookupFieldSchema(*field());
        if (lookupFieldSchema && !lookupFieldSchema->rowSource().name().isEmpty())
            return lookupFieldSchema;
    }
    return 0;
}

int KexiComboBoxBase::rowToHighlightForLookupTable() const
{
    if (!popup())
        return -1;//err
    KexiDB::LookupFieldSchema *lookupFieldSchema = this->lookupFieldSchema();
    if (!lookupFieldSchema)
        return -1;
    if (lookupFieldSchema->boundColumn() == -1)
        return -1; //err
    bool ok;
    const int rowUid = origValue().toInt();
//! @todo for now we're assuming the id is INTEGER
    KexiDB::TableViewData *tvData = popup()->tableView()->data();
    const int boundColumn = boundColumnIndex();
    int row = -1;
    for (KexiDB::TableViewData::Iterator it(tvData->constBegin()); it != tvData->constEnd(); ++it) {
        row++;
        KexiDB::RecordData* record = *it;
        if (record->at(boundColumn).toInt(&ok) == rowUid && ok)
            return row;
        if (!ok)
            break;
    }
    //item not found: highlight 1st row, if available
    return -1;
}

void KexiComboBoxBase::setValueInternal(const QVariant& add_, bool removeOld)
{
    Q_UNUSED(removeOld);
    m_mouseBtnPressedWhenPopupVisible = false;
    m_updatePopupSelectionOnShow = true;
    QString add(add_.toString());
    if (add.isEmpty()) {
        KexiDB::TableViewData *relData = column() ? column()->relatedData() : 0;
        QVariant valueToSet;
        bool hasValueToSet = true;
        int rowToHighlight = -1;
        KexiDB::LookupFieldSchema *lookupFieldSchema = this->lookupFieldSchema();
        if (lookupFieldSchema) {
            //use 'lookup field' model
//! @todo support more RowSourceType's, not only table
            if (lookupFieldSchema->boundColumn() == -1)
//! @todo errmsg
                return;
            if (m_setVisibleValueOnSetValueInternal) {
                //only for table views
                if (!popup())
                    createPopup(false/*!show*/);
            }
            if (popup()) {
                const int rowToHighlight = rowToHighlightForLookupTable();
                popup()->tableView()->setHighlightedRow(rowToHighlight);

                const int visibleColumn = visibleColumnIndex();
                if (m_setVisibleValueOnSetValueInternal && -1 != visibleColumn) {
                    //only for table views
                    KexiDB::RecordData *record = popup()->tableView()->highlightedItem();
                    if (record)
                        valueToSet = record->at(visibleColumn);
                } else {
                    hasValueToSet = false;
                }
            }
        } else if (relData) {
            //use 'related table data' model
            valueToSet = valueForString(origValue().toString(), &rowToHighlight, 0, 1);
        } else {
            //use 'enum hints' model
            const int row = origValue().toInt();
            valueToSet = field()->enumHint(row).trimmed();
        }
        if (hasValueToSet)
            setValueOrTextInInternalEditor(valueToSet);
        /*impl.*/moveCursorToEndInInternalEditor();
        /*impl.*/selectAllInInternalEditor();

        if (popup()) {
            if (origValue().isNull()) {
                popup()->tableView()->clearSelection();
                popup()->tableView()->setHighlightedRow(0);
            } else {
                if (relData) {
                    if (rowToHighlight != -1)
                        popup()->tableView()->setHighlightedRow(rowToHighlight);
                } else if (!lookupFieldSchema) {
                    //popup()->tableView()->selectRow(origValue().toInt());
                    popup()->tableView()->setHighlightedRow(origValue().toInt());
                }
            }
        }
    } else {
        //! @todo autocompl.?
        if (popup())
            popup()->tableView()->clearSelection();
        /*impl.*/setValueInInternalEditor(add); //not setLineEditText(), because 'add' is entered by user!
        //setLineEditText( add );
        /*impl.*/moveCursorToEndInInternalEditor();
    }
}

KexiDB::RecordData* KexiComboBoxBase::selectItemForEnteredValueInLookupTable(const QVariant& v)
{
    KexiDB::LookupFieldSchema *lookupFieldSchema = this->lookupFieldSchema();
    if (!popup() || !lookupFieldSchema)
        return 0; //safety
//-not effective for large sets: please cache it!
//.trimmed() is not generic!

    const bool valueIsText = v.type() == QVariant::String || v.type() == QVariant::ByteArray; //most common case
    const QString txt(valueIsText ? v.toString().trimmed() : QString());
    KexiDB::TableViewData *lookupData = popup()->tableView()->data();
    const int visibleColumn = visibleColumnIndex();
    if (-1 == visibleColumn)
        return 0;
    KexiDB::TableViewData::Iterator it(lookupData->constBegin());
    int row;
    for (row = 0;it != lookupData->constEnd();++it, row++) {
        if (valueIsText) {
            if ((*it)->at(visibleColumn).toString().trimmed().compare(txt, Qt::CaseInsensitive) == 0)
                break;
        } else {
            if ((*it)->at(visibleColumn) == v)
                break;
        }
    }

    m_setValueOrTextInInternalEditor_enabled = false; // <-- this is the entered value,
    //     so do not change the internal editor's contents
    if (it != lookupData->constEnd())
        popup()->tableView()->selectRow(row);
    else
        popup()->tableView()->clearSelection();

    m_setValueOrTextInInternalEditor_enabled = true;

    return it != lookupData->constEnd() ? *it : 0;
}

QString KexiComboBoxBase::valueForString(const QString& str, int* row,
        uint lookInColumn, uint returnFromColumn, bool allowNulls)
{
    Q_UNUSED(returnFromColumn);
    KexiDB::TableViewData *relData = column() ? column()->relatedData() : 0;
    if (!relData)
        return QString(); //safety
    //use 'related table data' model
    //-not effective for large sets: please cache it!
    //.trimmed() is not generic!

    const QString txt(str.trimmed());
    KexiDB::TableViewData::Iterator it(relData->constBegin());
    for (*row = 0;it != relData->constEnd();++it, (*row)++) {
        const QString s((*it)->at(lookInColumn).toString());
        if (s.trimmed().compare(txt, Qt::CaseInsensitive) == 0)
            return s;
    }

    *row = -1;

    if (column() && column()->isRelatedDataEditable())
        return str; //new value entered and that's allowed

    kWarning() << "no related record found, ID will be painted!";
    if (allowNulls)
        return QString();
    return str; //for sanity but it's weird to show id to the user
}

int KexiComboBoxBase::boundColumnIndex() const
{
    if (!lookupFieldSchema()) {
        return -1;
    }
    switch (lookupFieldSchema()->rowSource().type()) {
    case KexiDB::LookupFieldSchema::RowSource::Table:
        // When the row source is Table we have hardcoded columns: <visible>, <bound>
        return lookupFieldSchema()->visibleColumns().count();
    default:;
    }
    // When the row source is Query we use the lookup field's bound column index
    //! @todo Implement for other types
    return lookupFieldSchema()->boundColumn();
}

int KexiComboBoxBase::visibleColumnIndex() const
{
    if (!lookupFieldSchema() || lookupFieldSchema()->visibleColumns().isEmpty()) {
        return -1;
    }
    switch (lookupFieldSchema()->rowSource().type()) {
    case KexiDB::LookupFieldSchema::RowSource::Table:
        // When the row source is Table we have hardcoded columns: <visible>, <bound>
        return lookupFieldSchema()->visibleColumn(0);
    default:;
    }
    // When the row source is Query we use the lookup field's visible column index
    //! @todo Implement for multiple visible columns
    //! @todo Implement for other types
    return lookupFieldSchema()->visibleColumns().first();
}

QVariant KexiComboBoxBase::value()
{
    KexiDB::TableViewData *relData = column() ? column()->relatedData() : 0;
    KexiDB::LookupFieldSchema *lookupFieldSchema = 0;
    if (relData) {
        if (m_internalEditorValueChanged) {
            //we've user-entered text: look for id
//! @todo make error if matching text not found?
            int rowToHighlight;
            return valueForString(m_userEnteredValue.toString(), &rowToHighlight, 1, 0, true/*allowNulls*/);
        } else {
            //use 'related table data' model
            KexiDB::RecordData *record = popup() ? popup()->tableView()->selectedItem() : 0;
            return record ? record->at(0) : origValue();
        }
    } else if ((lookupFieldSchema = this->lookupFieldSchema())) {
        if (lookupFieldSchema->boundColumn() == -1)
            return origValue();
        KexiDB::RecordData *record = popup() ? popup()->tableView()->selectedItem() : 0;
        if (/*!record &&*/ m_internalEditorValueChanged && !m_userEnteredValue.toString().isEmpty()) { //
            //try to select a row using the user-entered text
            if (!popup()) {
                QVariant prevUserEnteredValue = m_userEnteredValue;
                createPopup(false);
                m_userEnteredValue = prevUserEnteredValue;
            }
            record = selectItemForEnteredValueInLookupTable(m_userEnteredValue);
        }
        return record ? record->at(boundColumnIndex()) : QVariant();
    } else if (popup()) {
        //use 'enum hints' model
        const int row = popup()->tableView()->currentRow();
        if (row >= 0)
            return QVariant(row);
    }

    if (valueFromInternalEditor().toString().isEmpty())
        return QVariant();
    /*! \todo don't return just 1st row, but use autocompletion feature
          and: show message box if entered text does not match! */
    return origValue(); //unchanged
}

QVariant KexiComboBoxBase::visibleValueForLookupField()
{
    KexiDB::LookupFieldSchema *lookupFieldSchema = this->lookupFieldSchema();
    if (!popup() || !lookupFieldSchema)
        return QVariant();
    const int visibleColumn = visibleColumnIndex();
    //kDebug() << "visibleColumn" << visibleColumn;
    if (-1 == visibleColumn)
        return QVariant();
    KexiDB::RecordData *record = popup()->tableView()->selectedItem();
    return record ? record->at(qMin(visibleColumn, record->count() - 1)/*sanity*/) : QVariant();
}

QVariant KexiComboBoxBase::visibleValue()
{
    return m_visibleValue;
}

void KexiComboBoxBase::clear()
{
    if (popup())
        popup()->hide();
    slotInternalEditorValueChanged(QVariant());
}

tristate KexiComboBoxBase::valueChangedInternal()
{
    //avoid comparing values:
    KexiDB::TableViewData *relData = column() ? column()->relatedData() : 0;
    KexiDB::LookupFieldSchema *lookupFieldSchema = this->lookupFieldSchema();
    if (relData || lookupFieldSchema) {
        if (m_internalEditorValueChanged)
            return true;

        //use 'related table data' model
        KexiDB::RecordData *record = popup() ? popup()->tableView()->selectedItem() : 0;
        if (!record)
            return false;
    } else {
        //use 'enum hints' model
        const int row = popup() ? popup()->tableView()->currentRow() : -1;
        if (row < 0 && !m_internalEditorValueChanged/*true if text box is cleared*/)
            return false;
    }

    return cancelled;
}

bool KexiComboBoxBase::valueIsNull()
{
// bool ok;
    QVariant v(value());
    return v.isNull();
// return !ok || v.isNull();
}

bool KexiComboBoxBase::valueIsEmpty()
{
    return valueIsNull();
}

void KexiComboBoxBase::showPopup()
{
    //kDebug(44010);
    createPopup(true);
}

void KexiComboBoxBase::createPopup(bool show)
{
    //kDebug() << show << field() << popup() << m_updatePopupSelectionOnShow;
    if (!field())
        return;
    m_insideCreatePopup = true;
    QWidget* thisWidget = dynamic_cast<QWidget*>(this);
    QWidget *widgetToFocus = internalEditor() ? internalEditor() : thisWidget;
    //kDebug() << "widgetToFocus:" << widgetToFocus;

    if (m_reinstantiatePopupOnShow) {
        QWidget *oldPopup = popup();
        setPopup(0);
        delete oldPopup;
    }

    if (!popup()) {
        setPopup(column() ? new KexiComboBoxPopup(thisWidget, *column())
                 : new KexiComboBoxPopup(thisWidget, *field()));
        QObject::connect(popup(), SIGNAL(rowAccepted(KexiDB::RecordData*,int)),
                         thisWidget, SLOT(slotRowAccepted(KexiDB::RecordData*,int)));
        QObject::connect(popup()->tableView(), SIGNAL(itemSelected(KexiDB::RecordData*)),
                         thisWidget, SLOT(slotItemSelected(KexiDB::RecordData*)));

        popup()->setFocusProxy(widgetToFocus);
        popup()->tableView()->setFocusProxy(widgetToFocus);
        popup()->installEventFilter(thisWidget);

        if (origValue().isNull())
            popup()->tableView()->clearSelection();
        else {
            popup()->tableView()->selectRow(0);
            popup()->tableView()->setHighlightedRow(0);
        }
    }
    if (show && internalEditor() && !internalEditor()->isVisible())
        /*emit*/editRequested();

    QPoint posMappedToGlobal = mapFromParentToGlobal(thisWidget->pos());
    if (posMappedToGlobal != QPoint(-1, -1)) {
//! todo alter the position to fit the popup within screen boundaries
        QPoint pos = posMappedToGlobal + QPoint(0, thisWidget->height());
        if (qobject_cast<KexiTableScrollAreaWidget*>(thisWidget->parentWidget())) {
            KexiTableScrollArea* tableScroll = qobject_cast<KexiTableScrollAreaWidget*>(thisWidget->parentWidget())->scrollArea;
            pos -= QPoint(tableScroll->horizontalScrollBar()->value(),
                          tableScroll->verticalScrollBar()->value());
        }
        popup()->hide();
        popup()->move(pos);
        //kDebug() << "pos:" << posMappedToGlobal + QPoint(0, thisWidget->height());
        //to avoid flickering: first resize to 0-height, then show and resize back to prev. height
        int w = popupWidthHint();
        popup()->resize(w, 0);
        if (show) {
            popup()->show();
            //kDebug(44010) << "SHOW!!!";
        }
        popup()->updateSize(w);

        // make sure the popup fits on the screen
        const QRect screen = QApplication::desktop()->availableGeometry(posMappedToGlobal);
        pos -= screen.topLeft(); // to simplify computation
        w = popup()->width();
        int h = popup()->height();
        if (screen.width() < w) {
            w = screen.width();
            pos.setX(0);
        } else if (screen.width() < (pos.x() + w - 1)) {
            pos.setX(screen.width() - w + 1);
        } else if (pos.x() < 0) {
            pos.setX(0);
        }
        if (screen.height() < h) {
            h = screen.height();
            pos.setY(0);
        } else if (screen.height() < (pos.y() + h - 1)) {
            const int topY = pos.y() - thisWidget->height() - h;
            if (topY >= 0 && (topY + h - 1 < screen.height())) {
                pos.setY(pos.y() - thisWidget->height() - h);
            } else {
                pos.setY(screen.height() - h + 1);
            }
        } else if (pos.y() < 0) {
            pos.setY(0);
        }
        popup()->move(pos + screen.topLeft());
        popup()->resize(w, h);

        if (m_updatePopupSelectionOnShow) {
            int rowToHighlight = -1;
            KexiDB::LookupFieldSchema *lookupFieldSchema = this->lookupFieldSchema();
            KexiDB::TableViewData *relData = column() ? column()->relatedData() : 0;
            if (lookupFieldSchema) {
                rowToHighlight = rowToHighlightForLookupTable();
            } else if (relData) {
                (void)valueForString(origValue().toString(), &rowToHighlight, 0, 1);
            } else //enum hint
                rowToHighlight = origValue().toInt();

            /*-->*/ m_moveCursorToEndInInternalEditor_enabled = show;
            m_selectAllInInternalEditor_enabled = show;
            m_setValueInInternalEditor_enabled = show;
            if (rowToHighlight == -1) {
                rowToHighlight = qMax(popup()->tableView()->highlightedRow(), 0);
                setValueInInternalEditor(QVariant());
            }
            popup()->tableView()->selectRow(rowToHighlight);
            popup()->tableView()->setHighlightedRow(rowToHighlight);
            popup()->tableView()->ensureCellVisible(-1, 0); // scroll to left as expected

            /*-->*/ m_moveCursorToEndInInternalEditor_enabled = true;
            m_selectAllInInternalEditor_enabled = true;
            m_setValueInInternalEditor_enabled = true;
        }
    }

    if (show) {
        moveCursorToEndInInternalEditor();
        selectAllInInternalEditor();
        if (m_focusPopupBeforeShow) {
            widgetToFocus->setFocus();
        }
        popup()->show();
        popup()->raise();
        popup()->repaint();
        if (!m_focusPopupBeforeShow) {
            widgetToFocus->setFocus();
        }
    }
    m_insideCreatePopup = false;
}

void KexiComboBoxBase::hide()
{
    if (popup())
        popup()->hide();
}

void KexiComboBoxBase::slotRowAccepted(KexiDB::RecordData* record, int row)
{
    Q_UNUSED(row);
    //update our value
    //..nothing to do?
    updateButton();
    slotItemSelected(record);
    /*emit*/acceptRequested();
}

void KexiComboBoxBase::acceptPopupSelection()
{
    if (!popup())
        return;
    KexiDB::RecordData *record = popup()->tableView()->highlightedItem();
    if (record) {
        popup()->tableView()->selectRow(popup()->tableView()->highlightedRow());
        slotRowAccepted(record, -1);
    }
    popup()->hide();
}

void KexiComboBoxBase::slotItemSelected(KexiDB::RecordData*)
{
    //kDebug(44010) << "m_visibleValue=" << m_visibleValue;

    QVariant valueToSet;
    KexiDB::TableViewData *relData = column() ? column()->relatedData() : 0;
    KexiDB::LookupFieldSchema *lookupFieldSchema = this->lookupFieldSchema();

    m_visibleValue = lookupFieldSchema ? visibleValueForLookupField() : QVariant();

    if (relData) {
        //use 'related table data' model
        KexiDB::RecordData *record = popup()->tableView()->selectedItem();
        if (record)
            valueToSet = record->at(1);
    } else if (lookupFieldSchema) {
        KexiDB::RecordData *record = popup()->tableView()->selectedItem();
        const int visibleColumn = visibleColumnIndex();
        if (record && visibleColumn != -1 /* && (int)item->size() >= visibleColumn --already checked*/) {
            valueToSet = record->at(qMin(visibleColumn, record->count() - 1)/*sanity*/);
            popup()->tableView()->ensureCellVisible(popup()->tableView()->highlightedRow(), -1);
        }
    } else {
        //use 'enum hints' model
        valueToSet = field()->enumHint(popup()->tableView()->currentRow());
        if (valueToSet.toString().isEmpty() && !m_insideCreatePopup) {
            clear();
            QWidget* thisWidget = dynamic_cast<QWidget*>(this);
            thisWidget->parentWidget()->setFocus();
            return;
        }
    }
    setValueOrTextInInternalEditor(valueToSet);
    QWidget* thisWidget = dynamic_cast<QWidget*>(this);
    thisWidget->setFocus();
    if (m_setValueOrTextInInternalEditor_enabled) {
        moveCursorToEndInInternalEditor();
        selectAllInInternalEditor();
    }
    // a new (temp) popup table index is selected: do not update selection next time:
    m_updatePopupSelectionOnShow = false;
}

void KexiComboBoxBase::slotInternalEditorValueChanged(const QVariant& v)
{
    if (!m_slotInternalEditorValueChanged_enabled)
        return;
    m_userEnteredValue = v;
    m_internalEditorValueChanged = true;
    if (v.toString().isEmpty()) {
        if (popup()) {
            popup()->tableView()->clearSelection();
        }
        return;
    }
}

void KexiComboBoxBase::setValueOrTextInInternalEditor(const QVariant& value)
{
    if (!m_setValueOrTextInInternalEditor_enabled)
        return;
    setValueInInternalEditor(value);
    //this text is not entered by hand:
    m_userEnteredValue = QVariant();
    m_internalEditorValueChanged = false;
}

bool KexiComboBoxBase::handleKeyPressForPopup(QKeyEvent *ke)
{
    const int k = ke->key();
    int highlightedOrSelectedRow = popup() ? popup()->tableView()->highlightedRow() : -1;
    if (popup() && highlightedOrSelectedRow < 0)
        highlightedOrSelectedRow = popup()->tableView()->currentRow();

    const bool enterPressed = k == Qt::Key_Enter || k == Qt::Key_Return;

    // The editor may be active but the pull down menu not existent/visible,
    // e.g. when the user has pressed a normal button to activate the editor
    // Don't handle the event here in that case.
    if (!popup() || (!enterPressed && !popup()->isVisible())) {
        return false;
    }

    switch (k) {
    case Qt::Key_Up:
        popup()->tableView()->setHighlightedRow(
            qMax(highlightedOrSelectedRow - 1, 0));
        updateTextForHighlightedRecord();
        return true;
    case Qt::Key_Down:
        popup()->tableView()->setHighlightedRow(
            qMin(highlightedOrSelectedRow + 1, popup()->tableView()->rowCount() - 1));
        updateTextForHighlightedRecord();
        return true;
    case Qt::Key_PageUp:
        popup()->tableView()->setHighlightedRow(
            qMax(highlightedOrSelectedRow - popup()->tableView()->rowsPerPage(), 0));
        updateTextForHighlightedRecord();
        return true;
    case Qt::Key_PageDown:
        popup()->tableView()->setHighlightedRow(
            qMin(highlightedOrSelectedRow + popup()->tableView()->rowsPerPage(),
                 popup()->tableView()->rowCount() - 1));
        updateTextForHighlightedRecord();
        return true;
    case Qt::Key_Home:
        popup()->tableView()->setHighlightedRow(0);
        updateTextForHighlightedRecord();
        return true;
    case Qt::Key_End:
        popup()->tableView()->setHighlightedRow(popup()->tableView()->rowCount() - 1);
        updateTextForHighlightedRecord();
        return true;
    case Qt::Key_Enter:
    case Qt::Key_Return: //accept
        //select row that is highlighted
        if (popup()->tableView()->highlightedRow() >= 0) {
            popup()->tableView()->selectRow(popup()->tableView()->highlightedRow());
            acceptPopupSelection();
            return true;
        }
    default: ;
    }
    return false;
}

void KexiComboBoxBase::updateTextForHighlightedRecord()
{
    KexiDB::RecordData* record = popup() ? popup()->tableView()->highlightedItem() : 0;
    if (record)
        slotItemSelected(record);
}

void KexiComboBoxBase::undoChanges()
{
    KexiDB::LookupFieldSchema *lookupFieldSchema = this->lookupFieldSchema();
    if (lookupFieldSchema) {
//  kDebug() << "m_visibleValue BEFORE=" << m_visibleValue;
        if (popup())
            popup()->tableView()->selectRow(popup()->tableView()->highlightedRow());
        m_visibleValue = visibleValueForLookupField();
//  kDebug() << "m_visibleValue AFTER=" << m_visibleValue;
        setValueOrTextInInternalEditor(m_visibleValue);
    }
}
