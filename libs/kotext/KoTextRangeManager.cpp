/* This file is part of the KDE project
 * Copyright (c) 2012 Boudewijn Rempt <boud@kogmbh.com>
<<<<<<< HEAD
=======
 * Copyright (c) 2012 C. Boemann <cbo@boemann.dk>
>>>>>>> master
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
#include "KoTextRangeManager.h"
#include "KoTextDocument.h"
#include "KoBookmark.h"
#include "KoBookmarkManager.h"
<<<<<<< HEAD
#include "KoAnnotation.h"
#include "KoAnnotationManager.h"
#include "KoAnnotation.h"
#include "KoAnnotationManager.h"
=======
>>>>>>> master

#include <QTextCursor>

KoTextRangeManager::KoTextRangeManager(QObject *parent)
<<<<<<< HEAD
        : QObject(parent),
        m_lastObjectId(0)
=======
        : QObject(parent)
>>>>>>> master
{
}

KoTextRangeManager::~KoTextRangeManager()
{
}

void KoTextRangeManager::insert(KoTextRange *textRange)
{
    if (!textRange) {
        return;
    }

<<<<<<< HEAD
    int id = textRange->id();
    if (id == -1) {
        textRange->setId(++m_lastObjectId);
        textRange->setManager(this);
    }
    else {
        m_deletedTextRanges.remove(id);
=======

    if (m_textRanges.contains(textRange)) {
        return;
    }

    if (m_deletedTextRanges.contains(textRange)) {
        m_deletedTextRanges.remove(textRange);
    } else {
        textRange->setManager(this);
>>>>>>> master
    }

    KoBookmark *bookmark = dynamic_cast<KoBookmark *>(textRange);
    if (bookmark) {
        m_bookmarkManager.insert(bookmark->name(), bookmark);
    }
<<<<<<< HEAD
    else {
        KoAnnotation *annotation = dynamic_cast<KoAnnotation *>(textRange);
        if (annotation) {
            m_annotationManager.insert(annotation->name(), annotation);
        }
    }
    m_textRanges.insert(textRange->id(), textRange);
=======
    m_textRanges.insert(textRange);
>>>>>>> master
}

void KoTextRangeManager::remove(KoTextRange *textRange)
{
    if (!textRange) {
        return;
    }

    KoBookmark *bookmark = dynamic_cast<KoBookmark *>(textRange);
    if (bookmark) {
        m_bookmarkManager.remove(bookmark->name());
    }
<<<<<<< HEAD
    else {
        KoAnnotation *annotation = dynamic_cast<KoAnnotation *>(textRange);
        if (annotation) {
            m_annotationManager.remove(annotation->name());
        }
    }

    int id = textRange->id();
    m_textRanges.remove(id);
    m_deletedTextRanges[id] = textRange;
=======

    m_textRanges.remove(textRange);
    m_deletedTextRanges.insert(textRange);
>>>>>>> master
}

const KoBookmarkManager *KoTextRangeManager::bookmarkManager() const
{
    return &m_bookmarkManager;
}

<<<<<<< HEAD
const KoAnnotationManager *KoTextRangeManager::annotationManager() const
{
    return &m_annotationManager;
}

=======
>>>>>>> master
QList<KoTextRange *> KoTextRangeManager::textRanges() const
{
    return m_textRanges.values();
}


QHash<int, KoTextRange *> KoTextRangeManager::textRangesChangingWithin(int first, int last, int matchFirst, int matchLast) const
{
    QHash<int, KoTextRange *> ranges;
    foreach (KoTextRange *range, m_textRanges) {
<<<<<<< HEAD
        if (!range->hasSelection()) {
            if (range->cursor().position() >= first && range->cursor().position() <= last) {
                ranges.insertMulti(range->cursor().position(), range);
            }
        } else {
            if (range->cursor().selectionStart() >= first && range->cursor().selectionStart() <= last) {
                if (matchLast == -1 || range->cursor().selectionEnd() <= matchLast) {
                    ranges.insertMulti(range->cursor().selectionStart(), range);
                }
            }
            if (range->cursor().selectionEnd() >= first && range->cursor().selectionEnd() <= last) {
                if (matchFirst == -1 || range->cursor().selectionStart() >= matchFirst) {
                    ranges.insertMulti(range->cursor().selectionEnd(), range);
=======
        if (!range->hasRange()) {
            if (range->rangeStart() >= first && range->rangeStart() <= last) {
                ranges.insertMulti(range->rangeStart(), range);
            }
        } else {
            if (range->rangeStart() >= first && range->rangeStart() <= last) {
                if (matchLast == -1 || range->rangeEnd() <= matchLast) {
                    ranges.insertMulti(range->rangeStart(), range);
                }
            }
            if (range->rangeEnd() >= first && range->rangeEnd() <= last) {
                if (matchFirst == -1 || range->rangeStart() >= matchFirst) {
                    ranges.insertMulti(range->rangeEnd(), range);
>>>>>>> master
                }
            }
        }
    }
    return ranges;
}

#include <KoTextRangeManager.moc>
