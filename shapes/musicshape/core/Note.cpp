/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
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
#include "Note.h"

namespace MusicCore {

class Note::Private {
public:
    Chord* chord;
    Staff* staff;
    int pitch;
    int accidentals;
};

Note::Note(Chord* chord, Staff* staff, int pitch, int accidentals) : d(new Private)
{
    d->chord = chord;
    d->staff = staff;
    d->pitch = pitch;
    d->accidentals = accidentals;
}

Note::~Note()
{
    delete d;
}

Chord* Note::chord()
{
    return d->chord;
}

Staff* Note::staff()
{
    return d->staff;
}

int Note::pitch() const
{
    return d->pitch;
}

int Note::accidentals() const
{
    return d->accidentals;
}

} // namespace MusicCore
