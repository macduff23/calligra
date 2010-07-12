/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */

#include "kis_color_selector_component.h"

#include "kis_color_selector_base.h"

#include "KoColorSpace.h"
#include <QPainter>
#include <QMouseEvent>


KisColorSelectorComponent::KisColorSelectorComponent(KisColorSelectorBase* parent) :
    QObject(parent),
    m_parent(parent),
    m_param1(1),
    m_param2(2)
{
    Q_ASSERT(parent);
}

void KisColorSelectorComponent::setGeometry(int x, int y, int width, int height)
{
    m_x=x;
    m_y=y;
    m_width=width;
    m_height=height;
}

void KisColorSelectorComponent::paintEvent(QPainter* painter)
{
    painter->save();
    painter->translate(m_x, m_y);
    paint(painter);
    painter->restore();
}

void KisColorSelectorComponent::mouseEvent(int x, int y)
{
    int newX=x-m_x;
    int newY=y-m_y;
    if(newX>0 && newX<width() &&
       newY>0 && newY<height())
        selectColor(newX, newY);
}

const KoColorSpace* KisColorSelectorComponent::colorSpace() const
{
    const KoColorSpace* cs = m_parent->colorSpace();
    Q_ASSERT(cs);
    return cs;
}

QColor KisColorSelectorComponent::currentColor()
{
    return QColor();
}

void KisColorSelectorComponent::setParam(qreal p)
{
    m_param1 = p;
    emit update();
}

void KisColorSelectorComponent::setParam(qreal p1, qreal p2)
{
    m_param1 = p1;
    m_param2 = p2;
    emit update();
}

int KisColorSelectorComponent::width() const
{
    return m_width;
}

int KisColorSelectorComponent::height() const
{
    return m_height;
}

qreal KisColorSelectorComponent::parameter1() const
{
    return m_param1;
}

qreal KisColorSelectorComponent::parameter2() const
{
    return m_param2;
}
