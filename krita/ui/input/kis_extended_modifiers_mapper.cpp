/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_extended_modifiers_mapper.h"

#include <QDebug>
#include <QApplication>

#ifdef Q_WS_X11

#include <QX11Info>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

struct KeyMapping {
    KeyMapping() {}
    KeyMapping(KeySym sym, Qt::Key key) : x11KeySym(sym), qtKey(key) {}
    KeySym x11KeySym;
    Qt::Key qtKey;
};

#endif /* Q_WS_X11 */

struct KisExtendedModifiersMapper::Private
{
    Private();

#ifdef Q_WS_X11

    QVector<KeyMapping> mapping;
    char keysState[32];

    bool checkKeyCodePressedX11(KeyCode key);
    bool checkKeySymPressedX11(KeySym sym);
#endif /* Q_WS_X11 */
};

#ifdef Q_WS_X11

KisExtendedModifiersMapper::Private::Private()
{
    XQueryKeymap(QX11Info::display(), keysState);

    mapping.append(KeyMapping(XK_Shift_L, Qt::Key_Shift));
    mapping.append(KeyMapping(XK_Shift_R, Qt::Key_Shift));

    mapping.append(KeyMapping(XK_Control_L, Qt::Key_Control));
    mapping.append(KeyMapping(XK_Control_R, Qt::Key_Control));

    mapping.append(KeyMapping(XK_Meta_L, Qt::Key_Meta));
    mapping.append(KeyMapping(XK_Meta_R, Qt::Key_Meta));

    mapping.append(KeyMapping(XK_Super_L, Qt::Key_Super_L));
    mapping.append(KeyMapping(XK_Super_R, Qt::Key_Super_R));

    mapping.append(KeyMapping(XK_Hyper_L, Qt::Key_Hyper_L));
    mapping.append(KeyMapping(XK_Hyper_R, Qt::Key_Hyper_R));


    mapping.append(KeyMapping(XK_space, Qt::Key_Space));

    for (int qtKey = Qt::Key_0, x11Sym = XK_0;
         qtKey <= Qt::Key_9;
         qtKey++, x11Sym++) {

        mapping.append(KeyMapping(x11Sym, Qt::Key(qtKey)));
    }

    for (int qtKey = Qt::Key_A, x11Sym = XK_a;
         qtKey <= Qt::Key_Z;
         qtKey++, x11Sym++) {

        mapping.append(KeyMapping(x11Sym, Qt::Key(qtKey)));
    }
}

bool KisExtendedModifiersMapper::Private::checkKeyCodePressedX11(KeyCode key)
{
    int byte = key / 8;
    char mask = 1 << (key % 8);

    return keysState[byte] & mask;
}

bool KisExtendedModifiersMapper::Private::checkKeySymPressedX11(KeySym sym)
{
    KeyCode key = XKeysymToKeycode(QX11Info::display(), sym);
    return key != 0 ? checkKeyCodePressedX11(key) : false;
}

#else /* Q_WS_X11 */

KisExtendedModifiersMapper::Private::Private()
{
}

#endif /* Q_WS_X11 */


KisExtendedModifiersMapper::KisExtendedModifiersMapper()
    : m_d(new Private)
{
}

KisExtendedModifiersMapper::~KisExtendedModifiersMapper()
{
}

KisExtendedModifiersMapper::ExtendedModifiers
KisExtendedModifiersMapper::queryExtendedModifiers()
{
    ExtendedModifiers modifiers;

#ifdef Q_WS_X11

    foreach (const KeyMapping &map, m_d->mapping) {
        if (m_d->checkKeySymPressedX11(map.x11KeySym)) {
            modifiers << map.qtKey;
        }
    }

#else /* Q_WS_X11 */

    Qt::KeyboardModifiers modifiers = queryStandardModifiers();

    if (modifiers & Qt::ShiftModifier) {
        modifiers << Qt::Key_Shift;
    }

    if (modifiers & Qt::ControlModifier) {
        modifiers << Qt::Key_Control;
    }

    if (modifiers & Qt::AltModifier) {
        modifiers << Qt::Key_Alt;
    }

    if (modifiers & Qt::MetaModifier) {
        modifiers << Qt::Key_Meta;
    }

#endif /* Q_WS_X11 */

    return modifiers;
}

Qt::KeyboardModifiers KisExtendedModifiersMapper::queryStandardModifiers()
{
#if QT_VERSION >= 0x040800
    return QApplication::queryKeyboardModifiers();
#else
    return QApplication::keyboardModifiers();
#endif
}
