/*
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 * Copyright (C) 2011 by Radoslaw Wicik (radoslaw@wicik.pl)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "KoReportItemMaps.h"
#include <koproperty/Property.h>
#include <koproperty/Set.h>
#include <KoGlobal.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <kglobalsettings.h>
#include <QBuffer>
#include <kcodecs.h>
#include <renderobjects.h>
#include <MarbleWidget.h>
#include <MarbleModel.h>
#include <HttpDownloadManager.h>
#include <QImage>
#include <QPixmap>
#include <sys/socket.h>
#include <QLabel>
#include <QStringList>

#define myDebug() kDebug(44021)

KoReportItemMaps::KoReportItemMaps(QDomNode & element)
{
    myDebug() << "======" << this;
    createProperties();
    QDomNodeList nl = element.childNodes();
    QString n;
    QDomNode node;

    m_name->setValue(element.toElement().attribute("report:name"));
    m_controlSource->setValue(element.toElement().attribute("report:item-data-source"));
    //m_resizeMode->setValue(element.toElement().attribute("report:resize-mode", "stretch"));
    Z = element.toElement().attribute("report:z-index").toDouble();

    parseReportRect(element.toElement(), &m_pos, &m_size);
    myDebug() << "====== childgren:";
    for (int i = 0; i < nl.count(); i++) {
        node = nl.item(i);
        n = node.nodeName();

//         if (n == "report:Maps-data") {
// 
//             setInlineImageData(node.firstChild().nodeValue().toLatin1());
//         } else {
            kDebug() << "====== while parsing image element encountered unknow element: " << n;
//         }
    }
    //m_mapImage = new QImage(m_size.toScene().toSize(), QImage::Format_ARGB32);
    //m_mapImage->fill(QColor(200, 150, 5).rgb());
    m_totalJobs = 0;
    m_completedJobs = 0;
    initMarble();
}

void KoReportItemMaps::initMarble()
{
    
    m_marble = new Marble::MarbleWidget();
    //marble->setMapThemeId("earth/srtm/srtm.dgml");

    connect( m_marble->model()->downloadManager(), SIGNAL( jobAdded() ),
             this, SLOT( addProgresItem() ), Qt::UniqueConnection );
    
    connect( m_marble->model()->downloadManager(), SIGNAL( jobRemoved() ),
             this, SLOT( removeProgresItem() ), Qt::UniqueConnection );
    /// @todo: MapThemId should be loaded form xml or form database.
    m_marble->setMapThemeId("earth/openstreetmap/openstreetmap.dgml");

    m_marble->centerOn(20.81,52.12, false);
    m_marble->zoomView(2100);
    m_marble->setShowOverviewMap(false);
    m_marble->setFixedSize(m_size.toScene().toSize());
}


KoReportItemMaps::~KoReportItemMaps()
{
    //delete m_mapImage;
    delete m_set;
    delete m_marble;
}

// bool KoReportItemMaps::isInline() const
// {
//     return !(inlineImageData().isEmpty());
// }

// QByteArray KoReportItemMaps::inlineImageData() const
// {
//     QPixmap pixmap = m_staticImage->value().value<QPixmap>();
//     QByteArray ba;
//     QBuffer buffer(&ba);
//     buffer.open(QIODevice::ReadWrite);
//     pixmap.save(&buffer, "PNG");   // writes pixmap into ba in PNG format,
//     //TODO should i remember the format used, or save as PNG as its lossless?
// 
//     QByteArray imageEncoded(KCodecs::base64Encode(buffer.buffer(), true));
//     return imageEncoded;
// }

// void KoReportItemMaps::setInlineImageData(QByteArray dat, const QString &fn)
// {
//     //oryginal image function
//     if (!fn.isEmpty()) {
//         QPixmap pix(fn);
//         if (!pix.isNull())
//             m_staticImage->setValue(pix);
//         else {
//             QPixmap blank(1, 1);
//             blank.fill();
//             m_staticImage->setValue(blank);
//         }
//     } else {
//         const QByteArray binaryStream(KCodecs::base64Decode(dat));
//         const QPixmap pix(QPixmap::fromImage(QImage::fromData(binaryStream), Qt::ColorOnly));
//         m_staticImage->setValue(pix);
//     }
// 
// }

// QString KoReportItemMaps::mode() const
// {
//     return m_resizeMode->value().toString();
// }

// void KoReportItemMaps::setMode(const QString &m)
// {
//     if (mode() != m) {
//         m_resizeMode->setValue(m);
//     }
// }

void KoReportItemMaps::createProperties()
{
    m_set = new KoProperty::Set(0, "Maps");

    m_controlSource = new KoProperty::Property("item-data-source", QStringList(), QStringList(), QString(), i18n("Data Source"));

    //QStringList keys, strings;
    //keys << "clip" << "stretch";
    //strings << i18n("Clip") << i18n("Stretch");
    //m_resizeMode = new KoProperty::Property("resize-mode", keys, strings, "clip", i18n("Resize Mode"));

    //m_staticImage = new KoProperty::Property("static-image", QPixmap(), i18n("Static Image"));

    addDefaultProperties();
    m_set->addProperty(m_controlSource);
    //m_set->addProperty(m_resizeMode);
    //m_set->addProperty(m_staticImage);
}


void KoReportItemMaps::setColumn(const QString &c)
{
    m_controlSource->setValue(c);
}

QString KoReportItemMaps::itemDataSource() const
{
    kDebug() << m_controlSource->value().toString();
    return m_controlSource->value().toString();
}

QString KoReportItemMaps::typeName() const
{
    return "report:maps";
}

int KoReportItemMaps::render(OROPage* page,
                             OROSection* section,
                             QPointF offset,
                             QVariant data,
                             KRScriptHandler *script)
{
    Q_UNUSED(script)

    m_rendering = true;
    
    
    myDebug() << this << "data:" << data;
    
    QString dataKey = data.toString();
    QStringList dataList = dataKey.split(";");
    //myDebug() << "splited:" << dataList;

    m_targetPage = page;
    m_targetSection = section;
    m_targetOffset = offset;

    m_marble->setCenterLatitude( dataList[0].toDouble() );
    m_marble->setCenterLongitude( dataList[1].toDouble() );
    m_marble->zoomView( dataList[2].toInt() );
    
    /*if(m_marbles.count(dataKey)==0){ //no such marble yet
        marble = initMarble();
        m_marbles.insert(dataKey, marble);
        connect(marble->model(), SIGNAL(modelChanged()), this, SLOT(requestRedraw()));
        if(dataList.count()==3){
            marble->setCenterLatitude(dataList[0].toDouble());
            marble->setCenterLongitude(dataList[1].toDouble());
            marble->zoomView(dataList[2].toInt());
        }
    }else{
        marble = m_marbles[dataKey];
    }

    marble->render(m_mapImage);
    
    OROImage * id = new OROImage();
    id->setImage(*m_mapImage);
    id->setScaled(false);

    id->setPosition(m_pos.tom_progressShowTimerScene() + offset);
    id->setSize(m_size.toScene());
    OroIds oroIds;
    if (page) {
        page->addPrimitive(id);
        oroIds.pageId = id;
        myDebug() << "page:id=" <<id;
    }
    
    if (section) {
        OROImage *i2 = dynamic_cast<OROImage*>(id->clone());
        i2->setPosition(m_pos.toPoint());
        section->addPrimitive(i2);
        oroIds.sectionId = i2;
        myDebug() << "section:id=" << i2;
    }
    
    if (!page) {
        delete id;
        oroIds.pageId=0;
    }
    oroIds.marbleWidget = marble;
    m_marbleImgs[marble->model()]=oroIds;*/
    
    return 0; //Item doesnt stretch the section height
}

