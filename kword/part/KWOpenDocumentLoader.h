/* This file is part of the KDE project
 * Copyright (C) 2005 David Faure <faure@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
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

#ifndef KWOPENDOCUMENTLOADER_H
#define KWOPENDOCUMENTLOADER_H

#include <QObject>
#include <KoStore.h>
#include <KoXmlReader.h>
#include <KoTextLoader.h>

class KWDocument;
class KWPageManager;
class KWTextFrameSet;
class KoOasisStyles;
class KoTextLoadingContext;
class KoTextAnchor;

class QTextCursor;
//class QColor;

/**
 * Class that has a lot of the OpenDocument (ODF) loading code for KWord.
 */
class KWOpenDocumentLoader : public KoTextLoader
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @param document the document this loader will work for.
     */
    explicit KWOpenDocumentLoader(KWDocument *document);
    virtual ~KWOpenDocumentLoader();

    KWDocument* document() const;
    KWPageManager* pageManager();
    QString currentMasterPage() const;
    QString currentFramesetName() const;

    /**
     *  @brief Loads an OASIS OpenDocument from a store.
     *  This implements the KoDocument::loadOasis method.
     */
    bool load(const KoXmlDocument& doc, KoOasisStyles& styles, const KoXmlDocument& settings, KoStore* store);

protected:
    virtual void loadSettings(KoTextLoadingContext& context, const KoXmlDocument& settings);
    virtual bool loadPageLayout(KoTextLoadingContext& context, const QString& masterPageName);
    virtual bool loadMasterPageStyle(KoTextLoadingContext& context, const QString& masterPageName);

    virtual void loadFrame(KoTextLoadingContext& context, const KoXmlElement& frameElem, QTextCursor& cursor);

private:
    void loadHeaderFooter(KoTextLoadingContext& context, const KoXmlElement& masterPage, const KoXmlElement& masterPageStyle, bool isHeader);
    void loadFinished(KoTextLoadingContext& context, QTextCursor& cursor);

private:
    /// \internal d-pointer class.
    class Private;
    /// \internal d-pointer instance.
    Private* const d;
};

#endif
