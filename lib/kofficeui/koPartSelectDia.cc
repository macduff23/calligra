/* This file is part of the KDE libraries
    Copyright (C) 1998 Torben Weis <weis@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <koQueryTypes.h>

#include "koPartSelectDia.h"

#include <klocale.h>
#include <kapp.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qlistview.h>
#include <qobjectlist.h>
#include <qpushbutton.h>

/****************************************************
 *
 * KoPartSelectDia
 *
 ****************************************************/

KoPartSelectDia::KoPartSelectDia( QWidget* parent, const char* name ) : 
    KDialogBase( parent, name, TRUE, "Insert Part", KDialogBase::Ok | KDialogBase::Cancel )
{
    listview = new QListView( this );
    listview->addColumn( i18n( "Part" ) );
    listview->addColumn( i18n( "Comment" ) );
    listview->setAllColumnsShowFocus( TRUE );
    listview->setShowSortIndicator( TRUE );
    setMainWidget( listview );
    connect( listview, SIGNAL( doubleClicked( QListViewItem * ) ),
	     this, SLOT( slotOk() ) );
    connect( listview, SIGNAL( selectionChanged( QListViewItem * ) ),
	     this, SLOT( selectionChanged( QListViewItem * ) ) );
    
    // Query for documents
    m_lstEntries = KoDocumentEntry::query();
    QValueList<KoDocumentEntry>::Iterator it = m_lstEntries.begin();
    for( ; it != m_lstEntries.end(); ++it ) {
	QListViewItem *item = new QListViewItem( listview, ( *it ).name, ( *it ).comment );
	item->setPixmap( 0, ( *it ).miniIcon );
    }

    okButton = 0;
    
#warning "After KRASH the KDialogBase API has to be opened a bit. It�s unacceptable"
#warning "that one cannot access the buttons (Ok, Cancel, etc.) without a hack!"
    QObjectList *lst = queryList( "QPushButton" );
    if ( lst && lst->first() ) {
	for ( QPushButton *b = (QPushButton*)lst->first(); b; b = (QPushButton*)lst->next() ) {
	    if ( b->text() == i18n( "&OK" ) ) {
		okButton = b;
		break;
	    }
	}
    }
    selectionChanged( 0 );
}

void KoPartSelectDia::selectionChanged( QListViewItem *item ) 
{
    if ( okButton )
	okButton->setEnabled( item != 0 );
}

KoDocumentEntry KoPartSelectDia::entry()
{
    if ( listview->currentItem() ) {
	QValueList<KoDocumentEntry>::Iterator it = m_lstEntries.begin();
	for ( ; it != m_lstEntries.end(); ++it ) {
	    if ( ( *it ).name == listview->currentItem()->text( 0 ) )
		return *it;
	}
    }
    return KoDocumentEntry();
}

KoDocumentEntry KoPartSelectDia::selectPart()
{
    KoDocumentEntry e;

    KoPartSelectDia *dlg = new KoPartSelectDia( 0, "PartSelect" );

    if (dlg->exec() == QDialog::Accepted)
	e = dlg->entry();

    delete dlg;

    return e;
}

#include "koPartSelectDia.moc"
