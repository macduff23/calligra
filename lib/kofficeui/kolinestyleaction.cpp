/* This file is part of the KDE project
   Copyright (C) 2004 Peter Simonsson <psn@linux.se>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kolinestyleaction.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qwhatsthis.h>
#include <qmenubar.h>

#include <kpopupmenu.h>
#include <kapplication.h>
#include <kdebug.h>
#include <ktoolbar.h>
#include <ktoolbarbutton.h>
#include <kiconloader.h>
#include <klocale.h>

class KoLineStyleAction::KoLineStyleActionPrivate
{
  public:
    KoLineStyleActionPrivate()
    {
      m_popup = new KPopupMenu(0L,"KoLineStyleAction::popup");
      m_currentStyle = Qt::SolidLine;
    }
    
    ~KoLineStyleActionPrivate()
    {
      delete m_popup;
      m_popup = 0;
    }
    
    KPopupMenu* m_popup;
    int m_currentStyle;
};

KoLineStyleAction::KoLineStyleAction(const QString &text, const QString& icon,
  QObject* parent, const char* name) : KActionMenu(text, icon, parent, name)
{
  d = new KoLineStyleActionPrivate;
   
  createMenu();
}

KoLineStyleAction::KoLineStyleAction(const QString &text, const QString& icon, const QObject* receiver,
  const char* slot, QObject* parent, const char* name) : KActionMenu(text, icon, parent, name)
{
  d = new KoLineStyleActionPrivate;
  
  createMenu();
  
  connect(this, SIGNAL(newLineStyle(int)), receiver, slot);
  connect(popupMenu(), SIGNAL(activated(int)), this, SLOT(execute(int)));
}

KoLineStyleAction::~KoLineStyleAction()
{
  delete d;
}

void KoLineStyleAction::createMenu()
{
  KPopupMenu* popup = popupMenu();
  QBitmap mask;
  QPixmap pix(70, 21);
  QPainter p(&pix, popup);
  int cindex = 0;
  QPen pen;
  pen.setWidth(2);
  popup->insertItem(i18n("None"),cindex++);

  for(int i = 1; i < 6; i++) {
    pix.fill(white);
    pen.setStyle(static_cast<Qt::PenStyle>(i));
    p.setPen(pen);
    p.drawLine(0, 10, pix.width(), 10);
    mask = pix;
    pix.setMask(mask);
    popup->insertItem(pix,cindex++);
  }
}

KPopupMenu* KoLineStyleAction::popupMenu() const
{
  return d->m_popup;
}

void KoLineStyleAction::popup(const QPoint& global)
{
  popupMenu()->popup(global);
}

int KoLineStyleAction::plug(QWidget* widget, int index)
{
  // This function is copied from KActionMenu::plug
  if (kapp && !kapp->authorizeKAction(name()))
    return -1;
  kdDebug(129) << "KAction::plug( " << widget << ", " << index << " )" << endl; // remove -- ellis
  if ( widget->inherits("QPopupMenu") )
  {
    QPopupMenu* menu = static_cast<QPopupMenu*>( widget );
    int id;

    if ( hasIconSet() )
      id = menu->insertItem( iconSet(), text(), popupMenu(), -1, index );
    else
      id = menu->insertItem( kapp->iconLoader()->loadIcon(icon(), KIcon::Small),
        text(), popupMenu(), -1, index );

    if ( !isEnabled() )
      menu->setItemEnabled( id, false );

    addContainer( menu, id );
    connect( menu, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }
  else if ( widget->inherits( "KToolBar" ) )
  {
    KToolBar *bar = static_cast<KToolBar *>( widget );

    int id_ = KAction::getToolButtonID();

    if ( icon().isEmpty() && !iconSet().isNull() )
      bar->insertButton( iconSet().pixmap(), id_, SIGNAL( clicked() ), this,
                          SLOT( slotActivated() ), isEnabled(), plainText(),
                          index );
    else
    {
      KInstance *instance;

      if ( m_parentCollection )
        instance = m_parentCollection->instance();
      else
        instance = KGlobal::instance();

      bar->insertButton( icon(), id_, SIGNAL( clicked() ), this,
                          SLOT( slotActivated() ), isEnabled(), plainText(),
                          index, instance );
    }

    addContainer( bar, id_ );

    if (!whatsThis().isEmpty())
      QWhatsThis::add( bar->getButton(id_), whatsThis() );

    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    bar->getButton(id_)->setPopup(popupMenu(), true );

    return containerCount() - 1;
  }
  else if ( widget->inherits( "QMenuBar" ) )
  {
    QMenuBar *bar = static_cast<QMenuBar *>( widget );

    int id;

    id = bar->insertItem( text(), popupMenu(), -1, index );

    if ( !isEnabled() )
      bar->setItemEnabled( id, false );

    addContainer( bar, id );
    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  return -1;
}

void KoLineStyleAction::execute(int index)
{
  setCurrentStyle(index);
  emit newLineStyle(d->m_currentStyle);
}

int KoLineStyleAction::currentStyle()
{
  return d->m_currentStyle;
}

void KoLineStyleAction::setCurrentStyle(int style)
{
  popupMenu()->setItemChecked(d->m_currentStyle, false);
  popupMenu()->setItemChecked(style, true);
  d->m_currentStyle = style;
}

#include "kolinestyleaction.moc"