void KoReportItemMaps::requestRedraw()
{
    /*myDebug() << sender();
    QImage tmpImg(*m_mapImage);
    Marble::MarbleModel* marbleModel = dynamic_cast<Marble::MarbleModel*>(sender());
    OroIds *oroIds = &m_marbleImgs[marbleModel];
    oroIds->marbleWidget->render(&tmpImg);
    if(oroIds->pageId)
        oroIds->pageId->setImage(tmpImg);
    if(oroIds->sectionId)
        oroIds->sectionId->setImage(tmpImg);
    myDebug() << "pageId sectionId marbleWidget";
    myDebug() << oroIds->pageId << oroIds->sectionId << oroIds->marbleWidget;
    */
    OROPicture * pic = new OROPicture();
    QPainter p( pic->picture() );
    m_marble->render( &p );
    QPointF pos = m_pos.toScene();
    QSizeF size = m_size.toScene();

    pos += m_targetOffset;

    pic->setPosition( pos );
    pic->setSize( size );
    if ( m_targetPage ) {
        m_targetPage->addPrimitive( pic, false, true );
    }

    OROPicture *p2 = dynamic_cast<OROPicture*>( pic->clone() );
    p2->setPosition( m_pos.toPoint() );
    if ( m_targetSection ) {
        m_targetSection->addPrimitive(p2);
    }

    m_rendering = false;
    emit(finishedRendering());
}

void KoReportItemMaps::addProgresItem()
{
    m_jobMutex.lock();
    ++m_totalJobs;
    m_jobMutex.unlock();
    myDebug() << "======" << m_totalJobs;
}

void KoReportItemMaps::removeProgresItem()
{
    
    m_jobMutex.lock();
    --m_totalJobs;
    m_jobMutex.unlock();
    myDebug() << "======" << m_totalJobs;

    if ( m_totalJobs == 0 ) {
        requestRedraw();
    }
}

