/* This file is part of the KDE project
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_tool_select_base.h"
#include "canvas/kis_canvas2.h"
#include "kis_selection_options.h"
#include <QVBoxLayout>

KisToolSelectBase::KisToolSelectBase(KoCanvasBase *canvas, const QCursor& cursor) : KisTool(canvas, cursor)
{
    m_optWidget = 0;
    m_selectAction = SELECTION_REPLACE;
    m_selectionMode = PIXEL_SELECTION;
}


QWidget* KisToolSelectBase::createOptionWidget()
{
    KisCanvas2* canvas = dynamic_cast<KisCanvas2*>(m_canvas);
    Q_ASSERT(canvas);
    m_optWidget = new KisSelectionOptions(canvas);
    Q_CHECK_PTR(m_optWidget);
    m_optWidget->setObjectName(toolId() + " option widget");
    m_optWidget->setWindowTitle(i18n("Selection"));

    connect(m_optWidget, SIGNAL(actionChanged(int)), this, SLOT(slotSetAction(int)));
    connect(m_optWidget, SIGNAL(modeChanged(int)), this, SLOT(slotSetSelectionMode(int)));


    QVBoxLayout * l = dynamic_cast<QVBoxLayout*>(m_optWidget->layout());
    Q_ASSERT(l);
    if (l) {
        l->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    }
    m_optWidget->setFixedHeight(m_optWidget->sizeHint().height());
    return m_optWidget;
}

QWidget* KisToolSelectBase::optionWidget()
{
    return m_optWidget;
}

void KisToolSelectBase::slotSetAction(int action)
{
    if (action >= SELECTION_REPLACE && action <= SELECTION_INTERSECT)
        m_selectAction = (selectionAction)action;
}

void KisToolSelectBase::slotSetSelectionMode(int mode)
{
    m_selectionMode = (selectionMode)mode;
}

void KisToolSelectBase::activate(bool tmp)
{
    KisTool::activate(tmp);

    if (!m_optWidget)
        return;

    m_optWidget->slotActivated();
}


#include "kis_tool_select_base.moc"

